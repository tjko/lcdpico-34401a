# logo.S
# Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This file is part of LCD-Pico
#

	#include "config.h"
	#include LCDPICO_IMG_HEADER

#
# Stub for embedding default_config.json in the executable.
#

	.global lcdpico_boot_logo
	.global lcdpico_boot_logo_end
	.global lcdpico_hp_logo
	.global lcdpico_hp_logo_end

	.section .rodata

	.p2align 4
lcdpico_boot_logo:
	.incbin	BOOT_LOGO
lcdpico_boot_logo_end:


lcdpico_hp_logo:
	.incbin "img/logo.png"
lcdpico_hp_logo_end:


# eof
