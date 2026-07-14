# default_config.S
# Copyright (C) 2021-2024 Timo Kokkonen <tjko@iki.fi>
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# This file is part of FanPico.
#

#include "config.h"

#
# Stub for embedding default_config.json in the executable.
#

	.global lcdpico_default_config
	.global lcdpico_default_config_end

	.section .rodata

	.p2align 2
lcdpico_default_config:
	.incbin	LCDPICO_DEFAULT_CONFIG
	.byte	 0x00
lcdpico_default_config_end:

# eof
