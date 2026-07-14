/* pwm.c
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
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "lcd-pico.h"

#define PWM_IN_CLOCK_DIVIDER 100
#define PWM_IN_SAMPLE_INTERVAL 10 /* milliseconds */



uint pwm_out_top = 0;
float pwm_in_count_rate = 0;

/* Set PMW output signal duty cycle.
 */
void set_pwm_duty_cycle(uint pin, float duty)
{
	uint level;

	if (duty >= 100.0) {
		level = pwm_out_top + 1;
	} else if (duty > 0.0) {
		level = (duty * (pwm_out_top + 1) / 100);
	} else {
		level = 0;
	}
	pwm_set_gpio_level(pin, level);
}


/* Initialize PWM hardware to generate 25kHz PWM signal on output pins.
 */
void setup_pwm_outputs()
{
	uint32_t sys_clock = clock_get_hz(clk_sys);
	pwm_config config = pwm_get_default_config();
	uint pwm_freq = 25000;
	uint slice_num;

	log_msg(LOG_NOTICE, "Initializing PWM outputs...");
	log_msg(LOG_NOTICE, "PWM Frequency: %0.2f kHz", pwm_freq / 1000.0);

	pwm_out_top = (sys_clock / pwm_freq / 2) - 1;  /* for phase-correct PWM signal */

	pwm_config_set_clkdiv(&config, 1);
	pwm_config_set_phase_correct(&config, 1);
	pwm_config_set_wrap(&config, pwm_out_top);

#if LCM_BL_PIN >= 0
	gpio_set_function(LCM_BL_PIN, GPIO_FUNC_PWM);
	slice_num = pwm_gpio_to_slice_num(LCM_BL_PIN);
	pwm_init(slice_num, &config, true);
#endif
}

