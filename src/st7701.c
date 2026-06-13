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

#define ST7701_DEBUG 2


#define SET_CS(cs)  gpio_put(LCD_CS_PIN, cs)
#define SET_CLK(clk) gpio_put(LCD_CLK_PIN, clk)
#define SET_DI(di) gpio_put(LCD_MOSI_PIN, di)



static inline void spi_write_byte(uint8_t byte, bool command)
{
#if 1
	uint16_t data = byte | (command ? 0x000 : 0x100);
	//sleep_us(1);
	int res = spi_write16_blocking(LCD_SPI_HW, &data, 1);
	//sleep_us(1);
	if (res != 1) {
#if ST7701_DEBUG > 0
		printf("spi_write_byte(%02x,%d): %04x failed %d\n", byte, command, data, res);
#endif
	}

#else
	foo
	for (int i = 0; i < 8; i++) {
		SET_CLK(0);
		SET_DI((byte & 0x80) ? 1 : 0);
		SET_CLK(1);
		byte <<= 1;
	}
#endif
}

static inline void spi_cmd_write(uint8_t cmd)
{
	SET_CS(0);
	sleep_us(10);
	spi_write_byte(cmd, true);
	SET_CS(1);
	sleep_us(10);
}

static inline void spi_data_write(uint8_t data)
{
	SET_CS(0);
	sleep_us(10);
	spi_write_byte(data, false);
	SET_CS(1);
	sleep_us(10);
}


#define SPI_WriteComm spi_cmd_write
#define SPI_WriteData spi_data_write

