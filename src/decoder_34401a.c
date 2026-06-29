/**
  ******************************************************************************
  * @file    decoder_34401a.c
  * Sniffing code gleaned from here, re-written and modified. See decoder.cpp
  * https://github.com/openscopeproject/HP34401a-OLED-FW/
  * Original = Arduino C++
  * This project = STM32 HAL embedded C
  ******************************************************************************
*/


#include "decoder_34401a.h"
//#include "main.h"     // for FP_* pin defines
#include "pico/stdlib.h"
#include <ctype.h>
#include <string.h>

#include "config.h"

// Match original config.h
#define MAX_SCK_DELAY_US 1500u  // 1.5ms

// FIFO size must be power of 2 for simple wrap
#define BYTE_FIFO_SIZE 256u
#define BYTE_FIFO_MASK (BYTE_FIFO_SIZE - 1u)

// ===== Extracted data =====
volatile char     dmm_main[16];
volatile uint16_t dmm_ann_state;
volatile int16_t  dmm_bar;
volatile uint8_t  dmm_bar_style;        // 0=POSITIVE, 1=FULLSCALE
volatile uint32_t dmm_new_data_counter;

volatile uint16_t dmm_blink_mask;
static void decodeControlFrame(void);

volatile uint32_t dbg_main_gap_us;
volatile uint32_t dbg_main_gap_us_max;
volatile uint32_t dbg_last_main_us;

volatile uint32_t dbg_any_gap_us;
volatile uint32_t dbg_any_gap_us_max;
volatile uint32_t dbg_last_any_us;

volatile uint32_t dbg_fifo_level_max;
volatile uint32_t dbg_fifo_level;

// ===== Internal sniff state =====
static volatile uint8_t  byte_len;
static volatile uint8_t  input_acc, output_acc;

typedef struct {
    uint8_t in;
    uint8_t out;
} sniff_byte_t;

static volatile sniff_byte_t byte_fifo[BYTE_FIFO_SIZE];
static volatile uint8_t fifo_wr;
static volatile uint8_t fifo_rd;

static uint32_t last_us;

// ===== Frame buffers & parse =====
static uint8_t input_buf[100];
static uint8_t output_buf[100];
static uint8_t buf_len;

// ===== Minimal debug =====
static volatile uint32_t dbg_byte_overrun_count = 0;

volatile uint32_t dmm_main_counter;
volatile uint32_t dmm_ann_counter;
volatile uint32_t dmm_bar_counter;

// ===== SHIFT handling =====
static uint32_t shift_window_start_us = 0;
static uint8_t  shift_press_count = 0;
static bool     shift_window_active = false;

typedef enum {
    FRAME_INIT,
    FRAME_UNKNOWN,
    FRAME_MESSAGE,
    FRAME_ANNUNCIATORS,
    FRAME_CONTROL,
    FRAME_BUTTON
} frame_state_t;

static frame_state_t frame_state = FRAME_INIT;

// ===== MESSAGE assembly (like Eventhandler::messageByte) =====
static uint8_t msg_idx = 0;
static bool need_reset = true;
static char msg_work[16];
static uint16_t msg_blink_work = 0;

static void dmm_putc_safe(char c)
{
    if (msg_idx < 14u) {
        msg_work[msg_idx++] = c;
    }
}

