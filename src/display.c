/* display.c
   Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>

   SPDX-License-Identifier: GPL-3.0-or-later

   This file is part of LanPico.

   LcdPico is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   LcdPico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LcdPico. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#define __LINUX__
#include "PNGdec.h"
#undef __LINUX__

#include "lcd-pico.h"
#include "lt7680.h"
#include "st7701.h"
#include LCDPICO_THEME_HEADER


extern const char lcdpico_boot_logo[];
extern const char lcdpico_boot_logo_end[];
extern const char lcdpico_display_graphics[];
extern const char lcdpico_display_graphics_end[];
extern const char lcdpico_indicator_graphics[];
extern const char lcdpico_indicator_graphics_end[];

#define ROMIMAGESIZE(img) ((uint32_t)((img ## _end) - img))
#define RGB888_TO_RGB565(r,g,b) ((uint16_t)( ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3) ))


static bool display_active = false;
static uint32_t vmem_asset_ptr = LAYER3_START_ADDR;

typedef struct vmem_image {
	uint32_t base_addr;
	uint32_t size;
	uint16_t w;
	uint16_t h;
	uint8_t bpp;
} vmem_image_t;


vmem_image_t *logo = NULL;
vmem_image_t *disp = NULL;
vmem_image_t *disp2 = NULL;


#define DISPLAY_COLS 12

struct display_cell_state {
	char c;
	uint8_t flags;
};

static uint8_t display_tile_map[256];
static struct display_cell_state display_state[DISPLAY_COLS];


static int draw_cb(PNGDRAW *d)
{
	uint16_t *color = d->pUser;
	int fifo = 0;
#if 0
	if (d->y % 100 == 0) {
		printf("draw_cp: y=%d, width=%d, pitch=%d, pixeltype=%d bpp=%d alpha=%d\n",
			d->y, d->iWidth, d->iPitch, d->iPixelType, d->iBpp, d->iHasAlpha);
	}
#endif
	for (int i = 0; i < d->iWidth; i++) {
		uint16_t bpc = d->iBpp; // "iBpp" is bits per color (bpc)...
		uint16_t pixel = Black;
		int o, p;

		if (d->iPixelType == PNG_PIXEL_TRUECOLOR) {
			if (bpc == 8) {
				o = i * 3;
				pixel = RGB888_TO_RGB565(d->pPixels[o + 0], d->pPixels[o + 1], d->pPixels[o + 2]);
			}
		}
		else if (d->iPixelType == PNG_PIXEL_TRUECOLOR_ALPHA) {
			if (bpc == 8) {
				o = i * 4;
				pixel = RGB888_TO_RGB565(d->pPixels[o + 0], d->pPixels[o + 1], d->pPixels[o + 2]);
			}
		}
		else if (d->iPixelType == PNG_PIXEL_GRAYSCALE) {
			if (bpc == 1) {
				o = i / 8;
				p = i % 8;
				pixel = ( (d->pPixels[o] << p) & 0x80 ? Black : *color );
			}
		}
		if (fifo < 1) {
			lt7680_wr_fifo_empty_wait();
			fifo=32;
		}
		lt7680_data_write(pixel);
		lt7680_data_write(pixel >> 8);
		fifo--;
	}
	return 1;
}


vmem_image_t* load_image_to_vmem(const char *buf, uint32_t len, uint16_t mono_color, uint32_t* addr)
{
	PNGIMAGE *png = NULL;
	vmem_image_t *img = NULL;
	int res;
	uint16_t w, h;

	log_msg(LOG_DEBUG, "load_image_to_vmem(%p,%lu,%lx)", buf, len, addr);

	if (!(png = malloc(sizeof(PNGIMAGE)))) {
		log_msg(LOG_WARNING, "Not enough memory to decode PNG images!");
		return NULL;
	}

	res = PNG_openRAM(png, (uint8_t*)buf, len, draw_cb);
	if (res == 0) {
		w = PNG_getWidth(png);
		h = PNG_getHeight(png);

		log_msg(LOG_INFO, "Decoding PNG %dx%d %dbpc alpha=%d pixeltype=%d\n", w, h,
			PNG_getBpp(png), PNG_hasAlpha(png), PNG_getPixelType(png));
		PNG_setBuffer(png, NULL);

		lt7680_set_canvas_addr(*addr);
		lt7680_set_canvas_width(w);
		lt7680_set_active_window_xy(0,0);
		lt7680_set_active_window_wh(w,h);
		lt7680_set_graphics_xy(0,0);
		lt7680_cmd_write(0x04);

		if ((res = PNG_decode(png, &mono_color, 0))) {
			log_msg(LOG_WARNING, "PNG_decode() failed: %d\n", res);
		}
	} else {
		log_msg(LOG_WARNING, "PNG_openRAM() failed: %d", res);
	}

	PNG_close(png);
	free(png);

	lt7680_wr_fifo_empty_wait();
	log_msg(LOG_DEBUG, "PNG decode complete");

	if (res == 0) {
		if ((img = malloc(sizeof(vmem_image_t)))) {
			img->base_addr = *addr;
			img->size = (uint32_t)w * h * 2;
			img->w = w;
			img->h = h;
			img->bpp = 16;

			*addr += img->size;
			//printf("image: base_addr=%08lx, size=%lu, w=%u h=%u\n", img->base_addr, img->size, img->w, img->h);
		}
	}
	return img;
}


static void init_theme()
{
	int i;

	/* Build lookup table for display character graphics tiles */
	for(i = 0; i < 256; i++) {
		display_tile_map[i] = DISPLAY_CHAR_BLANK_IDX;
	}

	i = 0;
	while (display_chars[i] != 0) {
		int c = display_chars[i];
		if (c >= 0) {
			display_tile_map[c] = i;
			display_tile_map[tolower(c)] = i;
		}
		i++;
	}

	for(i = 0; i < DISPLAY_COLS; i++) {
		struct display_cell_state *cell = &display_state[i];
		cell->c = 0;
		cell->flags = 0;
	}
}


