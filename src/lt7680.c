/* lt7680.c
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

#include "lt7680.h"
#include "config.h"

#define LT7680_DEBUG 2


#if LT7680_DEBUG > 0
#define DUMP_REGISTER(reg) {				\
		uint8_t val = spi_read_register(reg);	\
		printf("REG[%02x]: %02x\n", reg, val);	\
	}
#define DUMP_REGISTER_U16(reg) {					\
		uint16_t val = spi_read_register_u16(reg);		\
		printf("REG[%02x-%02x]: %04x (%d)\n", reg, reg + 1, val, val); \
	}
#else
#define DUMP_REGISTER(reg) {}
#define DUMP_REGISTER_U16(reg) {}
#endif

#define SET_CS(cs)  gpio_put(LCM_CS_PIN, cs)


static inline void set_bits_u32(uint32_t *var, uint8_t bit, uint8_t len, uint32_t val)
{
	uint32_t mask = (1 << len) - 1;
	uint8_t shift = bit - (len - 1);
	*var = (*var & ~(mask << shift)) | ((val & mask) << shift);
}

static inline uint8_t spi_rw_byte(uint8_t byte)
{
	uint8_t r = 0;
	int res = spi_write_read_blocking(LCM_SPI_HW, &byte, &r, 1);
	if (res != 1) {
#if LT7680_DEBUG > 0
		printf("spi_rw_byte(%02x): failed %d (%02x)\n", byte, res, r);
#endif
	}
	return r;
}

static inline void spi_cmd_write(uint8_t cmd)
{
	SET_CS(0);
	spi_rw_byte(0x00);
	spi_rw_byte(cmd);
	SET_CS(1);
}

static inline void spi_data_write(uint8_t data)
{
	SET_CS(0);
	spi_rw_byte(0x80);
	spi_rw_byte(data);
	SET_CS(1);
}

static inline void spi_data_write_pixel(uint16_t data)
{
	SET_CS(0);
	spi_rw_byte(0x80);
	spi_rw_byte(data);
	SET_CS(1);

	SET_CS(0);
	spi_rw_byte(0x80);
	spi_rw_byte(data >> 8);
	SET_CS(1);
}

static inline uint8_t spi_status_read()
{
	uint8_t val = 0;

	SET_CS(0);
	spi_rw_byte(0x40);
	val = spi_rw_byte(0x00);
	SET_CS(1);

#if LT7680_DEBUG > 1
	printf("spi_status_read(): %02x\n", val);
#endif

	return val;
}

static inline uint8_t spi_data_read()
{
	uint8_t r = 0;

	SET_CS(0);
	spi_rw_byte(0xc0);
	r = spi_rw_byte(0x00);
	SET_CS(1);

	return r;
}

static inline uint8_t spi_read_register(uint8_t reg)
{
	uint8_t val;

	spi_cmd_write(reg);
	val = spi_data_read();
#if LT7680_DEBUG > 1
	printf("spi_read_register(%02x): %02x\n", reg, val);
#endif
	return val;
}

static inline uint16_t spi_read_register_u16(uint8_t reg)
{
	uint8_t val1, val2;

	val1 = spi_read_register(reg);
	val2 = spi_read_register(reg + 1);

	return (((uint16_t)val2 << 8) | val1);
}

static inline void spi_write_register(uint8_t reg, uint8_t val)
{
	spi_cmd_write(reg);
	spi_data_write(val);
#if LT7680_DEBUG > 1
	printf("spi_write_register(%02x): %02x\n", reg, val);
#endif
}

static inline void spi_write_register_u16(uint8_t reg, uint16_t val)
{
	spi_write_register(reg, val);
	spi_write_register(reg + 1, val >> 8);
}

void lt7680_hw_reset()
{
	printf("hw reset\n");
	gpio_put(LCM_RESET_PIN, 0);
	sleep_ms(100);
	gpio_put(LCM_RESET_PIN, 1);
	sleep_ms(200);
}


bool lt7680_system_check()
{
	int i = 0;
	int system_ok = 0;

	do {
		sleep_ms(1);
		uint8_t status = spi_status_read();
		printf("status=%02x\n", status);
		if (!(status & 0x02)) {
			sleep_ms(2);
			uint8_t temp = spi_read_register(CCR_REG);
			printf("status2=%02x\n", temp);
			if ((temp & 0x80)) {
				system_ok=1;
				i=0;
			} else {
				//sleep_ms(2);
				//spi_write_register(CCR_REG, 0x80);
				//spi_cmd_write(0x01);
				//sleep_ms(2);
				//spi_data_write(0x80);
			}
		}

		i++;

		if (system_ok == 0 && (i > 0 && i % 5)) {
			printf("reset\n");
			lt7680_hw_reset();
		}
	} while (system_ok == 0 && i < 100);

	return (system_ok ? true : false);
}


bool lt7680_init()
{
	uint8_t tmp = 0;

	/* Initialize PLL */
	uint8_t lpllOD_sclk = 2;
	uint8_t lpllOD_cclk = 2;
	uint8_t lpllOD_mclk = 2;
	uint8_t lpllR_sclk = 5;
	uint8_t lpllR_cclk = 5;
	uint8_t lpllR_mclk = 5;
	uint8_t lpllN_sclk = 25;
	uint8_t lpllN_cclk = 100;
	uint8_t lpllN_mclk = 100;

