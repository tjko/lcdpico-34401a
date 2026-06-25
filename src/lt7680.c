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

#include "config.h"
#include "lt7680.h"

#define LT7680_DEBUG 0


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

#define SET_BIT(uint, bit) ((uint) |= (1u << (bit)))
#define CLR_BIT(uint, bit) ((uint) &= ~(1u << (bit)))
#define TGL_BIT(uint, bit) ((uint) ^= (1u << (bit)))
#define HAS_BIT(uint, bit) ((uint) & (1u << (bit)))


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

static inline uint32_t spi_read_register_u32(uint8_t reg)
{
	uint8_t val1, val2, val3, val4;

	val1 = spi_read_register(reg);
	val2 = spi_read_register(reg + 1);
	val3 = spi_read_register(reg + 2);
	val4 = spi_read_register(reg + 3);

	return (((uint32_t)val4 << 24) | (val3 << 16) | (val2 << 8) | val1);
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

static inline void spi_write_register_u32(uint8_t reg, uint32_t val)
{
	spi_write_register(reg, val);
	spi_write_register(reg + 1, val >> 8);
	spi_write_register(reg + 2, val >> 16);
	spi_write_register(reg + 3, val >> 24);
}


void lt7680_cmd_write(uint8_t cmd)
{
	spi_cmd_write(cmd);
}

void lt7680_data_write(uint8_t data)
{
	spi_data_write(data);
}

void lt7680_write_pixels(uint8_t *buf, uint32_t len)
{
	uint32_t i = 0;

	while (i < len) {
		lt7680_wr_fifo_notfull_wait();
		spi_data_write(buf[i++]);
	}
	lt7680_wr_fifo_empty_wait();
}

uint8_t lt7680_core_idle_wait()
{
	uint8_t status;

	while ((status = spi_status_read()) & 0x08) {
		// wait...
	}

	return status;
}

uint8_t lt7680_wr_fifo_notfull_wait()
{
	uint8_t status;

	while ((status = spi_status_read()) & 0x80) {
		// wait...
	}

	return status;
}

uint8_t lt7680_wr_fifo_empty_wait()
{
	uint8_t status;

	while (((status = spi_status_read()) & 0x40) == 0) {
		// wait...
	}

	return status;
}


void lt7680_hw_reset()
{
	gpio_put(LCM_RESET_PIN, 0);
	sleep_ms(5); // RST must be low at least 256 (OSC) Clocks...
	gpio_put(LCM_RESET_PIN, 1);
	sleep_ms(10);
}


bool lt7680_system_check()
{
	int i = 0;
	int system_ok = 0;

	do {
		sleep_ms(1);
		uint8_t status = spi_status_read();
		//printf("status=%02x\n", status);
		if (!(status & 0x02)) {
			sleep_ms(2);
			uint8_t temp = spi_read_register(CCR_REG);
			//printf("status2=%02x\n", temp);
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

		if (system_ok == 0 && (i > 0 && (i % 5 == 0))) {
			//printf("reset\n");
			lt7680_hw_reset();
		}
	} while (system_ok == 0 && i < 20);

	return (system_ok ? true : false);
}


#define REFRESH_RATE 60

bool lt7680_init()
{
	uint8_t tmp = 0;

	uint32_t temp = (LCD_HBPD + LCD_HFPD + LCD_HSPW + LCD_WIDTH) *
		(LCD_VBPD + LCD_VFPD + LCD_VSPW + LCD_HEIGHT) * REFRESH_RATE;
	//printf("scan clock: %lu\n", temp);
	temp = (temp + 500000) / 1000000;
	//printf("%lu\n", temp);

	/* Initialize PLL */
	uint8_t lpllOD_sclk = 2;
	uint8_t lpllOD_cclk = 2;
	uint8_t lpllOD_mclk = 2;
	uint8_t lpllR_sclk = 5;
	uint8_t lpllR_cclk = 5;
	uint8_t lpllR_mclk = 5;
	uint8_t lpllN_sclk = temp; //25;
	uint8_t lpllN_cclk = temp*2; //100;
	uint8_t lpllN_mclk = temp*2; //100;

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
	sleep_ms(10);

	while (((tmp = spi_read_register(CCR_REG)) & 0x80) == 0) {
		printf("%02x (status=%02x)\n", tmp, spi_status_read());
		sleep_ms(1000);
	}





	/* SDRAM Initialization */

	//spi_write_register(SDRCR_REG, (1<<2));

	DUMP_REGISTER(SDRAR_REG);
	DUMP_REGISTER(SDRMD_REG);
	spi_write_register(SDRAR_REG, 0x29); // 16MB
	spi_write_register(SDRMD_REG, 0x03); // CAS=3

	DUMP_REGISTER_U16(SDRREF_REG);
	//uint32_t sdram_itv = ((64000000 / 8192) / (1000/lpllN_mclk)) - 2;
	uint32_t sdram_itv = ((64 * lpllN_mclk * 1000) / 4096) - 2;
	//printf("sdram_itv = %lu (%04lx)\n",sdram_itv,sdram_itv);
	spi_write_register_u16(SDRREF_REG, sdram_itv);
	DUMP_REGISTER(SDRCR_REG);
	spi_write_register(SDRCR_REG, 0x01);
	DUMP_REGISTER(SDRCR_REG);

	int count = 0;
	while (((tmp = spi_status_read()) & 0x04) == 0) {
		count++;
	}
	//printf("count=%d\n",count);
	sleep_ms(10);


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


void lt7680_display_on(bool display_on)
{
	uint8_t reg = spi_read_register(DPCR_REG);
	if (display_on)
		SET_BIT(reg, 6);
	else
		CLR_BIT(reg, 6);
//	reg |= (1 << 5); // Set Test Color Bar
	spi_write_register(DPCR_REG, reg);
	sleep_ms(1);
}

void lt7680_setup(uint16_t w, uint16_t h)
{
	uint32_t reg;

	/* Set Main Window 16bpp */
	reg = spi_read_register(MPWCTR_REG);
	set_bits_u32(&reg, 3, 2, 0x01); // 16bpp
	spi_write_register(MPWCTR_REG, reg);

	/* Set DT/S0/S1 to 16bpp */
	spi_write_register(BLT_COLR_REG, 0x25);

	/* Set Main Window Image Start Address */
	spi_write_register_u32(MISA_REG, LAYER1_START_ADDR);

	/* Set Main Window Image Width */
	spi_write_register_u16(MIW_REG, w);

	/* Set Main Window Start (X/Y) */
	spi_write_register_u16(MWULX_REG, 0);
	spi_write_register_u16(MWULY_REG, 0);

	/* Set Canvas Start Address */
	spi_write_register_u32(CVSSA_REG, 0);

	/* Set Canvas Image Width */
	spi_write_register_u16(CVSIMWTH_REG, w);

	/* Set Active Window Upper Left (X/Y) */
	spi_write_register_u16(AWULX_REG, 0);
	spi_write_register_u16(AWULY_REG, 0);

	/* Set Active Window Width/Height */
	spi_write_register_u16(AWWTH_REG, w);
	spi_write_register_u16(AWHT_REG, h);
}



void lt7680_set_fg_16bpp(uint16_t color)
{
	spi_write_register(FGCR_REG, color >> 8);
	spi_write_register(FGCG_REG, color >> 3);
	spi_write_register(FGCB_REG, color << 3);
}


void lt7680_set_bg_16bpp(uint16_t color)
{
	spi_write_register(BGCR_REG, color >> 8);
	spi_write_register(BGCG_REG, color >> 3);
	spi_write_register(BGCB_REG, color << 3);
}

void lt7680_set_misa_addr(uint32_t addr)
{
	spi_write_register_u32(MISA_REG, addr & 0xfffffffc);
}

void lt7680_set_graphics_addr(uint32_t addr)
{
	// use in linear mode only
	spi_write_register_u32(CURH_REG, addr);
}

void lt7680_set_graphics_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(CURH_REG, x & 0x1fff);
	spi_write_register_u16(CURV_REG, y & 0x1fff);
}

void lt7680_set_text_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(F_CURX_REG, x & 0x1fff);
	spi_write_register_u16(F_CURY_REG, y & 0x1fff);
}


void lt7680_set_canvas_addr(uint32_t addr)
{
	spi_write_register_u32(CVSSA_REG, addr & 0xfffffffc);
}

void lt7680_set_canvas_width(uint16_t width)
{
	spi_write_register_u32(CVSIMWTH_REG, width & 0x3ffc);
}

void lt7680_set_active_window_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(AWULX_REG, x & 0x1fff);
	spi_write_register_u16(AWULY_REG, y & 0x1fff);
}

