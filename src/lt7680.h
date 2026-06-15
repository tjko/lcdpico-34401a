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


#if 0
#define LCD_WIDTH   240
#define LCD_HEIGHT  960

#define LCD_VBPD		 11
#define LCD_VFPD	 	 5
#define LCD_VSPW		 5
#define LCD_HBPD		 128
#define LCD_HFPD		 5
#define LCD_HSPW	   	 5

#define LAYER1_START_ADDR  0
#define LAYER2_START_ADDR  460800
#define LAYER3_START_ADDR  921600
#else
#define LCD_WIDTH 320
#define LCD_HEIGHT  960

#define LCD_VBPD                 10
#define LCD_VFPD                 12
#define LCD_VSPW                 3
#define LCD_HBPD                 80
#define LCD_HFPD                 20
#define LCD_HSPW                 20

#define LAYER1_START_ADDR  0
#define LAYER2_START_ADDR  614400
#define LAYER3_START_ADDR  1288000
#endif



#define LCD_PCLK_FALLING_EDGE  0 // 1=Falling, 0=Raising
#define LCD_HSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_VSYNC_POLARITY     0 // 1=High, 0=Low
#define LCD_DE_POLARITY        1 // 1=Low, 0=High


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

#define SRR_REG       0x00
#define CCR_REG       0x01
#define MACR_REG      0x02
#define ICR_REG       0x03
#define MRWDP_REG     0x04
#define PLLLC1_REG    0x05
#define PLLLC2_REG    0x06
#define MPLLC1_REG    0x07
#define MPLLC2_REG    0x08
#define CPLLC1_REG    0x09
#define CPLLC2_REG    0x0a
#define INTEN_REG     0x0b
#define INTF_REG      0x0c
#define MINTFR_REG    0x0d
#define PUENR_REG     0x0e
#define PSFSR_REG     0x0f
#define MPWCTR_REG    0x10
#define PIPCDEP_REG   0x11
#define DPCR_REG      0x12
#define PCSR_REG      0x13
#define HDWR_REG      0x14
#define HDWRFTR_REG   0x15
#define HNDR_REG      0x16
#define HNDRFTR_REG   0x17
#define HSTR_REG      0x18
#define HPWR_REG      0x19
#define VHDR_REG      0x1a // 1A-1B
#define VNDR_REG      0x1c // 1C-1D
#define VSTR_REG      0x1e
#define VPWR_REG      0x1f
#define MISA_REG      0x20 // 20-23
#define MIW_REG       0x24 // 24-25
#define MWULX_REG     0x26 // 26-27
#define MWULY_REG     0x28 // 28-29

#define CVSSA_REG     0x50 // 50-53
#define CVSIMWTH_REG  0x54 // 54-55
#define AWULX_REG     0x56 // 56-57
#define AWULY_REG     0x58 // 58-59
#define AWWTH_REG     0x5a // 5A-5B
#define AWHT_REG      0x5c // 5C-5D
#define AW_COLOR_REG  0x5e
#define DLHSR_REG     0x68 // 68-89
#define DLVSR_REG     0x6a // 6A-6B
#define DLHER_REG     0x6c // 6C-6D
#define DLVER_REG     0x6e // 6E-6F
#define DTPH_REG      0x70 // 70-71
#define DTPV_REG      0x72 // 72-73
#define DCR1_REG      0x76
#define FGCR_REG      0xd2
#define FGCG_REG      0xd3
#define FGCB_REG      0xd4
#define BGCR_REG      0xd5
#define BGCG_REG      0xd6
#define BGCB_REG      0xd7
#define SDRAR_REG     0xe0
#define SDRMD_REG     0xe1
#define SDRREF_REG    0xe2 // E2-E3
#define SDR_TIMING1   0xe0
#define SDR_TIMING2   0xe1
#define SDR_TIMING3   0xe2
#define SDR_TIMING4   0xe3
#define SDRCR_REG     0xe4
#define I2CMCK_REG    0xe5 // E5-E6
#define I2CMTXR_REG   0xe7
#define I2CMRXR_REG   0xe7
#define I2CMCMD_REG   0xe9
#define GPIOAD_REG    0xf0
#define GPIOA_REG     0xf1
#define GPIOB_REG     0xf2
#define GPIOCD_REG    0xf3
#define GPIOC_REG     0xf4
#define GPIODD_REG    0xf5
#define GPIOD_REG     0xf6


void lt7680_hw_reset();
bool lt7680_system_check();
bool lt7680_init();
void lt7680_display_on();
void lt7680_setup();
void lt7680_set_fg_16bpp(uint16_t color);
void lt7680_set_bg_16bpp(uint16_t color);
void lt7680_draw_point1_xy(uint16_t x, uint16_t y);
void lt7680_draw_point2_xy(uint16_t x, uint16_t y);
void lt7680_draw_point3_xy(uint16_t x, uint16_t y);
void lt7680_draw_rect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, bool fill);

#endif /* _LT7680_H_ */
