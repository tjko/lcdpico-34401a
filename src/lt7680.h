/* lt7680.h
   Copyright (C) 2025 Timo Kokkonen <tjko@iki.fi>

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

#ifndef _LT7680_H_
#define _LT7680_H_

#include LCDPICO_LCD_HEADER


#define Black   0x0000
#define White   0xffff
#define Red     0xf800
#define Green   0x07e0
#define Blue    0x001f
#define Yellow  Red|Green
#define Cyan    Green|Blue
#define Purple  Red|Blue
#define LIGHTRED      0xfc10
#define LIGHTGREEN    0x87f0
#define LIGHTBLUE     0x841f
#define LIGHTYELLOW   0xfff0


/* LT7680 Registers */

#define SRR_REG         0x00
#define CCR_REG         0x01
#define MACR_REG        0x02
#define ICR_REG         0x03
#define MRWDP_REG       0x04
#define PLLLC1_REG      0x05
#define PLLLC2_REG      0x06
#define MPLLC1_REG      0x07
#define MPLLC2_REG      0x08
#define CPLLC1_REG      0x09
#define CPLLC2_REG      0x0a
#define INTEN_REG       0x0b
#define INTF_REG        0x0c
#define MINTFR_REG      0x0d
#define PUENR_REG       0x0e
#define PSFSR_REG       0x0f
#define MPWCTR_REG      0x10
#define PIPCDEP_REG     0x11
#define DPCR_REG        0x12
#define PCSR_REG        0x13
#define HDWR_REG        0x14
#define HDWRFTR_REG     0x15
#define HNDR_REG        0x16
#define HNDRFTR_REG     0x17
#define HSTR_REG        0x18
#define HPWR_REG        0x19
#define VHDR_REG        0x1a // 16bit
#define VNDR_REG        0x1c // 16bit
#define VSTR_REG        0x1e
#define VPWR_REG        0x1f
#define MISA_REG        0x20 // 32bit
#define MIW_REG         0x24 // 16bit
#define MWULX_REG       0x26 // 16bit
#define MWULY_REG       0x28 // 16bit
#define PWDULX_REG      0x2a // 16bit
#define PWDULY_REG      0x2c // 16bit
#define PISA_REG        0x2e // 32bit
#define PIW_REG         0x32 // 16bit
#define PWIULX_REG      0x34 // 16bit
#define PWIULY_REG      0x36 // 16bit
#define PWW_REG         0x38 // 16bit
#define PWH_REG         0x3a // 16bit
#define GTCCR_REG       0x3c
#define BTCR_REG        0x3d
#define CURHS_REG       0x3e
#define CURVS_REG       0x3f
#define GCHP_REG        0x40 // 16bit
#define GCVP_REG        0x42 // 16bit
#define GCC0_REG        0x44
#define GCC1_REG        0x45
#define CVSSA_REG       0x50 // 32bit
#define CVSIMWTH_REG    0x54 // 16bit
#define AWULX_REG       0x56 // 16bit
#define AWULY_REG       0x58 // 16bit
#define AWWTH_REG       0x5a // 16bit
#define AWHT_REG        0x5c // 16bit
#define AW_COLOR_REG    0x5e
#define CURH_REG        0x5f // 16bit
#define CURV_REG        0x61 // 16bit
#define F_CURX_REG      0x63 // 16bit
#define F_CURY_REG      0x65 // 16bit
#define DCR0_REG        0x67
#define DLHSR_REG       0x68 // 16bit
#define DLVSR_REG       0x6a // 16bit
#define DLHER_REG       0x6c // 16bit
#define DLVER_REG       0x6e // 16bit
#define DTPH_REG        0x70 // 16bit
#define DTPV_REG        0x72 // 16bit
#define DCR1_REG        0x76
#define ELL_A_REG       0x77
#define ELL_B_REG       0x79
#define DEHR_REG        0x7b // 16bit
#define DEVR_REG        0x7d // 16bit
#define PSCLR_REG       0x84
#define PMUXR_REG       0x85
#define PCFGR_REG       0x86
#define DZ_LENGTH_REG   0x87
#define TCMPB0_REG      0x88 // 16bit
#define TCNTB0_REG      0x8a // 16bit
#define TCMPB1_REG      0x8c // 16bit
#define TCNTB1_REG      0x8e // 16bit
#define BLT_CTRL0_REG   0x90
#define BLT_CTRL1_REG   0x91
#define BLT_COLR_REG    0x92
#define S0_STR_REG      0x93 // 32bit
#define S0_WTH_REG      0x97 // 16bit
#define S0_X_REG        0x99 // 16bit
#define S0_Y_REG        0x9b // 16bit
#define S1_STR_REG      0x9d // 32bit
#define S1_WTH_REG      0xa1 // 16bit
#define S1_X_REG        0xa3 // 16bit
#define S1_Y_REG        0xa5 // 16bit
#define DT_STR_REG      0xa7 // 32bit
#define DT_WTH_REG      0xab // 16bit
#define DT_X_REG        0xad // 16bit
#define DT_Y_REG        0xaf // 16bit
#define BLT_WTH_REG     0xb1 // 16bit
#define BLT_HIG_REG     0xb3 // 16bit
#define APB_CTR_REG     0xb5
#define DMA_CTRL_REG    0xb6
#define SFL_CTRL_REG    0xb7
#define SPIDR_REG       0xb8
#define SPIMCR2_REG     0xb9
#define SPIMSR_REG      0xba
#define SPI_DIVSOR_REG  0xbb
#define DMA_SSTR_REG    0xbc // 32bit
#define DMA_DX_REG      0xc0 // 16bit
#define DMA_DY_REG      0xc2 // 16bit
#define DMAW_WTH_REG    0xc6 // 16bit
#define DMAW_HIGH_REG   0xc8 // 16bit
#define DMA_SWTH_REG    0xca // 16bit
#define CCR0_REG        0xcc
#define CCR1_REG        0xcd
#define FLDR_REG        0xd0
#define F2FSSR_REG      0xd1
#define FGCR_REG        0xd2
#define FGCG_REG        0xd3
#define FGCB_REG        0xd4
#define BGCR_REG        0xd5
#define BGCG_REG        0xd6
#define CGRAM_STR0_REG  0xdb
#define BGCB_REG        0xd7
#define PMU_REG         0xdf
#define SDRAR_REG       0xe0
#define SDR_TIMING1     0xe0
#define SDRMD_REG       0xe1
#define SDR_TIMING2     0xe1
#define SDRREF_REG      0xe2 // 16bit
#define SDR_TIMING3     0xe2
#define SDR_TIMING4     0xe3
#define SDRCR_REG       0xe4
#define I2CMCK_REG      0xe5 // 16bit
#define I2CMTXR_REG     0xe7
#define I2CMRXR_REG     0xe7
#define I2CMCMD_REG     0xe9
#define I2CMST_REG      0xea
#define GPIOAD_REG      0xf0
#define GPIOA_REG       0xf1
#define GPIOB_REG       0xf2
#define GPIOCD_REG      0xf3
#define GPIOC_REG       0xf4
#define GPIODD_REG      0xf5
#define GPIOD_REG       0xf6



