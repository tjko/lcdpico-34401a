/* httpd.c
   Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>

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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "pico/stdlib.h"
#include "cJSON.h"

#include "lcd-pico.h"


#define BUF_LEN 1024

u16_t csv_stats(char *insert, int insertlen, u16_t current_tag_part, u16_t *next_tag_part)
{
	const struct system_state *st = sys_state;
	static char *buf = NULL;
	static char *p;
	static u16_t part;
	static size_t buf_left;
	char row[128];
	double pwm;
	int i;
	size_t printed, count;

	if (current_tag_part == 0) {
		/* Generate 'output' into a buffer that then will be fed in chunks to LwIP... */
		if (!(buf = malloc(BUF_LEN)))
			return 0;
		buf[0] = 0;

		for (i = 0; i < VSENSOR_COUNT; i++) {
			pwm = 0; //sensor_get_duty(&cfg->vsensors[i].map, st->vtemp[i]);
			snprintf(row, sizeof(row), "vsensor%d,\"%s\",%.1lf,%.1lf,%.0f,%.0f\n",
				i+1,
				cfg->vsensors[i].name,
				st->vtemp[i],
				pwm,
				st->vhumidity[i],
				st->vpressure[i]);
			strncatenate(buf, row, BUF_LEN);
		}

		snprintf(row, sizeof(row), "vref1,\"ADC Vref\",%.4lf\n",
			cfg->adc_vref);
		strncatenate(buf, row, BUF_LEN);

		p = buf;
		buf_left = strlen(buf);
		part = 1;
	}

	/* Copy a part of the multi-part response into LwIP buffer ...*/
	count = (buf_left < insertlen - 1 ? buf_left : insertlen - 1);
	memcpy(insert, p, count);

	p += count;
	printed = count;
	buf_left -= count;

	if (buf_left > 0) {
		*next_tag_part = part++;
	} else {
		free(buf);
		buf = p = NULL;
	}

	return printed;
}


u16_t json_stats(char *insert, int insertlen, u16_t current_tag_part, u16_t *next_tag_part)
{
	const struct system_state *st = sys_state;
	cJSON *json = NULL;
	static char *buf = NULL;
	static char *p;
	static u16_t part;
	static size_t buf_left;
	int i;
	size_t printed, count;

	if (current_tag_part == 0) {
		/* Generate 'output' into a buffer that then will be fed in chunks to LwIP... */
		cJSON *array, *o;

		if (!(json = cJSON_CreateObject()))
			goto panic;


		/* Virtual Sensors */
		if (!(array = cJSON_CreateArray()))
			goto panic;
		for (i = 0; i < VSENSOR_COUNT; i++) {
			double pwm = 0; //sensor_get_duty(&cfg->vsensors[i].map, st->vtemp[i]);
			if (!(o = cJSON_CreateObject()))
				goto panic;

			cJSON_AddItemToObject(o, "sensor", cJSON_CreateNumber(i+1));
			cJSON_AddItemToObject(o, "name", cJSON_CreateString(cfg->vsensors[i].name));
			cJSON_AddItemToObject(o, "temperature", cJSON_CreateNumber(round_decimal(st->vtemp[i], 1)));
			cJSON_AddItemToObject(o, "duty_cycle", cJSON_CreateNumber(round_decimal(pwm, 1)));
			if (st->vhumidity[i] >= 0.0)
				cJSON_AddItemToObject(o, "humidity", cJSON_CreateNumber(round_decimal(st->vhumidity[i], 1)));
			if (st->vpressure[i] >= 0.0)
				cJSON_AddItemToObject(o, "pressure", cJSON_CreateNumber(round_decimal(st->vpressure[i], 1)));
			cJSON_AddItemToArray(array, o);
		}
		cJSON_AddItemToObject(json, "vsensors", array);

		if (!(buf = cJSON_Print(json)))
			goto panic;
		cJSON_Delete(json);
		json = NULL;

		p = buf;
		buf_left = strlen(buf);
		part = 1;
	}

	/* Copy a part of the multi-part response into LwIP buffer ...*/
	count = (buf_left < insertlen - 1 ? buf_left : insertlen - 1);
	memcpy(insert, p, count);

	p += count;
	printed = count;
	buf_left -= count;

	if (buf_left > 0) {
		*next_tag_part = part++;
	} else {
		free(buf);
		buf = p = NULL;
	}

	return printed;

panic:
	if (json)
		cJSON_Delete(json);
	return 0;
}