void lt7680_set_active_window_wh(uint16_t w, uint16_t h)
{
	spi_write_register_u16(AWWTH_REG, w & 0x3fff);
	spi_write_register_u16(AWHT_REG, h & 0x3fff);
}


void lt7680_set_s0_addr(uint32_t addr)
{
	spi_write_register_u32(S0_STR_REG, addr);
}

void lt7680_set_s0_width(uint16_t width)
{
	spi_write_register_u32(S0_WTH_REG, width & 0x1ffc);
}

void lt7680_set_s0_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(S0_X_REG, x & 0x1fff);
	spi_write_register_u16(S0_Y_REG, y & 0x1fff);
}

void lt7680_set_s1_addr(uint32_t addr)
{
	spi_write_register_u32(S1_STR_REG, addr);
}

void lt7680_set_s1_width(uint16_t width)
{
	spi_write_register_u32(S1_WTH_REG, width & 0x1ffc);
}

void lt7680_set_s1_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(S1_X_REG, x & 0x1fff);
	spi_write_register_u16(S1_Y_REG, y & 0x1fff);
}

void lt7680_set_dt_addr(uint32_t addr)
{
	spi_write_register_u32(DT_STR_REG, addr);
}

void lt7680_set_dt_width(uint16_t width)
{
	spi_write_register_u32(DT_WTH_REG, width & 0x1ffc);
}

