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
#if 0
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


void st7701_read_id()
{
	uint8_t id = spi_cmd_read(0xda);
	printf("id1=%02x\n", id);
	id = spi_cmd_read(0xdb);
	printf("id2=%02x\n", id);
	id = spi_cmd_read(0xdc);
	printf("id3=%02x\n", id);
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


bool st7701b_init()
{
	//ST7701S+AUO4.58
	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x13);

	SPI_WriteComm (0xEF);
	SPI_WriteData (0x08);

	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x10);

	SPI_WriteComm (0xC0);
	SPI_WriteData (0x77);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xC1);
	SPI_WriteData (0x09);
	SPI_WriteData (0x08);

	SPI_WriteComm (0xC2);//inv
	SPI_WriteData (0x37);
	SPI_WriteData (0x02);

	SPI_WriteComm (0xC3); //????
	SPI_WriteData (0x80);
	SPI_WriteData (0x05);
	SPI_WriteData (0x0d);

	SPI_WriteComm (0xCC);
	SPI_WriteData (0x10);

	SPI_WriteComm (0xB0);
	SPI_WriteData (0x40);
	SPI_WriteData (0x14);
	SPI_WriteData (0x59);
	SPI_WriteData (0x10);
	SPI_WriteData (0x12);
	SPI_WriteData (0x08);
	SPI_WriteData (0x03);
	SPI_WriteData (0x09);
	SPI_WriteData (0x05);
	SPI_WriteData (0x1E);
	SPI_WriteData (0x05);
	SPI_WriteData (0x14);
	SPI_WriteData (0x10);
	SPI_WriteData (0x68);
	SPI_WriteData (0x33);
	SPI_WriteData (0x15);

	SPI_WriteComm (0xB1);
	SPI_WriteData (0x40);
	SPI_WriteData (0x08);
	SPI_WriteData (0x53);
	SPI_WriteData (0x09);
	SPI_WriteData (0x11);
	SPI_WriteData (0x09);
	SPI_WriteData (0x02);
	SPI_WriteData (0x07);
	SPI_WriteData (0x09);
	SPI_WriteData (0x1A);
	SPI_WriteData (0x04);
	SPI_WriteData (0x12);
	SPI_WriteData (0x12);
	SPI_WriteData (0x64);
	SPI_WriteData (0x29);
	SPI_WriteData (0x29);

	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x11);

	SPI_WriteComm (0xB0);
	SPI_WriteData (0x6D);  //6D

	SPI_WriteComm (0xB1);   //vcom
	SPI_WriteData (0x1D);

	SPI_WriteComm (0xB2);
	SPI_WriteData (0x87);

	SPI_WriteComm (0xB3);
	SPI_WriteData (0x80);

	SPI_WriteComm (0xB5);
	SPI_WriteData (0x49);

	SPI_WriteComm (0xB7);
	SPI_WriteData (0x85);

	SPI_WriteComm (0xB8);
	SPI_WriteData (0x20);

	SPI_WriteComm (0xC1);
	SPI_WriteData (0x78);

	SPI_WriteComm (0xC2);
	SPI_WriteData (0x78);

	SPI_WriteComm (0xD0);
	SPI_WriteData (0x88);

	SPI_WriteComm (0xE0);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x02);

	SPI_WriteComm (0xE1);
	SPI_WriteData (0x02);
	SPI_WriteData (0x8C);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x03);
	SPI_WriteData (0x8C);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);

	SPI_WriteComm (0xE2);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);
	SPI_WriteData (0xC9);
	SPI_WriteData (0x3C);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0xCA);
	SPI_WriteData (0x3C);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xE3);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);

	SPI_WriteComm (0xE4);
	SPI_WriteData (0x44);
	SPI_WriteData (0x44);

	SPI_WriteComm (0xE5);
	SPI_WriteData (0x05);
	SPI_WriteData (0xCD);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x01);
	SPI_WriteData (0xC9);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x07);
	SPI_WriteData (0xCF);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x03);
	SPI_WriteData (0xCB);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);

	SPI_WriteComm (0xE6);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x33);
	SPI_WriteData (0x33);

	SPI_WriteComm (0xE7);
	SPI_WriteData (0x44);
	SPI_WriteData (0x44);

	SPI_WriteComm (0xE8);
	SPI_WriteData (0x06);
	SPI_WriteData (0xCE);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x02);
	SPI_WriteData (0xCA);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x08);
	SPI_WriteData (0xD0);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);
	SPI_WriteData (0x04);
	SPI_WriteData (0xCC);
	SPI_WriteData (0x82);
	SPI_WriteData (0x82);

	SPI_WriteComm (0xEB);
	SPI_WriteData (0x08);
	SPI_WriteData (0x01);
	SPI_WriteData (0xE4);
	SPI_WriteData (0xE4);
	SPI_WriteData (0x88);
	SPI_WriteData (0x00);
	SPI_WriteData (0x40);

	SPI_WriteComm (0xEC);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);

	SPI_WriteComm (0xED);
	SPI_WriteData (0xFF);
	SPI_WriteData (0xF0);
	SPI_WriteData (0x07);
	SPI_WriteData (0x65);
	SPI_WriteData (0x4F);
	SPI_WriteData (0xFC);
	SPI_WriteData (0xC2);
	SPI_WriteData (0x2F);
	SPI_WriteData (0xF2);
	SPI_WriteData (0x2C);
	SPI_WriteData (0xCF);
	SPI_WriteData (0xF4);
	SPI_WriteData (0x56);
	SPI_WriteData (0x70);
	SPI_WriteData (0x0F);
	SPI_WriteData (0xFF);

	SPI_WriteComm (0xEF);
	SPI_WriteData (0x10);
	SPI_WriteData (0x0D);
	SPI_WriteData (0x04);
	SPI_WriteData (0x08);
	SPI_WriteData (0x3F);
	SPI_WriteData (0x1F);

	SPI_WriteComm (0xFF);
	SPI_WriteData (0x77);
	SPI_WriteData (0x01);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);
	SPI_WriteData (0x00);

	SPI_WriteComm (0x11);
	sleep_ms(120);

	SPI_WriteComm (0x35);
	SPI_WriteData (0x00);

	SPI_WriteComm (0x3A);
	SPI_WriteData (0x66);

	SPI_WriteComm (0x29);



	return true;
}

/* eof */