u16_t lcdpico_ssi_handler(const char *tag, char *insert, int insertlen,
			u16_t current_tag_part, u16_t *next_tag_part)
{
	const struct system_state *st = sys_state;
	size_t printed = 0;

	/* printf("ssi_handler(\"%s\",%lx,%d,%u,%u)\n", tag, (uint32_t)insert, insertlen, current_tag_part, *next_tag_part); */

	if (!strncmp(tag, "datetime", 8)) {
		time_t t;
		if (rtc_get_time(&t)) {
			time_t_to_str(insert, insertlen, t);
			printed = strnlen(insert, insertlen);
		}
	}
	else if (!strncmp(tag, "uptime", 6)) {
		uint32_t secs = to_us_since_boot(get_absolute_time()) / 1000000;
		uint32_t mins =  secs / 60;
		uint32_t hours = mins / 60;
		uint32_t days = hours / 24;

		printed = snprintf(insert, insertlen, "%lu days %02lu:%02lu:%02lu",
				days,
				hours % 24,
				mins % 60,
				secs % 60);
	}
	else if (!strncmp(tag, "watchdog", 8)) {
		printed = snprintf(insert, insertlen, "%s",
				(rebooted_by_watchdog ? "&#x2639;" : "&#x263a;"));
	}
	else if (!strncmp(tag, "model", 5)) {
		printed = snprintf(insert, insertlen, "%s", LCDPICO_MODEL);
	}
	else if (!strncmp(tag, "version", 5)) {
		printed = snprintf(insert, insertlen, "%s", LCDPICO_VERSION);
	}
	else if (!strncmp(tag, "name", 4)) {
		printed = snprintf(insert, insertlen, "%s", cfg->name);
	}
	else if (!strncmp(tag, "vsenrow", 7)) {
		uint8_t i = tag[7] - '1';
		if (i < VSENSOR_COUNT && cfg->http_vsensor_mask & (1 << i)) {
			char other[24], tmp[12];

			other[0] = 0;
			if (st->vhumidity[i] >= 0.0) {
				snprintf(tmp, sizeof(tmp), "%0.0f %%rh", st->vhumidity[i]);
				strncatenate(other, tmp, sizeof(other));
			}
			if (st->vpressure[i] >= 0.0) {
				snprintf(tmp, sizeof(tmp), "%0.0f hPa", st->vpressure[i]);
				if (strlen(other) > 0)
					strncatenate(other, ", ", sizeof(other));
				strncatenate(other, tmp, sizeof(other));
			}

			printed = snprintf(insert, insertlen,
					"<tr><td>%d<td>%s<td align=\"right\">%0.1f &#x2103;<td>%s",
					i + 1, cfg->vsensors[i].name, st->vtemp[i], other);
		}
	}
	else if (!strncmp(tag, "adcvref", 7)) {
		printed = snprintf(insert, insertlen, "<td>Vref<td>ADC<td align=\"right\">%0.4f V",
					cfg->adc_vref);
	}
	else if (!strncmp(tag, "csvstat", 7)) {
		printed = csv_stats(insert, insertlen, current_tag_part, next_tag_part);
	}
	else if (!strncmp(tag, "jsonstat", 8)) {
		printed = json_stats(insert, insertlen, current_tag_part, next_tag_part);
	}
	else if (!strncmp(tag, "refresh", 8)) {
		/* generate "random" refresh time for a page, to help spread out the load... */
		printed = snprintf(insert, insertlen, "%u", (uint)(30 + ((double)rand() / RAND_MAX) * 30));
	}


	/* Check if snprintf() output was truncated... */
	printed = (printed >= insertlen ? insertlen - 1 : printed);
	/* printf("printed=%u\n", printed); */

	return printed;
}

