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


#include <stdio.h>
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


static inline void dmm_putc_safe(dmm_context_t *ctx, char c)
{
	int *state = &ctx->work_state;

	if (ctx->msg_idx >= (DISPLAY_BUF_LEN - 1))
		return;

	ctx->msg_work[ctx->msg_idx++] = (isprint(c) ? c : ' ');

	/* validate message for errors */

	if (ctx->corrupt_msg)
		return;
	if (!isprint(c) || iscntrl(c)) {
		ctx->corrupt_msg = true;
		return;
	}

	/* check if message looks like a valid measurement/reading */
	if (isdigit(c))
		ctx->num_count++;
	else if (c == '.')
		ctx->period_count++;

	if (*state < 0) {
		return;
	}
	else if (*state == 0) {
		if (c == '-' || c  == ' ') {
			*state = 1;
		} else if (isdigit(c)) {
			*state = 2;
		} else {
			*state = -1;
		}
	}
	else if (*state == 1) {
		if (isdigit(c)) {
			*state = 2;
		}
		else if (c == ' ') {
			if (ctx->msg_idx > 3)
				*state = -1;
		}
		else {
			*state = -1;
		}
	}
	else if (*state == 2) {
		if (isdigit(c)) {
			if (ctx->num_count >= 7)
				*state = 3;
		} else if (c == ' ') {
			*state = 3;
		} else if (c == '.' || c == ',') {
			if (ctx->period_count > 1)
				*state = -1;
		} else {
			*state = -1;
		}
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
    uint16_t state = ((uint16_t)h << 8) | l;

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
	if (ctx->msg_work_need_reset) {
		ctx->msg_idx = 0;
		ctx->msg_blink_work = 0;
		memset(ctx->msg_work, ' ', (DISPLAY_BUF_LEN - 2));   // build a fixed-width field here
		ctx->msg_work[DISPLAY_BUF_LEN - 2] = 0;
		ctx->msg_work[DISPLAY_BUF_LEN - 1] = 0;
		ctx->work_state = 0;
		ctx->num_count = 0;
		ctx->period_count = 0;
		ctx->corrupt_msg = false;
		ctx->valid_reading = false;
		ctx->msg_work_need_reset = false;
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
	case 0x1349: ctx->blink_mask = (1u << 3);  break;
	case 0x5449: ctx->blink_mask = (1u << 4);  break;
	case 0x2549: ctx->blink_mask = (1u << 5);  break;
	case 0x3649: ctx->blink_mask = (1u << 6);  break;
	case 0x4749: ctx->blink_mask = (1u << 7);  break;
	case 0x3849: ctx->blink_mask = (1u << 8);  break;
	case 0x4949: ctx->blink_mask = (1u << 9);  break;
	case 0x5A49: ctx->blink_mask = (1u << 10); break;
	case 0x2B49: ctx->blink_mask = (1u << 11); break;
	default:
		return;
	}

	ctx->new_data_counter++;
	ctx->main_counter++;
}

static void process_reset(dmm_context_t *ctx)
{
	ctx->shift_press_count = 0;
	ctx->shift_window_active = false;
	ctx->msg_work_need_reset = true;
	ctx->corrupt_msg = false;
	ctx->valid_reading = false;

	memset(ctx->main, 0, DISPLAY_BUF_LEN);
	strncpy(ctx->main, "SYSTEM RESET  ", DISPLAY_BUF_LEN);
	ctx->ann_state = 0;
	ctx->blink_mask = 0;
	ctx->ann_counter++;
	ctx->new_data_counter++;
	ctx->main_counter++;

	ctx->reset_received = false;
}


// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void decoder34401_init(dmm_context_t *ctx)
{
	// Initialize context
	memset(ctx, 0, sizeof(dmm_context_t));

	// Clear outputs
	memset(ctx->main, ' ', (DISPLAY_BUF_LEN - 2));
	ctx->main[DISPLAY_BUF_LEN - 2] = 0;
	ctx->main[DISPLAY_BUF_LEN - 1] = 0;
	memcpy(ctx->msg_work, ctx->main, DISPLAY_BUF_LEN);
	ctx->last_reading[0] = 0;

	// Internal
	ctx->frame_state = FRAME_INIT;
	ctx->msg_work_need_reset = true;
	ctx->shift_window_active = false;
}


static inline void fifo_append(dmm_context_t *ctx)
{
	uint16_t next_wr = (ctx->fifo_wr + 1u) & BYTE_FIFO_MASK;

	if (next_wr == ctx->fifo_rd) {
		ctx->dbg_byte_overrun_count++;
	}
	else {
		ctx->byte_fifo[ctx->fifo_wr].in = ctx->input_acc;
		ctx->byte_fifo[ctx->fifo_wr].out = ctx->output_acc;
		ctx->fifo_wr = next_wr;

		ctx->dbg_fifo_level = (uint32_t)((ctx->fifo_wr - ctx->fifo_rd) & BYTE_FIFO_MASK);
		if (ctx->dbg_fifo_level > ctx->dbg_fifo_level_max)
			ctx->dbg_fifo_level_max = ctx->dbg_fifo_level;
	}

	ctx->byte_len = 0;
}

void __time_critical_func(decoder34401_sckedge)(dmm_context_t *ctx)
{
	uint32_t now_us = micros32();
	uint32_t pins = gpio_get_all();


	// mid-byte gap detection (power-on / pause)
	if (ctx->last_us > 0) {
		ctx->dbg_sck_gap_us = now_us - ctx->last_us;
		if (ctx->dbg_sck_gap_us > ctx->dbg_sck_gap_us_max)
			ctx->dbg_sck_gap_us_max = ctx->dbg_sck_gap_us;

		if (ctx->byte_len != 0 && ctx->dbg_sck_gap_us > MAX_SCK_DELAY_US) {
			ctx->dbg_mid_byte_gap_input = ctx->input_acc;
			ctx->dbg_mid_byte_gap_output = ctx->output_acc;
			ctx->dbg_mid_byte_gap_bits = ctx->byte_len;
#if 1
			ctx->input_acc = 0x00;
			ctx->output_acc = 0xbb;
			fifo_append(ctx);
#else
			ctx->byte_len = 0;
#endif
			ctx->dbg_mid_byte_gap_count++;
			ctx->dbg_mid_byte_gap_last_time = now_us;
			ctx->dbg_mid_byte_gap_last_us = ctx->dbg_sck_gap_us;
			if (ctx->dbg_sck_gap_us > ctx->dbg_mid_byte_gap_max_us)
				ctx->dbg_mid_byte_gap_max_us = ctx->dbg_sck_gap_us;
		}
	}
	ctx->last_us = now_us;


	// read in one bit from DO and DI pins
	ctx->output_acc = (uint8_t)((ctx->output_acc << 1) | (pins & (1 << DO_PIN) ? 1 : 0));
	ctx->input_acc = (uint8_t)((ctx->input_acc << 1) | (pins & (1 << DI_PIN) ? 1 : 0));
	ctx->byte_len++;

	if (ctx->byte_len >= 8) {
		fifo_append(ctx);
	}

	ctx->dbg_sck_count++;
}


void __time_critical_func(decoder34401_reset)(dmm_context_t *ctx)
{
	ctx->dbg_reset_last_time = micros32();
	ctx->dbg_reset_count++;
	ctx->byte_len = 0;
	ctx->reset_received = true;
}


void __time_critical_func(decoder34401_int)(dmm_context_t *ctx)
{
	ctx->dbg_int_last_time = micros32();
	ctx->dbg_int_count++;
}


void decoder34401_process(dmm_context_t *ctx)
{
	uint32_t p_start = micros32();

	if (!ctx->reset_received)
		processShiftWindow(ctx);

	while (ctx->fifo_rd != ctx->fifo_wr) {
		// Check for reset signal between every byte
		if (ctx->reset_received) {
			process_reset(ctx);
			break;
		}

		uint8_t input_byte = ctx->byte_fifo[ctx->fifo_rd].in;
		uint8_t output_byte = ctx->byte_fifo[ctx->fifo_rd].out;
		ctx->fifo_rd = (uint8_t)((ctx->fifo_rd + 1u) & BYTE_FIFO_MASK);

		// Any-byte timing debug
		uint32_t now_us = micros32();
		if (ctx->dbg_any_last_time > 0) {
			ctx->dbg_any_gap_us = now_us - ctx->dbg_any_last_time;
			if (ctx->dbg_any_gap_us > ctx->dbg_any_gap_us_max)
				ctx->dbg_any_gap_us_max = ctx->dbg_any_gap_us;
		}
		ctx->dbg_any_last_time = now_us;

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
			if (lastBytesAreEof(ctx)) {
				endFrame(ctx);
				break;
			}

			// BUTTON frame signature
			if (ctx->buf_len == 1u && ctx->input_buf[0] == 0x00 && ctx->output_buf[0] == 0x77) {
				ctx->frame_state = FRAME_BUTTON;
				break;
			}

			if (ctx->buf_len >= 2u) {
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
				uint32_t now_us = micros32();

				if (ctx->work_state == 3) {
					ctx->valid_reading = true;
				}
				else if ((ctx->num_count >= 3 && ctx->period_count == 1) || ctx->num_count >=5) {
					//printf("invalid format: '%s' [%d]\n", ctx->msg_work,ctx->work_state);
					ctx->corrupt_msg = true;
				}

				if (ctx->corrupt_msg) {
					ctx->dbg_bad_msg_last_time = now_us;
					ctx->dbg_bad_msg_count++;
				}

				memcpy(ctx->main, ctx->msg_work, DISPLAY_BUF_LEN);
				if (ctx->valid_reading)
					memcpy(ctx->last_reading, ctx->msg_work, DISPLAY_BUF_LEN);
				ctx->blink_mask = ctx->msg_blink_work;
				ctx->new_data_counter++;
				ctx->main_counter++;

				// Main-message timing debug
				if (ctx->dbg_main_last_time > 0) {
					ctx->dbg_main_gap_us = now_us - ctx->dbg_main_last_time;
					if (ctx->dbg_main_gap_us > ctx->dbg_main_gap_us_max)
						ctx->dbg_main_gap_us_max = ctx->dbg_main_gap_us;
				}
				ctx->dbg_main_last_time = now_us;

				updateBarGraphFromMessageFrame(ctx);
				ctx->msg_work_need_reset = true;
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
			ctx->dbg_buf_overflow_count++;
		}
	}

	uint32_t p_time = micros32() - p_start;
	if (p_time > ctx->dbg_max_process_us) {
		ctx->dbg_max_process_us = p_time;
	}
}


const char* decoder34401_annunciator_name(uint ann)
{
	return (ann <= ANNUNCIATOR_COUNT ? annunciator_names[ann] : NULL);
}

