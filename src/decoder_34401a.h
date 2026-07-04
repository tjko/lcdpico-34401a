#ifndef DECODER_34401A_H
#define DECODER_34401A_H

#include "pico.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SCK_DELAY_US 1500u  // 1.5ms

// FIFO size must be power of 2 for simple wrap
#define BYTE_FIFO_SIZE 256u
#define BYTE_FIFO_MASK (BYTE_FIFO_SIZE - 1u)


typedef enum dmm_annunciators {
	ANN_SMP   = 0,
	ANN_ADRS  = 1,
	ANN_RMT   = 2,
	ANN_MAN   = 3,
	ANN_TRIG  = 4,
	ANN_HOLD  = 5,
	ANN_MEM   = 6,
	ANN_RATIO = 7,
	ANN_MATH  = 8,
	ANN_ERROR = 9,
	ANN_REAR  = 10,
	ANN_SHIFT = 11,
	ANN_DIODE = 12,
	ANN_CONT  = 13,
	ANN_4WIRE = 14,
} dmm_annunciators_t;

typedef struct sniff_byte {
	uint8_t in;
	uint8_t out;
} sniff_byte_t;

typedef enum frame_state {
    FRAME_INIT,
    FRAME_UNKNOWN,
    FRAME_MESSAGE,
    FRAME_ANNUNCIATORS,
    FRAME_CONTROL,
    FRAME_BUTTON
} frame_state_t;

typedef struct dmm_context {
	char     main[16];          // null-terminated
	uint16_t ann_state;         // annunciator bitfield (incl shift)
	int16_t  bar;               // parsed bargraph value
	uint8_t  bar_style;         // 0=POSITIVE, 1=FULLSCALE
	uint32_t new_data_counter;  // increments whenever any of above changes
	uint32_t main_counter;
	uint32_t ann_counter;
	uint32_t bar_counter;
	uint16_t blink_mask;   // bits 0..13 = blink this character position

        // ===== Minimal debug =====
	uint32_t dbg_byte_overrun_count;

	uint32_t dbg_main_gap_us;
	uint32_t dbg_main_gap_us_max;
	uint32_t dbg_last_main_us;

	uint32_t dbg_any_gap_us;
	uint32_t dbg_any_gap_us_max;
	uint32_t dbg_last_any_us;

	uint32_t dbg_fifo_level_max;
	uint32_t dbg_fifo_level;

        // ===== Internal sniff state =====
	uint8_t  byte_len;
	uint8_t  input_acc, output_acc;

	sniff_byte_t byte_fifo[BYTE_FIFO_SIZE];
	uint8_t fifo_wr;
	uint8_t fifo_rd;

	uint32_t last_us;

        // ===== Frame buffers & parse =====
	uint8_t input_buf[100];
	uint8_t output_buf[100];
	uint8_t buf_len;


        // ===== SHIFT handling =====
	uint32_t shift_window_start_us;
	uint8_t  shift_press_count;
	bool     shift_window_active;


	frame_state_t frame_state;

        // ===== MESSAGE assembly (like Eventhandler::messageByte) =====
	uint8_t msg_idx;
	bool need_reset;
	char msg_work[16];
	uint16_t msg_blink_work;

} dmm_context_t;


// ===== Decoder API =====
void decoder34401_init(dmm_context_t *ctx);
void __time_critical_func(decoder34401_sckedge)(dmm_context_t *ctx);    // call from GPIO interrupt callback
void decoder34401_process(dmm_context_t *ctx);    // call frequently in main loop


#ifdef __cplusplus
}
#endif

#endif
