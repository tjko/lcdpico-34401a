/* ******************************************************************************
  * @file decoder_34401a.c
  *
  * HP 34401A front display serial protocol decoder.
  *
  * Modified for Raspberry Pi Pico (Pico-SDK) and changed not
  * to use any global variables by Timo Kokkonen (2026).
  *
  * This code is based on Ian Johnstons work found here:
  * https://github.com/Ian-Johnston/34401A_VS_Display
  *
  * Original code comes from:
  * https://github.com/openscopeproject/HP34401a-OLED-FW/
  *
  * MIT License
  *
  * Copyright (c) 2018 qu1ck
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  *******************************************************************************/


#include "decoder_34401a.h"
#include "pico/stdlib.h"
#include <ctype.h>
#include <string.h>

#include "config.h"


static const char* annunciator_names[ANNUNCIATOR_COUNT] = {
	"*",
	"Adrs",
	"Rmt",
	"Man",
	"Trig",
	"Hold",
	"Mem",
	"Ratio",
	"Math",
	"ERROR",
	"Rear",
	"Shift",
	"Diode",
	"Continuity",
	"4-Wire"
};


static void dmm_putc_safe(dmm_context_t *ctx, char c)
{
	if (ctx->msg_idx < (sizeof(ctx->msg_work) - 2)) {
        ctx->msg_work[ctx->msg_idx++] = c;
    }
}

static inline uint32_t micros32(void)
{
	return time_us_32();
}

static inline bool lastBytesAreEof(dmm_context_t *ctx)
{
	// same as decoder.cpp: 0x00 / 0xBB pair
	return (ctx->buf_len > 0 &&
		ctx->input_buf[ctx->buf_len - 1] == 0x00 &&
		ctx->output_buf[ctx->buf_len - 1] == 0xbb);
}

static inline void endFrame(dmm_context_t *ctx)
{
	ctx->buf_len = 0;
	ctx->frame_state = FRAME_UNKNOWN;
}

static void processShiftWindow(dmm_context_t *ctx)
{
	if (!ctx->shift_window_active)
		return;

	uint32_t now_us = micros32();

	// 300 ms quiet window to collect single/double presses
	if ((uint32_t)(now_us - ctx->shift_window_start_us) > 300000u) {
		if (ctx->shift_press_count & 1u) {
			ctx->ann_state ^= 0x0800u;
			ctx->new_data_counter++;
			ctx->ann_counter++;
		}

		ctx->shift_press_count = 0;
		ctx->shift_window_active = false;
	}
}

static void updateBarGraphFromMessageFrame(dmm_context_t *ctx)
{
    // mirrors Decoder::updateBarGraph()

    // style: if input_buf[2] is digit -> POSITIVE else FULLSCALE
    uint8_t style = (isdigit((int)ctx->input_buf[2]) ? 0u : 1u);
    int16_t barvalue = 0;
    uint16_t st = (style == 0u) ? 2u : 3u;

    for (uint16_t c = 0; c < ((style == 0u) ? 4u : 3u) && st < 8u; st++) {
        if (isdigit((int)ctx->input_buf[st])) {
            barvalue = (int16_t)(10 * barvalue + (int16_t)(ctx->input_buf[st] - '0'));
            c++;
        }
    }

    if (style == 1u && ctx->input_buf[2] == '-') {
        barvalue = (int16_t)(-barvalue);
    }

    // publish only if changed
    if (ctx->bar_style != style || ctx->bar != barvalue) {
	    ctx->bar_style = style;
	    ctx->bar = barvalue;
	    ctx->new_data_counter++;
	    ctx->bar_counter++;
    }
}

static void publishAnnunciators(dmm_context_t *ctx, uint8_t h, uint8_t l)
{
    uint16_t state = ((uint16_t)h << 8) | (uint16_t)l;

    // Preserve SHIFT bit (bit11) from local button tracking
    uint16_t new_state = (uint16_t)((state & 0xF7FFu) | (ctx->ann_state & 0x0800u));

    if (new_state != ctx->ann_state) {
        ctx->ann_state = new_state;
        ctx->new_data_counter++;
        ctx->ann_counter++;
    }
}

static void messageByte(dmm_context_t *ctx, uint8_t byte)
{
	if (ctx->need_reset) {
		ctx->msg_idx = 0;
		ctx->msg_blink_work = 0;
		memset((void*)ctx->msg_work, ' ', (sizeof(ctx->msg_work) - 2));   // build a fixed-width field here
		ctx->msg_work[sizeof(ctx->msg_work) - 2] = 0;
		ctx->msg_work[sizeof(ctx->msg_work) - 1] = 0;
		ctx->need_reset = false;
	}

	switch (byte) {
	case 0x84:
		dmm_putc_safe(ctx, '.');
		break;

	case 0x86:
		dmm_putc_safe(ctx, ',');
		break;

	case 0x8d:
		// previous character blinks
		if (ctx->msg_idx > 0u)
			ctx->msg_blink_work |= (uint16_t)(1u << (ctx->msg_idx - 1u));
		dmm_putc_safe(ctx, ':');
		break;

	case 0x8c:
		dmm_putc_safe(ctx, ':');
		break;

	case 0x81:
		// control char: ignore
		break;

	case 0x00:
		// end of message handled at frame EOF
		break;

	default:
		dmm_putc_safe(ctx, (char)byte);
		break;
	}
}

