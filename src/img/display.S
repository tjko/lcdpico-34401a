# display.S
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

	.global lcdpico_display_graphics
	.global lcdpico_display_graphics_end
	.global lcdpico_indicator_graphics
	.global lcdpico_indicator_graphics_end

	.section .rodata

	.p2align 4
lcdpico_display_graphics:
	.incbin	DISPLAY_GRAPHICS
lcdpico_display_graphics_end:

	.p2align 4
lcdpico_indicator_graphics:
	.incbin	INDICATOR_GRAPHICS
lcdpico_indicator_graphics_end:



# eof