bool st7701_init()
{

	SPI_WriteComm (0xff);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x13);
	SPI_WriteComm (0xef);
	SPI_WriteData (0x08);
	SPI_WriteComm (0xff);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x10);
	SPI_WriteComm (0xc0);
	SPI_WriteData (0x77);
	SPI_WriteData (0x00);
	SPI_WriteComm (0xc1);
	SPI_WriteData (0x11);
	SPI_WriteData (0x0c);
	SPI_WriteComm (0xc2);
	SPI_WriteData (0x07);
	SPI_WriteData (0x02);

 	SPI_WriteComm (0xC3);
	SPI_WriteData (0x80);
	SPI_WriteData (0x10);
	SPI_WriteData (0x10);

	SPI_WriteComm (0xcc);
	SPI_WriteData (0x30);
	SPI_WriteComm (0xB0);
	SPI_WriteData (0x06);
	SPI_WriteData (0xCF);
	SPI_WriteData (0x14);
	SPI_WriteData (0x0C);
	SPI_WriteData (0x0F);
	SPI_WriteData (0x03);
	SPI_WriteData (0x00);
	SPI_WriteData (0x0A);
	SPI_WriteData (0x07);
	SPI_WriteData (0x1B);
	SPI_WriteData (0x03);
	SPI_WriteData (0x12);
	SPI_WriteData (0x10);
	SPI_WriteData (0x25);
	SPI_WriteData (0x36);
	SPI_WriteData (0x1E);
	SPI_WriteComm (0xB1);
	SPI_WriteData (0x0C);
	SPI_WriteData (0xD4);
	SPI_WriteData (0x18);
	SPI_WriteData (0x0C);
	SPI_WriteData (0x0E);
	SPI_WriteData (0x06);
	SPI_WriteData (0x03);
	SPI_WriteData (0x06);
	SPI_WriteData (0x08);
	SPI_WriteData (0x23);
	SPI_WriteData (0x06);
	SPI_WriteData (0x12);
	SPI_WriteData (0x10);
	SPI_WriteData (0x30);
	SPI_WriteData (0x2F);
	SPI_WriteData (0x1F);
	SPI_WriteComm (0xff);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x11);
	SPI_WriteComm (0xb0);
	SPI_WriteData (0x73);
	SPI_WriteComm (0xb1);
	SPI_WriteData (0x7C);
	SPI_WriteComm (0xb2);
	SPI_WriteData (0x83);
	SPI_WriteComm (0xb3);
	SPI_WriteData (0x80);
	SPI_WriteComm (0xb5);
	SPI_WriteData (0x49);
	SPI_WriteComm (0xb7);
	SPI_WriteData (0x87);
	SPI_WriteComm (0xb8);
	SPI_WriteData (0x33);
	SPI_WriteComm (0xb9);
	SPI_WriteData (0x10);
	SPI_WriteData (0x1f);
	SPI_WriteComm (0xbb);
	SPI_WriteData (0x03);
	SPI_WriteComm (0xc1);
	SPI_WriteData (0x08);
	SPI_WriteComm (0xc2);
	SPI_WriteData (0x08);
	SPI_WriteComm (0xd0);
	SPI_WriteData (0x88);
	SPI_WriteComm (0xe0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x0c);
	SPI_WriteComm (0xe1);
	SPI_WriteData (0x05);
	SPI_WriteData (0x96);
	SPI_WriteData (0x07);
	SPI_WriteData (0x96);
	SPI_WriteData (0x06);
	SPI_WriteData (0x96);
	SPI_WriteData (0x08);
	SPI_WriteData (0x96);
	SPI_WriteData (0x00);
	SPI_WriteData (0x44);
	SPI_WriteData (0x44);
	SPI_WriteComm (0xe2);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x03);
	SPI_WriteData (0x03);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);
	SPI_WriteData (0x00);
	SPI_WriteComm (0xe3);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);
	SPI_WriteComm (0xe4);
	SPI_WriteData (0x44);
	SPI_WriteData (0x44);
	SPI_WriteComm (0xe5);
	SPI_WriteData (0x0d);
	SPI_WriteData (0xd4);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x0f);
	SPI_WriteData (0xd6);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x09);
	SPI_WriteData (0xd0);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x0b);
	SPI_WriteData (0xd2);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteComm (0xe6);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);
	SPI_WriteComm (0xe7);
	SPI_WriteData (0x44);
	SPI_WriteData (0x44);
	SPI_WriteComm (0xe8);
	SPI_WriteData (0x0e);
	SPI_WriteData (0xd5);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x10);
	SPI_WriteData (0xd7);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x0a);
	SPI_WriteData (0xd1);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteData (0x0c);
	SPI_WriteData (0xd3);
	SPI_WriteData (0x28);
	SPI_WriteData (0x8c);
	SPI_WriteComm (0xeb);
	SPI_WriteData (0x00);
	SPI_WriteData (0x01);
	SPI_WriteData (0xe4);
	SPI_WriteData (0xe4);
	SPI_WriteData (0x44);
	SPI_WriteData (0x00);
	SPI_WriteComm (0xed);
	SPI_WriteData (0xf3);
	SPI_WriteData (0xc1);
	SPI_WriteData (0xba);
	SPI_WriteData (0x0f);
	SPI_WriteData (0x66);
	SPI_WriteData (0x77);
	SPI_WriteData (0x44);
	SPI_WriteData (0x55);
	SPI_WriteData (0x55);
	SPI_WriteData (0x44);
	SPI_WriteData (0x77);
	SPI_WriteData (0x66);
	SPI_WriteData (0xf0);
	SPI_WriteData (0xab);
	SPI_WriteData (0x1c);
	SPI_WriteData (0x3f);
	SPI_WriteComm (0xef);
	SPI_WriteData (0x10);
	SPI_WriteData (0x0d);
	SPI_WriteData (0x04);
	SPI_WriteData (0x08);
	SPI_WriteData (0x3f);
	SPI_WriteData (0x1f);
	SPI_WriteComm (0xff);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x13);
	SPI_WriteComm (0xe8);
	SPI_WriteData (0x00);
	SPI_WriteData (0x0e);

	SPI_WriteComm (0xe8);
	SPI_WriteData (0x00);
	SPI_WriteData (0x0c);
	sleep_ms(10);

	SPI_WriteComm (0xe8);
	SPI_WriteData (0x40);
	SPI_WriteData (0x00);
	SPI_WriteComm (0xff);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);

	SPI_WriteComm (0x36);
	SPI_WriteData (0x00);
	SPI_WriteComm (0x3A);
	SPI_WriteData (0x66);

	SPI_WriteComm (0x11);
	sleep_ms(120);
	SPI_WriteComm (0x29);
	sleep_ms(20);

	return true;
}

/* eof */