static void decodeControlFrame(dmm_context_t *ctx)
{
	if (ctx->buf_len < 2)
		return;

	uint16_t cmd = ((uint16_t)ctx->input_buf[0] << 8) | ctx->input_buf[1];

	switch (cmd) {
	case 0x0049: ctx->blink_mask = (1u << 0);  break;
	case 0x7149: ctx->blink_mask = (1u << 1);  break;
	case 0x6249: ctx->blink_mask = (1u << 2);  break;
	case 0x5349: ctx->blink_mask = (1u << 3);  break;
	case 0x4449: ctx->blink_mask = (1u << 4);  break;
	case 0x3549: ctx->blink_mask = (1u << 5);  break;
	case 0x2649: ctx->blink_mask = (1u << 6);  break;
	case 0x1749: ctx->blink_mask = (1u << 7);  break;
	case 0x0849: ctx->blink_mask = (1u << 8);  break;
	case 0x7949: ctx->blink_mask = (1u << 9);  break;
	case 0x6A49: ctx->blink_mask = (1u << 10); break;
	case 0x2B49: ctx->blink_mask = (1u << 11); break;
	default:
		return;
	}

	ctx->new_data_counter++;
	ctx->main_counter++;
}


// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void decoder34401_init(dmm_context_t *ctx)
{
	// Initialize context
	memset(ctx, 0, sizeof(dmm_context_t));

	// Clear outputs
	memset((void*)ctx->main, ' ', (sizeof(ctx->main) - 2));
	ctx->main[sizeof(ctx->main) - 2] = 0;
	ctx->main[sizeof(ctx->main) - 1] = 0;

	memset((void*)ctx->msg_work, ' ', (sizeof(ctx->msg_work) - 2));
	ctx->msg_work[sizeof(ctx->msg_work) - 2] = 0;
	ctx->msg_work[sizeof(ctx->msg_work) - 1] = 0;

	// Internal
	ctx->frame_state = FRAME_INIT;
	ctx->need_reset = true;
	ctx->shift_window_active = false;

	uint32_t t_now = micros32();
	ctx->dbg_last_main_us = t_now;
	ctx->dbg_last_any_us = t_now;
	ctx->last_us = t_now;
}


void __time_critical_func(decoder34401_sckedge)(dmm_context_t *ctx)
{
    uint32_t all = gpio_get_all();

    ctx->output_acc = (uint8_t)((ctx->output_acc << 1) | (all & (1 << DO_PIN) ? 1 : 0));
    ctx->input_acc = (uint8_t)((ctx->input_acc << 1) | (all & (1 << DI_PIN) ? 1 : 0));

    // mid-byte gap detection (power-on / pause)
    uint32_t now_us = micros32();
    if (ctx->byte_len != 0u && (uint32_t)(now_us - ctx->last_us) > MAX_SCK_DELAY_US) {
	    ctx->byte_len = 0u;
    }
    ctx->last_us = now_us;
    ctx->dbg_sck_count++;

    ctx->byte_len++;
    if (ctx->byte_len >= 8) {
	    uint8_t next_wr = (uint8_t)((ctx->fifo_wr + 1u) & BYTE_FIFO_MASK);

	    if (next_wr == ctx->fifo_rd) {
		    ctx->dbg_byte_overrun_count++;
	    }
	    else {
		    ctx->byte_fifo[ctx->fifo_wr].in = ctx->input_acc;
		    ctx->byte_fifo[ctx->fifo_wr].out = ctx->output_acc;
		    ctx->fifo_wr = next_wr;

		    ctx->dbg_fifo_level = (uint32_t)((ctx->fifo_wr - ctx->fifo_rd) & BYTE_FIFO_MASK);

		    if (ctx->dbg_fifo_level > ctx->dbg_fifo_level_max) {
			    ctx->dbg_fifo_level_max = ctx->dbg_fifo_level;
		    }
	    }

	    ctx->byte_len = 0;
    }
}


void __time_critical_func(decoder34401_reset)(dmm_context_t *ctx)
{
	//uint32_t now_us = micros32();

	endFrame(ctx);
	ctx->shift_press_count = 0;
	ctx->shift_window_active = false;
	ctx->need_reset = true;

	memset(ctx->main, 0, sizeof(ctx->main));
	strncpy(ctx->main, "SYSTEM RESET  ", sizeof(ctx->main));
	ctx->ann_state = 0;
	ctx->blink_mask = 0;
	ctx->ann_counter++;
	ctx->new_data_counter++;
	ctx->main_counter++;

	ctx->dbg_reset_count++;
}


void __time_critical_func(decoder34401_int)(dmm_context_t *ctx)
{
	ctx->dbg_int_count++;
}