void lt7680_set_dt_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(DT_X_REG, x & 0x1fff);
	spi_write_register_u16(DT_Y_REG, y & 0x1fff);
}

void lt7680_set_dt_color_depth(uint8_t depth)
{
	uint32_t reg = spi_read_register(BLT_COLR_REG);
	set_bits_u32(&reg, 0, 2, depth);
	spi_write_register(BLT_COLR_REG, reg);
}

void lt7680_set_s0_color_depth(uint8_t depth)
{
	uint32_t reg = spi_read_register(BLT_COLR_REG);
	set_bits_u32(&reg, 6, 2, depth);
	spi_write_register(BLT_COLR_REG, reg);
}

void lt7680_set_s1_color_depth(uint8_t depth)
{
	uint32_t reg = spi_read_register(BLT_COLR_REG);
	set_bits_u32(&reg, 4, 3, depth);
	spi_write_register(BLT_COLR_REG, reg);
}

void lt7680_set_bte_wh(uint16_t width, uint16_t height)
{
	spi_write_register_u32(BLT_WTH_REG, width & 0x1fff);
	spi_write_register_u32(BLT_HIG_REG, height & 0x1fff);
}

void lt7680_set_bte_mode(uint8_t rop, uint8_t op)
{
	spi_write_register(BLT_CTRL1_REG, (rop << 4) | (op & 0x0f));
}

void lt7680_bte_on(bool bte_on)
{
	uint32_t reg = spi_read_register(BLT_CTRL0_REG);
	set_bits_u32(&reg, 4, 1, bte_on ? 1 : 0);
	spi_write_register(BLT_CTRL0_REG, reg);
}


void lt7680_bte_memory_copy(uint32_t d_addr, uint16_t d_w, uint16_t d_x, uint16_t d_y,
			uint32_t s0_addr, uint16_t s0_w, uint16_t s0_x, uint16_t s0_y,
			uint32_t s1_addr, uint16_t s1_w, uint16_t s1_x, uint16_t s1_y,
			uint16_t w, uint16_t h, uint8_t rop)
{
	lt7680_set_dt_addr(d_addr);
	lt7680_set_dt_width(d_w);
	lt7680_set_dt_xy(d_x, d_y);
	lt7680_set_s0_addr(s0_addr);
	lt7680_set_s0_width(s0_w);
	lt7680_set_s0_xy(s0_x, s0_y);
	lt7680_set_s1_addr(s1_addr);
	lt7680_set_s1_width(s1_w);
	lt7680_set_s1_xy(s1_x, s1_y);
	lt7680_set_bte_wh(w, h);
	lt7680_set_bte_mode(rop, 0x02);
	lt7680_bte_on(true);
	lt7680_core_idle_wait();
}

void lt7680_bte_solid_fill(uint32_t d_addr, uint16_t d_w, uint16_t d_x, uint16_t d_y, uint16_t w, uint16_t h, uint16_t color)
{
	lt7680_set_dt_addr(d_addr);
	lt7680_set_dt_width(d_w);
	lt7680_set_dt_xy(d_x, d_y);
	lt7680_set_bte_wh(w, h);
	lt7680_set_fg_16bpp(color);
	lt7680_set_bte_mode(0, 0x0c);
	lt7680_bte_on(true);
	lt7680_core_idle_wait();
}


void lt7680_draw_point1_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(DLHSR_REG, x);
	spi_write_register_u16(DLVSR_REG, y);
}

void lt7680_draw_point2_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(DLHER_REG, x);
	spi_write_register_u16(DLVER_REG, y);
}

void lt7680_draw_point3_xy(uint16_t x, uint16_t y)
{
	spi_write_register_u16(DTPH_REG, x);
	spi_write_register_u16(DTPV_REG, y);
}

void lt7680_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool fill)
{
	lt7680_draw_point1_xy(x1, y1);
	lt7680_draw_point2_xy(x2, y2);
	lt7680_core_idle_wait();
	spi_write_register(DCR1_REG, (fill ?  0xe0 : 0xa0));
	lt7680_core_idle_wait();
}


/* eof */