#define DT_COLOR_8bpp        0x00
#define DT_COLOR_16bpp       0x01
#define DT_COLOR_24bpp       0x02

#define S0_COLOR_8bpp        0x00
#define S0_COLOR_16bpp       0x01
#define S0_COLOR_24bpp       0x02

#define S1_COLOR_8bpp        0x00
#define S1_COLOR_16bpp       0x01
#define S1_COLOR_24bpp       0x02
#define S1_COLOR_const       0x03
#define S1_COLOR_alpha_8bpp  0x04
#define S1_COLOR_alpha_16bpp 0x05



void lt7680_cmd_write(uint8_t cmd);
void lt7680_data_write(uint8_t data);
void lt7680_write_pixels(uint8_t *buf, uint32_t len);

uint8_t lt7680_core_idle_wait();
uint8_t lt7680_wr_fifo_notfull_wait();
uint8_t lt7680_wr_fifo_empty_wait();


void lt7680_hw_reset();
bool lt7680_system_check();
bool lt7680_init();
void lt7680_display_on(bool display_on);
void lt7680_setup(uint16_t w, uint16_t h);
void lt7680_set_fg_16bpp(uint16_t color);
void lt7680_set_bg_16bpp(uint16_t color);
void lt7680_set_misa_addr(uint32_t addr);
void lt7680_set_graphics_addr(uint32_t addr);
void lt7680_set_graphics_xy(uint16_t x, uint16_t y);
void lt7680_set_text_xy(uint16_t x, uint16_t y);

void lt7680_set_canvas_addr(uint32_t addr);
void lt7680_set_canvas_width(uint16_t width);
void lt7680_set_active_window_xy(uint16_t x, uint16_t y);
void lt7680_set_active_window_wh(uint16_t w, uint16_t h);

void lt7680_set_s0_addr(uint32_t addr);
void lt7680_set_s0_width(uint16_t width);
void lt7680_set_s0_xy(uint16_t x, uint16_t y);
void lt7680_set_s1_addr(uint32_t addr);
void lt7680_set_s1_width(uint16_t width);
void lt7680_set_s1_xy(uint16_t x, uint16_t y);
void lt7680_set_dt_addr(uint32_t addr);
void lt7680_set_dt_width(uint16_t width);
void lt7680_set_dt_xy(uint16_t x, uint16_t y);

void lt7680_draw_point1_xy(uint16_t x, uint16_t y);
void lt7680_draw_point2_xy(uint16_t x, uint16_t y);
void lt7680_draw_point3_xy(uint16_t x, uint16_t y);
void lt7680_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool fill);

void lt7680_set_dt_color_depth(uint8_t mode);
void lt7680_set_s0_color_depth(uint8_t mode);
void lt7680_set_s1_color_depth(uint8_t mode);
void lt7680_set_bte_wh(uint16_t width, uint16_t height);
void lt7680_set_bte_mode(uint8_t rop, uint8_t op);
void lt7680_bte_on(bool bte_on);
void lt7680_bte_memory_copy(uint32_t d_addr, uint16_t d_w, uint16_t d_x, uint16_t d_y,
			uint32_t s0_addr, uint16_t s0_w, uint16_t s0_x, uint16_t s0_y,
			uint32_t s1_addr, uint16_t s1_w, uint16_t s1_x, uint16_t s1_y,
			uint16_t w, uint16_t h, uint8_t rop);

void lt7680_bte_solid_fill(uint32_t d_addr, uint16_t d_w, uint16_t d_x, uint16_t d_y, uint16_t w, uint16_t h, uint16_t color);

#endif /* _LT7680_H_ */
