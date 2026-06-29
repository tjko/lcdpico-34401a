/* st7701.c
   Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>

   SPDX-License-Identifier: GPL-3.0-or-later

   This file is part of LcdPico.

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
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#include "st7701.h"
#include "config.h"

#include LCDPICO_LCD_HEADER


#define ST7701_DEBUG 0
#define USE_BITBANG 0
#define MANUAL_CS 1

#define SET_CS(cs)  gpio_put(LCD_CS_PIN, cs)
#define SET_CLK(clk) gpio_put(LCD_CLK_PIN, clk)
#define SET_DI(di) gpio_put(LCD_MOSI_PIN, di)


static inline void spi_write_byte(uint8_t byte, bool command)
{
#if !USE_BITBANG
	uint16_t data = byte | (command ? 0x000 : 0x100);
	int res = spi_write16_blocking(SPI_INSTANCE(LCD_SPI_HW), &data, 1);
	if (res != 1) {
#if ST7701_DEBUG
		printf("spi_write_byte(%02x,%d): %04x failed %d\n", byte, command, data, res);
#endif
	}

#else
	SET_DI(command ? 0 : 1);
	sleep_us(2);
	SET_CLK(1);
	sleep_us(5);
	SET_CLK(0);
	sleep_us(5);

	for (int i = 0; i < 8; i++) {
		SET_CLK(0);
		SET_DI(((byte & 0x80) ? 1 : 0));
		sleep_us(2);
		SET_CLK(1);
		sleep_us(5);
		SET_CLK(0);
		sleep_us(5);
		byte <<= 1;
	}
	SET_DI(0);
#endif
}

static inline void spi_cmd_write(uint8_t cmd)
{
#if MANUAL_CS
	SET_CS(0);
#endif
	sleep_us(1);
	spi_write_byte(cmd, true);
#if MANUAL_CS
	SET_CS(1);
#endif
	sleep_us(1);
}

static inline void spi_data_write(uint8_t data)
{
#if MANUAL_CS
	SET_CS(0);
#endif
	sleep_us(1);
	spi_write_byte(data, false);
#if MANUAL_CS
	SET_CS(1);
#endif
	sleep_us(1);
}

#if USE_BIGBANG
static inline uint8_t spi_cmd_read(uint8_t cmd)
{
	uint8_t data = 0;

	SET_CS(0);
	//gpio_set_dir(LCD_MOSI_PIN, GPIO_OUT);
	sleep_us(10);

	SET_DI(0); // D/CX bit
	sleep_us(2);
	SET_CLK(1);
	sleep_us(5);
	SET_CLK(0);
	sleep_us(5);

	for (int i = 0; i < 8; i++) {
		SET_DI(((cmd & 0x80) ? 1 : 0));
		sleep_us(2);
		SET_CLK(1);
		sleep_us(5);
		SET_CLK(0);
		sleep_us(5);
		cmd <<= 1;
	}

	SET_DI(0);
	gpio_set_dir(LCD_MOSI_PIN, GPIO_IN);
	sleep_us(5);

	/* read response */
	for (int i = 0; i < 8; i++) {
		SET_CLK(1);
		sleep_us(5);
		data <<= 1;
		data |= (gpio_get(LCD_MOSI_PIN) ? 1 : 0);
		SET_CLK(0);
		sleep_us(5);

	}

	gpio_set_dir(LCD_MOSI_PIN, GPIO_OUT);
	SET_DI(0);
	SET_CS(1);
	sleep_us(10);
	return data;
}
#endif


void st7701_read_id()
{
#if USE_BITBANG
	uint8_t id = spi_cmd_read(0xda);
	printf("id1=%02x\n", id);
	id = spi_cmd_read(0xdb);
	printf("id2=%02x\n", id);
	id = spi_cmd_read(0xdc);
	printf("id3=%02x\n", id);
#endif
}



bool st7701_init()
{
	const unsigned char* init_data = PANEL_INIT;
	int i = 0;
	int len;

	while ((len = init_data[i]) > 0) {
		i++;
		if (len == LCD_DELAY) {
			/* Special delay command */
			sleep_ms(init_data[i]);
		} else {
			/* Send SPI command to panel */
			spi_cmd_write(init_data[i]);
			for (int d = 1; d < len; d++) {
				i++;
				spi_data_write(init_data[i]);
			}
		}
		i++;
	}

	return true;
}


/* eof */