void display_init()
{
	uint16_t w = LCD_WIDTH;
	uint16_t h = LCD_HEIGHT;

	log_msg(LOG_NOTICE, "Initialize Display...");
	lt7680_hw_reset();

	//st7701_read_id();
	if (st7701_init()) {
		log_msg(LOG_NOTICE, "LCD initialized");
	} else {
		log_msg(LOG_NOTICE, "LCD initialization failed!");
		return;
	}
	//sleep_ms(150);
	if (lt7680_system_check()) {
		if (lt7680_init()) {
			log_msg(LOG_NOTICE, "GPU intialized");
			display_active = true;
		} else {
			log_msg(LOG_NOTICE, "GPU initialization failed!");
			return;
		}
		lt7680_setup(w, h);
#if 0
		lt7680_display_on(false);
		lt7680_set_fg_16bpp(Red);
		lt7680_draw_rect(0,0,w,h,true);
		lt7680_set_fg_16bpp(Green);
		lt7680_draw_rect(10,10,w-10,h-10,true);
		lt7680_set_fg_16bpp(Black);
		lt7680_draw_rect(20,20,w-20,h-20,true);
		lt7680_set_fg_16bpp(Blue);
		lt7680_draw_rect(25,25,75,50,true);
#endif
		clear_display();
		log_msg(LOG_INFO, "display on");
		lt7680_display_on(true);
	} else {
		log_msg(LOG_NOTICE, "No GPU found!");
		return;
	}



	log_msg(LOG_INFO, "Display boot logo");
	logo = load_image_to_vmem(lcdpico_boot_logo, ROMIMAGESIZE(lcdpico_boot_logo), Blue, &vmem_asset_ptr);
	lt7680_bte_memory_copy(0, w, (w - logo->w)/2, (h - logo->h) / 2,
			       logo->base_addr, logo->w, 0, 0,
			       0, 0, 0, 0,
			       logo->w, logo->h, 0x0c);

	log_msg(LOG_INFO, "Load graphics assets into VRAM");
	disp = load_image_to_vmem(lcdpico_display_graphics, ROMIMAGESIZE(lcdpico_display_graphics), Blue, &vmem_asset_ptr);
	disp2 = load_image_to_vmem(lcdpico_indicator_graphics, ROMIMAGESIZE(lcdpico_indicator_graphics), Blue, &vmem_asset_ptr);

	clear_display();

	init_theme();
	log_msg(LOG_INFO, "Display initialized");
}

void clear_display()
{
	if (!display_active)
		return;

	lt7680_bte_solid_fill(LAYER1_START_ADDR, LCD_WIDTH, 0, 0, LCD_WIDTH, LCD_HEIGHT * 2, Black);
}


