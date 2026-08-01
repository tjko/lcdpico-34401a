/* sensors.c
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
#include <string.h>
#include <math.h>
#include <assert.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "lcd-pico.h"




double get_temperature(const struct system_config *config)
{
	uint8_t pin;
	uint32_t raw = 0;
	uint64_t start, end;
	double t, volt;
	int i;


	start = to_us_since_boot(get_absolute_time());

	pin = SENSOR_READ_ADC;
	adc_select_input(pin);
	for (i = 0; i < ADC_AVG_WINDOW; i++) {
		raw += adc_read();
	}
	raw /= ADC_AVG_WINDOW;
	volt = raw * ((double)config->adc_vref / ADC_MAX_VALUE);

	t = 27.0 - ((volt - 0.706) / 0.001721);

	end = to_us_since_boot(get_absolute_time());

	log_msg(LOG_DEBUG, "get_temperature(): raw=%u,  volt=%lf, temp=%lf (duration=%llu)",
		raw, volt, t, end - start);

	return t;
}


int read_temps(struct system_config *config)
{
	if (!config)
		return -1;

	for (int i = 0; i < VSENSOR_COUNT; i++) {
		struct vsensor_input *v = &config->vsensors[i];

		if (v->mode != VSMODE_INTERNAL)
			continue;

		absolute_time_t t = get_absolute_time();
		float temp = get_temperature(config);
		temp = temp * v->temp_coefficient + v->temp_offset;

		mutex_enter_blocking(config_mutex);
		config->vtemp[i] = temp;
		config->vpressure[i] = -1.0;
		config->vhumidity[i] = -1.0;
		config->vtemp_updated[i] = t;
		mutex_exit(config_mutex);
	}

	return 0;
}