#if 0
	DUMP_REGISTER(PLLLC1_REG);
	DUMP_REGISTER(PLLLC2_REG);
	DUMP_REGISTER(MPLLC1_REG);
	DUMP_REGISTER(MPLLC2_REG);
	DUMP_REGISTER(CPLLC1_REG);
	DUMP_REGISTER(CPLLC2_REG);
#endif
	spi_write_register(PLLLC1_REG, (lpllOD_sclk << 6) | (lpllR_sclk << 1) | ((lpllN_sclk >> 8) & 0x01));
	spi_write_register(PLLLC2_REG, lpllN_sclk);
	spi_write_register(MPLLC1_REG, (lpllOD_mclk << 6) | (lpllR_mclk << 1) | ((lpllN_mclk >> 8) & 0x01));
	spi_write_register(MPLLC2_REG, lpllN_mclk);
	spi_write_register(CPLLC1_REG, (lpllOD_cclk << 6) | (lpllR_cclk << 1) | ((lpllN_cclk >> 8) & 0x01));
	spi_write_register(CPLLC2_REG, lpllN_cclk);

	/* Reconfigure PLL frequency */
	spi_write_register(SRR_REG, 0x80);
	sleep_ms(1);

	while (((tmp = spi_read_register(CCR_REG)) & 0x80) == 0) {
		printf("%02x (status=%02x)\n", tmp, spi_status_read());
		sleep_ms(1000);
	}


	/* SDRAM Initialization */
	DUMP_REGISTER(SDRAR_REG);
	DUMP_REGISTER(SDRMD_REG);
	spi_write_register(SDRAR_REG, 0x29); // 16MB
	spi_write_register(SDRMD_REG, 0x03); // CAS=3

	DUMP_REGISTER_U16(SDRREF_REG);
	uint16_t sdram_itv = ((64000000 / 8192) / (1000/60)) - 2;
	spi_write_register_u16(SDRREF_REG, sdram_itv);
	DUMP_REGISTER(SDRCR_REG);
	spi_write_register(SDRCR_REG, 0x01);
	DUMP_REGISTER(SDRCR_REG);

	int count = 0;
	while (((tmp = spi_status_read()) & 0x04) == 0) {
		count++;
	}
	printf("count=%d\n",count);



	/* Set Chip Configuration Register */
	uint32_t reg = spi_read_register(CCR_REG);
	set_bits_u32(&reg, 4, 2, 0x01); // [4-3] TFT Output Mode: 18bit
	set_bits_u32(&reg, 0, 1, 0x01); // Data Bus: 16bit
	spi_write_register(CCR_REG, reg);

	/* Set Memory Access Control Register */
	reg = 0x40; // 16bit I/F, Mem Read (Left->Right then Top->Bottom)
	spi_write_register(MACR_REG, reg);

	/* Set Input Control Register */
	reg = 0x00; // Graphics Mode, DRAM Image Buffer
	spi_write_register(ICR_REG, reg);

	/* Set Display Configuration Register (DPCD) */
	reg = 0x00; // HCAN (L->R), VSCAN (T->B), PDATA (RGB)
	set_bits_u32(&reg, 7, 1, LCD_PCLK_FALLING_EDGE); // PCLK Inversion
	spi_write_register(DPCR_REG, reg);

	/* Set Panel Scan Clock and Data Setting Register */
	reg = spi_read_register(PCSR_REG);
	reg = 0x00;
	set_bits_u32(&reg, 7, 1, LCD_HSYNC_POLARITY);
	set_bits_u32(&reg, 6, 1, LCD_VSYNC_POLARITY);
	set_bits_u32(&reg, 5, 1, LCD_DE_POLARITY);


	/* Set Horizontal Display Width */
	spi_write_register(HDWR_REG, (LCD_WIDTH / 8) - 1);
	spi_write_register(HDWRFTR_REG, (LCD_WIDTH % 8));

	/* Set Vertical Display Height */
	spi_write_register_u16(VHDR_REG, LCD_HEIGHT - 1);

	/* Set Horizontal Non-Display Period */
	spi_write_register(HNDR_REG, (LCD_HBPD < 8 ? 0 : (LCD_HBPD / 8) - 1));
	spi_write_register(HNDRFTR_REG, (LCD_HBPD % 8));

	/* HSYNC Start Position */
	spi_write_register(HSTR_REG, (LCD_HFPD < 8 ? 0 : (LCD_HFPD / 8) - 1));

	/* HSYNC Pulse Width */
	spi_write_register(HPWR_REG, (LCD_HSPW < 8 ? 0 : (LCD_HSPW / 8) - 1));

	/* Set Vertical Non-Display Period */
	spi_write_register_u16(VNDR_REG, LCD_VBPD - 1);

	/* Set VSYNC Start Position */
	spi_write_register(VSTR_REG, LCD_VFPD - 1);

	/* Set VSYNC Pulse Width */
	spi_write_register(VPWR_REG, LCD_VSPW - 1);


	/* Set Main Window 16bpp */
	reg = spi_read_register(MPWCTR_REG);
	set_bits_u32(&reg, 3, 2, 0x01); // 16bpp
	spi_write_register(MPWCTR_REG, reg);

	/* Color Depth of Canvas & Active Window */
	reg = spi_read_register(AW_COLOR_REG);
	set_bits_u32(&reg, 2, 1, 0x00); // Block Mode
	set_bits_u32(&reg, 1, 2, 0x01); // 16bpp
	spi_write_register(AW_COLOR_REG, reg);

	return true;
}

/* eof */