void decoder34401_process(dmm_context_t *ctx)
{
	processShiftWindow(ctx);

	while (ctx->fifo_rd != ctx->fifo_wr) {
		uint8_t input_byte = ctx->byte_fifo[ctx->fifo_rd].in;
		uint8_t output_byte = ctx->byte_fifo[ctx->fifo_rd].out;
		ctx->fifo_rd = (uint8_t)((ctx->fifo_rd + 1u) & BYTE_FIFO_MASK);

		// Any-byte timing debug
		{
			uint32_t now_us = micros32();

			ctx->dbg_any_gap_us = now_us - ctx->dbg_last_any_us;
			if (ctx->dbg_any_gap_us > ctx->dbg_any_gap_us_max) {
				ctx->dbg_any_gap_us_max = ctx->dbg_any_gap_us;
			}

			ctx->dbg_last_any_us = now_us;
		}

		// consume byte
		ctx->input_buf[ctx->buf_len] = input_byte;
		ctx->output_buf[ctx->buf_len] = output_byte;
		ctx->buf_len++;

		switch (ctx->frame_state) {

		case FRAME_INIT:
			if (lastBytesAreEof(ctx)) {
				endFrame(ctx);
			}
			break;

		case FRAME_UNKNOWN:
			// BUTTON frame signature
			if (ctx->buf_len == 1u && ctx->input_buf[0] == 0x00 && ctx->output_buf[0] == 0x77) {
				ctx->frame_state = FRAME_BUTTON;
				break;
			}

			if (ctx->buf_len == 2u) {
				if (ctx->input_buf[0] == 0x00 && ((ctx->input_buf[1] & 0x7F) == 0x7F)) {
					ctx->frame_state = FRAME_MESSAGE;
					break;
				}
				else if (((ctx->input_buf[0] & 0x7F) == 0x7F) && ctx->input_buf[1] == 0x00) {
					ctx->frame_state = FRAME_ANNUNCIATORS;
					break;
				}
				else {
					ctx->frame_state = FRAME_CONTROL;
					break;
				}
			}
			break;

		case FRAME_MESSAGE:
			if (lastBytesAreEof(ctx)) {
				memcpy((void*)ctx->main, (const void*)ctx->msg_work, sizeof(ctx->main));
				ctx->blink_mask = ctx->msg_blink_work;
				ctx->new_data_counter++;
				ctx->main_counter++;

				// Main-message timing debug
				{
					uint32_t now_us = micros32();

					ctx->dbg_main_gap_us = now_us - ctx->dbg_last_main_us;
					if (ctx->dbg_main_gap_us > ctx->dbg_main_gap_us_max) {
						ctx->dbg_main_gap_us_max = ctx->dbg_main_gap_us;
					}

					ctx->dbg_last_main_us = now_us;
				}

				ctx->need_reset = true;

				updateBarGraphFromMessageFrame(ctx);
				endFrame(ctx);
			}
			else {
				messageByte(ctx, ctx->input_buf[ctx->buf_len - 1u]);
			}
			break;

		case FRAME_ANNUNCIATORS:
			if (lastBytesAreEof(ctx)) {
				// same indices as original: input_buf[3], input_buf[2]
				if (ctx->buf_len >= 4u) {
					publishAnnunciators(ctx, ctx->input_buf[3], ctx->input_buf[2]);
				}
				endFrame(ctx);
			}
			break;

		case FRAME_CONTROL:
			if (lastBytesAreEof(ctx)) {
				decodeControlFrame(ctx);
				endFrame(ctx);
			}
			break;

		case FRAME_BUTTON:
			if (ctx->input_buf[ctx->buf_len - 1u] == 0x66) {
				if (ctx->buf_len >= 3u) {
					uint32_t code =
						((uint32_t)ctx->output_buf[0] << 16) |
						((uint32_t)ctx->output_buf[1] << 8) |
						((uint32_t)ctx->output_buf[2]);

					// SHIFT button code
					if (code == 7839183u) {
						uint32_t now_us = micros32();

						if (!ctx->shift_window_active) {
							ctx->shift_window_active = true;
							ctx->shift_window_start_us = now_us;
							ctx->shift_press_count = 1;
						}
						else {
							ctx->shift_press_count++;
							ctx->shift_window_start_us = now_us;   // extend window
						}
					}
					else {
						// Any other button consumes/clears SHIFT
						ctx->shift_window_active = false;
						ctx->shift_press_count = 0;

						if (ctx->ann_state & 0x0800u) {
							ctx->ann_state &= (uint16_t)~0x0800u;
							ctx->new_data_counter++;
							ctx->ann_counter++;
						}
					}
				}
				endFrame(ctx);
			}
			break;

		default:
			endFrame(ctx);
			break;
		}

		// Avoid buffer overflow in bad sync conditions
		if (ctx->buf_len >= sizeof(ctx->input_buf)) {
			endFrame(ctx);
		}
	}
}


const char* decoder34401_annunciator_name(uint ann)
{
	return (ann <= ANNUNCIATOR_COUNT ? annunciator_names[ann] : NULL);
}

