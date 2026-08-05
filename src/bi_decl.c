/* bi_decl.c
   Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>

   SPDX-License-Identifier: GPL-3.0-or-later

   This file is part of Lcdpico.

   Lcdpico is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Lcdpico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Lcdpico. If not, see <https://www.gnu.org/licenses/>.
*/

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "lcd-pico.h"

#define BI_TAG           0xf720
#define BOOT_SETTINGS    0x0001


void set_binary_info(struct fw_settings *settings)
{
	bi_decl(bi_program_description("LCDpico-" LCDPICO_BOARD " Display Controller"));
	bi_decl(bi_program_version_string(LCDPICO_VERSION LCDPICO_BUILD_TAG));
	bi_decl(bi_program_build_date_string("__DATE__"));
	bi_decl(bi_program_url("https://kokkonen.net/lcdpico/"));

	bi_decl(bi_program_feature_group(BI_TAG, BOOT_SETTINGS, "boot settings"));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, sysclock, 0));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, safemode, 0));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, bootdelay, 0));
	settings->sysclock = sysclock;
	settings->safemode = safemode;
	settings->bootdelay = bootdelay;

	bi_decl(bi_block_device(
			BI_TAG,
			"btflashbank",
			XIP_BASE + PICO_FLASH_BANK_STORAGE_OFFSET,
			PICO_FLASH_BANK_TOTAL_SIZE,
			NULL,
			BINARY_INFO_BLOCK_DEV_FLAG_READ
			| BINARY_INFO_BLOCK_DEV_FLAG_WRITE
			| BINARY_INFO_BLOCK_DEV_FLAG_PT_NONE));

        bi_decl(bi_block_device(
			BI_TAG,
			"littlefs",
			XIP_BASE + FANPICO_FS_OFFSET,
			FANPICO_FS_SIZE,
			NULL,
			BINARY_INFO_BLOCK_DEV_FLAG_READ
			| BINARY_INFO_BLOCK_DEV_FLAG_WRITE
			| BINARY_INFO_BLOCK_DEV_FLAG_PT_NONE));


#ifndef LIB_PIC_CYW43_ARCH
#if LED_PIN >= 0
	bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED (output)"));
#endif
#endif

#if TX_PIN >= 0
	bi_decl(bi_1pin_with_name(TX_PIN, "TTL Serial: TX"));
	bi_decl(bi_1pin_with_name(RX_PIN, "TTL Serial: RX"));
#endif
#if SDA_PIN >= 0
	bi_decl(bi_1pin_with_name(SDA_PIN, "I2C: SDA"));
	bi_decl(bi_1pin_with_name(SCL_PIN, "I2C: SCL"));
#endif

#if LCD_CS_PIN >= 0
	bi_decl(bi_1pin_with_name(LCD_CS_PIN, "LCD SPI: CS"));
	bi_decl(bi_1pin_with_name(LCD_CLK_PIN, "LCD SPI: SCK"));
	bi_decl(bi_1pin_with_name(LCD_MOSI_PIN, "LCD SPI: TX"));
#if LCD_MISO_PIN >= 0
	bi_decl(bi_1pin_with_name(LCD_MISO_PIN, "LCD SPI: RX"));
#endif
#if LCD_RESET_PIN >= 0
	bi_decl(bi_1pin_with_name(LCD_RESET_PIN, "LCD Reset"));
#endif
#endif

#if LCM_CS_PIN >= 0
	bi_decl(bi_1pin_with_name(LCM_CS_PIN, "LCM SPI: CS"));
	bi_decl(bi_1pin_with_name(LCM_CLK_PIN, "LCM SPI: SCK"));
	bi_decl(bi_1pin_with_name(LCM_MOSI_PIN, "LCM SPI: TX"));
	bi_decl(bi_1pin_with_name(LCM_MISO_PIN, "LCM SPI: RX"));
	bi_decl(bi_1pin_with_name(LCM_RESET_PIN, "LCM Reset"));
	bi_decl(bi_1pin_with_name(LCM_INT_PIN, "LCM Interrupt"));
	bi_decl(bi_1pin_with_name(LCM_BL_PIN, "LCM Backlight (PWM)"));
#endif

#if CTP_RESET_PIN >= 0
	bi_decl(bi_1pin_with_name(CTP_RESET_PIN, "CTP Reset"));
#endif
#if CTP_INT_PIN >= 0
	bi_decl(bi_1pin_with_name(CTP_RESET_PIN, "CTP Interrupt"));
#endif

	bi_decl(bi_1pin_with_name(DO_PIN, "DMM: DO"));
	bi_decl(bi_1pin_with_name(DI_PIN, "DMM: DI"));
	bi_decl(bi_1pin_with_name(SCK_PIN, "DMM: SCK"));
	bi_decl(bi_1pin_with_name(INT_PIN, "DMM: INT"));
	bi_decl(bi_1pin_with_name(RST_PIN, "DMM: RST"));

}



