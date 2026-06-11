/* bi_decl.c
   Copyright (C) 2021-2026 Timo Kokkonen <tjko@iki.fi>

   SPDX-License-Identifier: GPL-3.0-or-later

   This file is part of FanPico.

   FanPico is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   FanPico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FanPico. If not, see <https://www.gnu.org/licenses/>.
*/

#include "pico/stdlib.h"
#include "pico/binary_info.h"

#include "lcd-pico.h"

#define BI_TAG           0xf720
#define BOOT_SETTINGS    0x0001


void set_binary_info(struct fanpico_fw_settings *settings)
{
	bi_decl(bi_program_description("LCD-Pico-" LCDPICO_BOARD " Display Controller"));
	bi_decl(bi_program_version_string(LCDPICO_VERSION LCDPICO_BUILD_TAG));
	bi_decl(bi_program_build_date_string("__DATE__"));
	bi_decl(bi_program_url("https://kokkonen.net/lcd-pico/"));

	bi_decl(bi_program_feature_group(BI_TAG, BOOT_SETTINGS, "boot settings"));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, sysclock, 0));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, safemode, 0));
	bi_decl(bi_ptr_int32(BI_TAG, BOOT_SETTINGS, bootdelay, 0));
	settings->sysclock = sysclock;
	settings->safemode = safemode;
	settings->bootdelay = bootdelay;

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
	bi_decl(bi_1pin_with_name(TX_PIN, "TX (Serial) / MISO (SPI)"));
	bi_decl(bi_1pin_with_name(RX_PIN, "RX (Serial) / CS (SPI)"));
#endif
#if SDA_PIN >= 0
	bi_decl(bi_1pin_with_name(SDA_PIN, "SDA (I2C) / SCK (SPI)"));
	bi_decl(bi_1pin_with_name(SCL_PIN, "SCL (I2C) / MOSI (SPI)"));
#endif

}