static void update_display(struct display_cell_state newstate[], uint32_t ind_flags, bool force_refresh)
{
	static uint64_t max_delta = 0;
	static uint64_t min_delta = 0;
	static uint8_t state = 0;
	uint32_t fb = (state == 0)  ? LAYER1_START_ADDR : LAYER2_START_ADDR;

	absolute_time_t t_start = get_absolute_time();

	for (int i = 0; i < DISPLAY_COLS; i++) {
		if (force_refresh || newstate[i].c != display_state[i].c
			|| newstate[i].flags != display_state[i].flags) {

			/* Draw character */
			display_state[i] = newstate[i];
			uint16_t y = DISPLAY_Y_OFFSET + (DISPLAY_COLS - 1 - i) * DISPLAY_CHAR_H;
			uint16_t x = DISPLAY_X_OFFSET;
			uint8_t tile = display_tile_map[(uint8_t)display_state[i].c];
			uint16_t tile_x = (tile % DISPLAY_CHAR_MAP_W) * DISPLAY_CHAR_W;
			uint16_t tile_y = (tile / DISPLAY_CHAR_MAP_W) * DISPLAY_CHAR_H;

			lt7680_bte_memory_copy(fb, LCD_WIDTH, x, y,
					0, 0, 0, 0,
					disp->base_addr, disp->w, tile_x, tile_y,
					DISPLAY_CHAR_W, DISPLAY_CHAR_H, 0x0a);

			/* Draw character overlays */
			for (int o = 0; o < DISPLAY_CHAR_OVERLAY_COUNT; o++) {
				if (display_state[i].flags & (1 << o)) {
					const struct lcd_char_overlay *ov = &display_char_overlays[o];
					uint16_t ovtile_x = (ov->tile % DISPLAY_CHAR_MAP_W) * DISPLAY_CHAR_W;
					uint16_t ovtile_y = (ov->tile / DISPLAY_CHAR_MAP_W) * DISPLAY_CHAR_H;

					lt7680_bte_memory_copy(fb, LCD_WIDTH, x + ov->x, y + ov->y,
							0, 0, 0, 0,
							disp->base_addr, disp->w, ovtile_x + ov->x, ovtile_y + ov->y,
							ov->w, ov->h, 0x0a);
				}
			}
		}
	}

	/* Set indicator "lights" */
	for (int i = 0; i < DISPLAY_IND_COUNT; i++) {
		uint8_t tile = display_indicators[INDICATOR_BLANK].tile;
		const struct lcd_indicator *d = &display_indicators[i];

		if (i == INDICATOR_BLANK)
			continue;

		if (ind_flags & (1 << i))
			tile = d->tile;

		uint16_t y = d->y;
		uint16_t h = d->h > 0 ? d->h : DISPLAY_IND_H;
		uint16_t tile_x = (tile % DISPLAY_IND_MAP_W) * DISPLAY_IND_W;
		uint16_t tile_y = (tile / DISPLAY_IND_MAP_W) * DISPLAY_IND_H;

		lt7680_bte_memory_copy(fb, LCD_WIDTH, 0, y,
				0, 0, 0, 0,
				disp2->base_addr, disp2->w, tile_x, tile_y,
				DISPLAY_IND_W, h, 0x0a);
	}

	lt7680_set_misa_addr(fb);
	if (state == 0)
		state = 1;
	else
		state = 0;

	absolute_time_t t_end = get_absolute_time();
	uint64_t delta = absolute_time_diff_us(t_start, t_end);
	if (delta > max_delta) {
		max_delta = delta;
		log_msg(LOG_INFO, "display refresh max time: %llu%", max_delta);
	}
	if (delta < min_delta || min_delta == 0) {
		min_delta = delta;
		log_msg(LOG_INFO, "display refresh min time: %llu%", min_delta);
	}
}

void display_status(const struct fanpico_state *state,
	const struct fanpico_config *config)
{
	static int pos = 0;
	//uint16_t w = LCD_WIDTH;
	//uint16_t h = LCD_HEIGHT;
	struct display_cell_state disp[DISPLAY_COLS];
	uint32_t ind = 0x0000ffff;

	if (!display_active)
		return;

	for(int i = 0; i < DISPLAY_COLS; i++) {
		disp[i].c = '0' + i + (pos % 96) - 24;
		disp[i].flags = 0;
		if (i == 2) disp[i].flags = (1 << OVERLAY_PERIOD);
		if (i == 4) disp[i].flags = (1 << OVERLAY_COMMA);
		if (i == 6) disp[i].flags = (1 << OVERLAY_PERIOD) | (1 << OVERLAY_DOT);
		//printf("%c",disp[i].c);
	}
	//printf("\n");
	update_display(disp, ind, true);
	pos++;
#if 0
	lt7680_bte_memory_copy(0, w, 50, 0,
			disp->base_addr, disp->w, (i % 4) * 190, 0,
			0, 0, 0, 0,
			190, 12*80, 0x0c);
	i++;
#endif
}

void display_message(int rows, const char **text_lines)
{
	if (!display_active)
		return;
}


/* eof :-) */
