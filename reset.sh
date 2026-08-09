#!/bin/sh
#
# Reset Pico 2 using picoprobe
#

openocd -f interface/cmsis-dap.cfg \
        -c "adapter speed 5000" \
	-f target/rp2350.cfg \
	-c "init; reset; exit"


# eof :-)
