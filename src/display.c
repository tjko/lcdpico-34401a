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


extern const char lcdpico_boot_logo[];
extern const char lcdpico_boot_logo_end[];
extern const char lcdpico_hp_logo[];
extern const char lcdpico_hp_logo_end[];
extern const char lcdpico_display_graphics[];
extern const char lcdpico_display_graphics_end[];

#define ROMIMAGESIZE(img) ((uint32_t)((img ## _end) - img))


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

#define RGB888_TO_RGB565(r,g,b) ((uint16_t)( ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3) ))

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
		uint16_t pixel = Black;
		if (d->iPixelType == PNG_PIXEL_TRUECOLOR) {
			int o = i * 3;
			pixel = RGB888_TO_RGB565(d->pPixels[o + 0], d->pPixels[o + 1], d->pPixels[o + 2]);
		}
		else if (d->iPixelType == PNG_PIXEL_TRUECOLOR_ALPHA) {
			int o = i * 4;
			pixel = RGB888_TO_RGB565(d->pPixels[o + 0], d->pPixels[o + 1], d->pPixels[o + 2]);
		}
		else if (d->iPixelType == PNG_PIXEL_GRAYSCALE) {
			int o = i / 8;
			int p = i % 8;
			pixel = ( (d->pPixels[o] << p) & 0x80 ? Black : *color );
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

	log_msg(LOG_INFO, "load_image_to_vmem(%p,%lu,%lx)", buf, len, addr);

	if (!(png = malloc(sizeof(PNGIMAGE))))
		return NULL;

	res = PNG_openRAM(png, (uint8_t*)buf, len, draw_cb);
	if (res == 0) {
		w = PNG_getWidth(png);
		h = PNG_getHeight(png);

		printf("%dx%d %dbpp alpha=%d (%u)\n", w, h, PNG_getBpp(png), PNG_hasAlpha(png), sizeof(PNGIMAGE));
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
	log_msg(LOG_INFO, "decode done");

	if (res == 0) {
		if ((img = malloc(sizeof(vmem_image_t)))) {
			img->base_addr = *addr;
			img->size = (uint32_t)w * h * 2;
			img->w = w;
			img->h = h;
			img->bpp = 16;

			*addr += img->size;
			printf("image: base_addr=%08lx, size=%lu, w=%u h=%u\n", img->base_addr, img->size, img->w, img->h);
		}
	}
	return img;
}

void display_init()
{
	uint16_t w = LCD_WIDTH;
	uint16_t h = LCD_HEIGHT;

	log_msg(LOG_NOTICE, "Initialize GPU...");
	lt7680_hw_reset();
	log_msg(LOG_NOTICE, "reset done");

	//st7701_read_id();
	if (st7701_init()) {
		log_msg(LOG_NOTICE, "LCD initialized");
	} else {
		log_msg(LOG_NOTICE, "LCD initialization failed!");
	}
	//sleep_ms(150);
	if (lt7680_system_check()) {
		if (lt7680_init()) {
			log_msg(LOG_NOTICE, "GPU intialized");
		} else {
			log_msg(LOG_NOTICE, "GPU initialization failed!");
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
		log_msg(LOG_INFO, "solid fill");
		lt7680_bte_solid_fill(LAYER1_START_ADDR, w, 0, 0, w, h, Black);
		log_msg(LOG_INFO, "display on");
		lt7680_display_on(true);
	} else {
		log_msg(LOG_NOTICE, "No GPU found!");
		return;
	}



	logo = load_image_to_vmem(lcdpico_hp_logo, ROMIMAGESIZE(lcdpico_hp_logo), Blue, &vmem_asset_ptr);
	log_msg(LOG_INFO, "foo1");
	lt7680_bte_memory_copy(0, w, (w - logo->w)/2, (h - logo->h) / 2,
			       logo->base_addr, logo->w, 0, 0,
			       0, 0, 0, 0,
			       logo->w, logo->h, 0x0c);
	log_msg(LOG_INFO, "foo2");

	disp = load_image_to_vmem(lcdpico_display_graphics, ROMIMAGESIZE(lcdpico_display_graphics), Blue, &vmem_asset_ptr);

#if 0
	vmem_image_t *test = load_image_to_vmem(lcdpico_boot_logo, ROMIMAGESIZE(lcdpico_boot_logo), Blue, &vmem_asset_ptr);
	sleep_ms(1000);
	log_msg(LOG_INFO, "start");
	lt7680_bte_memory_copy(0, w, 0, 0,
			       test->base_addr, test->w, 0, 0,
			       0, 0, 0, 0,
			       test->w, test->h, 0x0c);
	log_msg(LOG_INFO, "end");
#endif

	lt7680_bte_solid_fill(LAYER1_START_ADDR, w, 0, 0, w, h, Black);

#if 0
	for (int i = 0; i < 200; i++) {
		lt7680_bte_memory_copy(0, w, 50, 0,
				disp->base_addr, disp->w, (i % 4) * 190, 0,
				0, 0, 0, 0,
				190, 12*80, 0x0c);
		sleep_ms(100);
	}
#endif
}

void clear_display()
{
	uint16_t w = LCD_WIDTH;
	uint16_t h = LCD_HEIGHT;
	lt7680_bte_solid_fill(LAYER1_START_ADDR, w, 0, 0, w, h, Black);
}

void display_status(const struct fanpico_state *state,
	const struct fanpico_config *config)
{
	static int i = 0;
	uint16_t w = LCD_WIDTH;
	uint16_t h = LCD_HEIGHT;


	lt7680_bte_memory_copy(0, w, 50, 0,
			disp->base_addr, disp->w, (i % 4) * 190, 0,
			0, 0, 0, 0,
			190, 12*80, 0x0c);
	i++;
}

void display_message(int rows, const char **text_lines)
{
}


/* eof :-) */
