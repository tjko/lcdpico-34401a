# credits.s
# Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This file is part of LcdPico.
#

#
# Stub for embedding credits.txt in the executable.
#

	.global lcdpico_credits_text
	.global lcdpico_credits_text_end

	.section .rodata

	.p2align 2
lcdpico_credits_text:
	.incbin	"credits.txt"
	.byte	 0x00
lcdpico_credits_text_end:

# eof
