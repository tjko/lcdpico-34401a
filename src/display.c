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

#define ROMIMAGESIZE(img) ((uint32_t)((img ## _end) - img))

static int draw_cb(PNGDRAW *d)
{
	uint32_t *c = d->pUser;
#if 1
	if (d->y % 100 == 0) {
		printf("draw_cp: y=%d, width=%d, pitch=%d, pixeltype=%d bpp=%d alpha=%d\n",
			d->y, d->iWidth, d->iPitch, d->iPixelType, d->iBpp, d->iHasAlpha);
	}
#endif
	return 1;
}


int load_image_to_vmem(const char *buf, uint32_t len, uint32_t addr)
{
	PNGIMAGE *png;
	int res;
	uint32_t out = 0;

	log_msg(LOG_INFO, "load_image_to_vmem(%p,%lu,%lx)", buf, len, addr);

	if (!(png = malloc(sizeof(PNGIMAGE))))
		return -1;

	if ((res = PNG_openRAM(png, (uint8_t*)buf, len, draw_cb)))
		return res;

	printf("%dx%d %dbpp alpha=%d (%u)\n", PNG_getWidth(png), PNG_getHeight(png), PNG_getBpp(png), PNG_hasAlpha(png), sizeof(PNGIMAGE));
	PNG_setBuffer(png, NULL);
	printf("decode start\n");
	if ((res = PNG_decode(png, &out, 0))) {
		printf("decode failed: %d (%d)\n", res, PNG_TOO_BIG);
		return res;
	}

	printf("close\n");
	PNG_close(png);
	free(png);

	log_msg(LOG_INFO, "decode done");
	return 0;
}

void display_init()
{
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
		lt7680_setup();
		lt7680_display_on(false);
		lt7680_set_fg_16bpp(Red);
		lt7680_draw_rect(0,0,LCD_WIDTH,LCD_HEIGHT,true);
		lt7680_set_fg_16bpp(Green);
		lt7680_draw_rect(10,10,LCD_WIDTH-10,LCD_HEIGHT-10,true);
		lt7680_set_fg_16bpp(Black);
		lt7680_draw_rect(20,20,LCD_WIDTH-20,LCD_HEIGHT-20,true);
		lt7680_display_on(true);
	} else {
		log_msg(LOG_NOTICE, "No GPU found!");
		return;
	}


	uint32_t vmem_addr = 0;

	load_image_to_vmem(lcdpico_boot_logo, ROMIMAGESIZE(lcdpico_boot_logo), vmem_addr);
}

void clear_display()
{
#if LCD_DISPLAY
	if (cfg->spi_active)
		lcd_clear_display();
#endif
#if OLED_DISPLAY
	if (!cfg->spi_active)
		oled_clear_display();
#endif
}

void display_status(const struct fanpico_state *state,
	const struct fanpico_config *config)
{
#if LCD_DISPLAY
	if (cfg->spi_active)
		lcd_display_status(state, config);
#endif
#if OLED_DISPLAY
	if (!cfg->spi_active)
		oled_display_status(state, config);
#endif
}

void display_message(int rows, const char **text_lines)
{
#if LCD_DISPLAY
	if (cfg->spi_active)
		lcd_display_message(rows, text_lines);
#endif
#if OLED_DISPLAY
	if (!cfg->spi_active)
		oled_display_message(rows, text_lines);
#endif
}


/* eof :-) */