#if 0
// -----------------------------------------------------------------------------
// DWT micros (STM32F1): fast and simple
// -----------------------------------------------------------------------------
static inline void DWT_Init(void)
{
    // Enable TRC
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    // Reset cycle counter
    DWT->CYCCNT = 0;
    // Enable cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t micros32(void)
{
    // SystemCoreClock in Hz; convert cycles to microseconds
    return (uint32_t)(DWT->CYCCNT / (SystemCoreClock / 1000000u));
}
#else
static inline uint32_t micros32(void)
{
	return time_us_32();
}
#endif

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
static inline bool lastBytesAreEof(void)
{
    // same as decoder.cpp: 0x00 / 0xBB pair
    return (buf_len > 0 &&
        input_buf[buf_len - 1] == 0x00 &&
        output_buf[buf_len - 1] == 0xbb);
}

static inline void endFrame(void)
{
    buf_len = 0;
    frame_state = FRAME_UNKNOWN;
}

static void processShiftWindow(void)
{
    if (shift_window_active) {
        uint32_t now_us = micros32();

        // 300 ms quiet window to collect single/double presses
        if ((uint32_t)(now_us - shift_window_start_us) > 300000u) {
            if (shift_press_count & 1u) {
                dmm_ann_state ^= 0x0800u;
                dmm_new_data_counter++;
                dmm_ann_counter++;
            }

            shift_press_count = 0;
            shift_window_active = false;
        }
    }
}

static void updateBarGraphFromMessageFrame(void)
{
    // mirrors Decoder::updateBarGraph()

    // style: if input_buf[2] is digit -> POSITIVE else FULLSCALE
    uint8_t style = (isdigit((int)input_buf[2]) ? 0u : 1u);
    int16_t barvalue = 0;

    uint16_t st = (style == 0u) ? 2u : 3u;
    uint16_t c = 0;

    for (; c < ((style == 0u) ? 4u : 3u) && st < 8u; st++) {
        if (isdigit((int)input_buf[st])) {
            barvalue = (int16_t)(10 * barvalue + (int16_t)(input_buf[st] - '0'));
            c++;
        }
    }

    if (style == 1u && input_buf[2] == '-') {
        barvalue = (int16_t)(-barvalue);
    }

    // publish only if changed
    if (dmm_bar_style != style || dmm_bar != barvalue) {
        dmm_bar_style = style;
        dmm_bar = barvalue;
        dmm_new_data_counter++;
        dmm_bar_counter++;
    }
}

static void publishAnnunciators(uint8_t h, uint8_t l)
{
    uint16_t state = ((uint16_t)h << 8) | (uint16_t)l;

    // Preserve SHIFT bit (bit11) from local button tracking
    uint16_t new_state = (uint16_t)((state & 0xF7FFu) | (dmm_ann_state & 0x0800u));

    if (new_state != dmm_ann_state) {
        dmm_ann_state = new_state;
        dmm_new_data_counter++;
        dmm_ann_counter++;
    }
}

static void messageByte(uint8_t byte)
{
    if (need_reset) {
        msg_idx = 0;
        msg_blink_work = 0;
        memset((void*)msg_work, ' ', 14);   // build a fixed-width field here
        msg_work[14] = '\0';
        msg_work[15] = '\0';
        need_reset = false;
    }

    switch (byte) {
    case 0x84:
        dmm_putc_safe('.');
        break;

    case 0x86:
        dmm_putc_safe(',');
        break;

    case 0x8d:
        // previous character blinks
        if (msg_idx > 0u)
            msg_blink_work |= (uint16_t)(1u << (msg_idx - 1u));
        dmm_putc_safe(':');
        break;

    case 0x8c:
        dmm_putc_safe(':');
        break;

    case 0x81:
        // control char: ignore
        break;

    case 0x00:
        // end of message handled at frame EOF
        break;

    default:
        dmm_putc_safe((char)byte);
        break;
    }
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
void Decoder34401_Init(void)
{
    dbg_fifo_level = 0;
    dbg_fifo_level_max = 0;

    dbg_main_gap_us = 0;
    dbg_main_gap_us_max = 0;
    dbg_last_main_us = micros32();

    dbg_any_gap_us = 0;
    dbg_any_gap_us_max = 0;
    dbg_last_any_us = micros32();



    //DWT_Init();

    dmm_blink_mask = 0;
    msg_blink_work = 0;

    // Clear outputs
    memset((void*)dmm_main, ' ', 14);
    dmm_main[14] = '\0';
    dmm_main[15] = '\0';

    memset((void*)msg_work, ' ', 14);
    msg_work[14] = '\0';
    msg_work[15] = '\0';

    dmm_ann_state = 0;
    dmm_bar = 0;
    dmm_bar_style = 0;
    dmm_new_data_counter = 0;

    // Internal
    byte_len = 0;
    input_acc = output_acc = 0;
    fifo_wr = 0;
    fifo_rd = 0;
    buf_len = 0;
    frame_state = FRAME_INIT;
    need_reset = true;

    shift_window_start_us = 0;
    shift_press_count = 0;
    shift_window_active = false;

    dbg_byte_overrun_count = 0;

    dmm_main_counter = 0;
    dmm_ann_counter = 0;
    dmm_bar_counter = 0;

    last_us = micros32();

    dbg_any_gap_us_max = 0;
    dbg_main_gap_us_max = 0;
    dbg_byte_overrun_count = 0;
}

void __time_critical_func(Decoder34401_SckEdge)(void)
{
    // mid-byte gap detection (power-on / pause)
    uint32_t now_us = micros32();
    if (byte_len != 0u && (uint32_t)(now_us - last_us) > MAX_SCK_DELAY_US) {
        byte_len = 0u;
    }
    last_us = now_us;

#if 0
    // Read PB14/PB15 as 2-bit: (DIN, DOUT) or vice versa per original
    // Original: tinp = (IDR >> 14) & 0b11; output uses bit1 (PB15), input uses bit0 (PB14)
    uint32_t idr = FP_GPIO_Port->IDR;
    uint8_t tinp = (uint8_t)((idr >> 14) & 0x3u);

    output_acc = (uint8_t)((output_acc << 1) | (tinp >> 1));
    input_acc = (uint8_t)((input_acc << 1) | (tinp & 1u));
#else
    uint32_t all = gpio_get_all();

    output_acc = (uint8_t)((output_acc << 1) | (all & (1 << DO_PIN) ? 1 : 0));
    input_acc = (uint8_t)((input_acc << 1) | (all & (1 << DI_PIN) ? 1 : 0));
#endif


    byte_len++;
    if (byte_len == 8u) {
        uint8_t next_wr = (uint8_t)((fifo_wr + 1u) & BYTE_FIFO_MASK);

        if (next_wr == fifo_rd) {
            dbg_byte_overrun_count++;
        }
        else {
            byte_fifo[fifo_wr].in = input_acc;
            byte_fifo[fifo_wr].out = output_acc;
            fifo_wr = next_wr;

            dbg_fifo_level = (uint32_t)((fifo_wr - fifo_rd) & BYTE_FIFO_MASK);

            if (dbg_fifo_level > dbg_fifo_level_max) {
                dbg_fifo_level_max = dbg_fifo_level;
            }
        }

        byte_len = 0u;
    }
}

void Decoder34401_Process(void)
{
    processShiftWindow();

    while (fifo_rd != fifo_wr) {
        uint8_t input_byte;
        uint8_t output_byte;

        input_byte = byte_fifo[fifo_rd].in;
        output_byte = byte_fifo[fifo_rd].out;
        fifo_rd = (uint8_t)((fifo_rd + 1u) & BYTE_FIFO_MASK);

        // Any-byte timing debug
        {
            uint32_t now_us = micros32();

            dbg_any_gap_us = now_us - dbg_last_any_us;
            if (dbg_any_gap_us > dbg_any_gap_us_max) {
                dbg_any_gap_us_max = dbg_any_gap_us;
            }

            dbg_last_any_us = now_us;
        }

        // consume byte
        input_buf[buf_len] = input_byte;
        output_buf[buf_len] = output_byte;
        buf_len++;

        switch (frame_state) {

        case FRAME_INIT:
            if (lastBytesAreEof()) {
                endFrame();
            }
            break;

        case FRAME_UNKNOWN:
            // BUTTON frame signature
            if (buf_len == 1u && input_buf[0] == 0x00 && output_buf[0] == 0x77) {
                frame_state = FRAME_BUTTON;
                break;
            }

            if (buf_len == 2u) {
                if (input_buf[0] == 0x00 && ((input_buf[1] & 0x7F) == 0x7F)) {
                    frame_state = FRAME_MESSAGE;
                    break;
                }
                else if (((input_buf[0] & 0x7F) == 0x7F) && input_buf[1] == 0x00) {
                    frame_state = FRAME_ANNUNCIATORS;
                    break;
                }
                else {
                    frame_state = FRAME_CONTROL;
                    break;
                }
            }
            break;

        case FRAME_MESSAGE:
            if (lastBytesAreEof()) {
                memcpy((void*)dmm_main, (const void*)msg_work, sizeof(dmm_main));
                dmm_blink_mask = msg_blink_work;
                dmm_new_data_counter++;
                dmm_main_counter++;

                // Main-message timing debug
                {
                    uint32_t now_us = micros32();

                    dbg_main_gap_us = now_us - dbg_last_main_us;
                    if (dbg_main_gap_us > dbg_main_gap_us_max) {
                        dbg_main_gap_us_max = dbg_main_gap_us;
                    }

                    dbg_last_main_us = now_us;
                }

                need_reset = true;

                //updateBarGraphFromMessageFrame();         // not using tha bargraph
                endFrame();
            }
            else {
                messageByte(input_buf[buf_len - 1u]);
            }
            break;

        case FRAME_ANNUNCIATORS:
            if (lastBytesAreEof()) {
                // same indices as original: input_buf[3], input_buf[2]
                if (buf_len >= 4u) {
                    publishAnnunciators(input_buf[3], input_buf[2]);
                }
                endFrame();
            }
            break;

        case FRAME_CONTROL:
            if (lastBytesAreEof()) {
                decodeControlFrame();
                endFrame();
            }
            break;

        case FRAME_BUTTON:
            if (input_buf[buf_len - 1u] == 0x66) {
                if (buf_len >= 3u) {
                    uint32_t code =
                        ((uint32_t)output_buf[0] << 16) |
                        ((uint32_t)output_buf[1] << 8) |
                        ((uint32_t)output_buf[2]);

                    // SHIFT button code
                    if (code == 7839183u) {
                        uint32_t now_us = micros32();

                        if (!shift_window_active) {
                            shift_window_active = true;
                            shift_window_start_us = now_us;
                            shift_press_count = 1;
                        }
                        else {
                            shift_press_count++;
                            shift_window_start_us = now_us;   // extend window
                        }
                    }
                    else {
                        // Any other button consumes/clears SHIFT
                        shift_window_active = false;
                        shift_press_count = 0;

                        if (dmm_ann_state & 0x0800u) {
                            dmm_ann_state &= (uint16_t)~0x0800u;
                            dmm_new_data_counter++;
                            dmm_ann_counter++;
                        }
                    }
                }
                endFrame();
            }
            break;

        default:
            endFrame();
            break;
        }

        // Avoid buffer overflow in bad sync conditions
        if (buf_len >= sizeof(input_buf)) {
            endFrame();
        }
    }
}


static void decodeControlFrame(void)
{
    uint16_t cmd;

    if (buf_len < 2u)
        return;

    cmd = ((uint16_t)input_buf[0] << 8) | (uint16_t)input_buf[1];

    switch (cmd) {
    case 0x0049: dmm_blink_mask = (1u << 0);  break;
    case 0x7149: dmm_blink_mask = (1u << 1);  break;
    case 0x6249: dmm_blink_mask = (1u << 2);  break;
    case 0x5349: dmm_blink_mask = (1u << 3);  break;
    case 0x4449: dmm_blink_mask = (1u << 4);  break;
    case 0x3549: dmm_blink_mask = (1u << 5);  break;
    case 0x2649: dmm_blink_mask = (1u << 6);  break;
    case 0x1749: dmm_blink_mask = (1u << 7);  break;
    case 0x0849: dmm_blink_mask = (1u << 8);  break;
    case 0x7949: dmm_blink_mask = (1u << 9);  break;
    case 0x6A49: dmm_blink_mask = (1u << 10); break;
    case 0x2B49: dmm_blink_mask = (1u << 11); break;
    default:
        return;
    }

    dmm_new_data_counter++;
    dmm_main_counter++;
}
