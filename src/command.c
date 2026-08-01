/* command.c
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
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include "pico/bootrom.h"
#include "pico/util/datetime.h"
#include "pico/aon_timer.h"
#include "pico/rand.h"
#include "hardware/watchdog.h"
#include "cJSON.h"
#include "b64/ccommon.h"
#include "lcd-pico.h"
#include "command_util.h"
#include "pico_sensor_lib.h"
#include "decoder_34401a.h"
#include "lfs.h"
#ifdef WIFI_SUPPORT
#include "lwip/ip_addr.h"
#include "lwip/stats.h"
#include "wolfssl/version.h"
#include "wolfssh/version.h"
#include "pico_telnetd/util.h"
#include "util_net.h"
#endif
#include "util_rp2.h"


struct error_t {
	const char    *error;
	int            error_num;
};

/* For now, mimic some actual instrument error codes/responses... */
const struct error_t error_codes[] = {
	{ "No Error", 0 },
	{ "Command Error", -100 },
	{ "Syntax Error", -102 },
	{ "Undefined Header", -113 },
	{ NULL, 0 }
};

int last_error_num = 0;

const struct system_state *st = NULL;
struct system_config *conf = NULL;



/* Command functions */

int cmd_idn(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int i;
	pico_unique_board_id_t board_id;

	if (!query)
		return 1;

	printf("TJKO Industries,LCDPICO-%s,", LCDPICO_MODEL);
	pico_get_unique_board_id(&board_id);
	for (i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; i++)
		printf("%02x", board_id.id[i]);
	printf(",%s%s\n", LCDPICO_VERSION, LCDPICO_BUILD_TAG);

	return 0;
}

int cmd_exit(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

#if WIFI_SUPPORT
	telnetserver_disconnect();
	sshserver_disconnect();
#endif
	return 0;
}

int cmd_who(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

#if WIFI_SUPPORT
	telnetserver_who();
	sshserver_who();
#endif
	return 0;
}

int cmd_usb_boot(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	char buf[64];
	const char *msg[] = {
		"FIRMWARE UPGRADE MODE",
		"=====================",
		"Use file (.uf2):",
		buf,
		"",
		"Copy file to: RPI-RP2",
		"",
		"Press RESET to abort.",
	};

	if (query)
		return 1;

	snprintf(buf, sizeof(buf), " lcdpico-%s-%s", LCDPICO_MODEL, PICO_BOARD);
	display_message(8, msg);

	reset_usb_boot(0, 0);
	return 0; /* should never get this far... */
}

int cmd_board(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (cmd && !query)
		return 1;

	print_rp2_board_info();

	return 0;
}

int cmd_version(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	const char* credits = lcdpico_credits_text;

	if (cmd && !query)
		return 1;

	printf("LCDpico-%s v%s%s (%s; %s; SDK v%s; %s)\n\n",
		LCDPICO_MODEL,
		LCDPICO_VERSION,
		LCDPICO_BUILD_TAG,
		__DATE__,
		PICO_CMAKE_BUILD_TYPE,
		PICO_SDK_VERSION_STRING,
		PICO_BOARD);

	if (query) {
		printf("%s\n", credits);
#ifdef __GNUC__
		printf("Compiled with: GCC v%s\n", __VERSION__);
#endif
		printf("littlefs: %d.%d\n", LFS_VERSION_MAJOR, LFS_VERSION_MINOR);
		printf("cJSON: %d.%d.%d\n", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);
		printf("libb64: %d.%d\n", BASE64_VER_MAJOR, BASE64_VER_MINOR);
#ifdef WIFI_SUPPORT
		printf("wolfSSH: %s\n", LIBWOLFSSH_VERSION_STRING);
		printf("wolfSSL: %s\n", LIBWOLFSSL_VERSION_STRING);
#endif
		printf("\n");
	}

	return 0;
}

int cmd_led(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int mode;

	if (query) {
		printf("%d\n", conf->led_mode);
	} else if (str_to_int(args, &mode, 10)) {
		if (mode >= 0 && mode <= 2) {
			log_msg(LOG_NOTICE, "Set system LED mode: %d -> %d", conf->led_mode, mode);
			conf->led_mode = mode;
		}
	}
	return 0;
}

int cmd_backlight(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int res = uint8_setting(cmd, args, query, prev_cmd,
				&conf->bl_brightness, 0, 100, "Backlight Brightness");
	if (res == 0 && !query) {
		set_pwm_duty_cycle(LCM_BL_PIN, cfg->bl_brightness);
	}

	return res;
}

int cmd_vsensors(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	printf("%d\n", VSENSOR_COUNT);
	return 0;
}

int cmd_vsensors_sources(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	for (int i = 0; i < VSENSOR_COUNT; i++) {
		const struct vsensor_input *v = &conf->vsensors[i];
		printf("vsensor%d,%s", i + 1, vsmode2str(v->mode));
		switch (v->mode) {
		case VSMODE_MANUAL:
			printf(",%0.2f,%ld", v->default_temp, v->timeout);
			break;
		case VSMODE_INTERNAL:
			printf(",%0.2f,%0.5f", v->temp_offset, v->temp_coefficient);
			break;
		case VSMODE_I2C:
			printf(",0x%02x,%s", v->i2c_addr, i2c_sensor_type_str(v->i2c_type));
			break;
		default:
			for (int j = 0; j < VSENSOR_SOURCE_MAX_COUNT; j++) {
				if (v->sensors[j])
					printf(",%d", v->sensors[j]);
			}
			break;
		}
		printf("\n");
	}

	return 0;
}

int cmd_null(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	log_msg(LOG_INFO, "null command: %s %s (query=%d)", cmd, args, query);
	return 0;
}

int cmd_debug(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	printf("DMM Decoder:\n");
	printf("            sck gap: %lu us\n", st->dmm.dbg_sck_gap_us);
	printf("        max sck gap: %lu us\n", st->dmm.dbg_sck_gap_us_max);
	printf("           main gap: %lu us\n", st->dmm.dbg_main_gap_us);
	printf("       max main gap: %lu us\n", st->dmm.dbg_main_gap_us_max);
	printf("            any gap: %lu us\n", st->dmm.dbg_any_gap_us);
	printf("        max any gap: %lu us\n", st->dmm.dbg_any_gap_us_max);
	printf("         fifo level: %lu\n", st->dmm.dbg_fifo_level);
	printf("     max fifo level: %lu\n", st->dmm.dbg_fifo_level_max);
	printf(" byte overrun count: %lu\n", st->dmm.dbg_byte_overrun_count);
	printf(" mid byte gap count: %lu\n", st->dmm.dbg_mid_byte_gap_count);
	printf(" buf overflow count: %lu\n", st->dmm.dbg_buf_overflow_count);
	printf("      bad msg count: %lu\n", st->dmm.dbg_bad_msg_count);
	printf("  last mid byte gap: %lu\n", st->dmm.dbg_mid_byte_gap_last_us);
	printf("       last bad msg: %lu\n", st->dmm.dbg_bad_msg_last_us);
	printf("         last reset: %lu\n", st->dmm.dbg_last_reset_us);
	printf("           last int: %lu\n", st->dmm.dbg_last_int_us);
	printf("          last main: %lu\n", st->dmm.dbg_last_main_us);
	printf("           last any: %lu\n", st->dmm.dbg_last_any_us);

	printf("\nInterrupts:\n");
	printf("            DMM_INT: %lu\n", st->dmm.dbg_int_count);
	printf("            DMM_SCK: %lu\n", st->dmm.dbg_sck_count);
	printf("          DMM_RESET: %lu\n", st->dmm.dbg_reset_count);
	printf("            LCM_INT: %lu\n", st->lcm_int_count);
	printf("            CTP_INT: %lu\n", st->ctp_int_count);
	printf("\n");

	return 0;
}

int cmd_log_level(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int level = get_log_level();
	int new_level;
	const char *name, *new_name;

	name = log_priority2str(level);

	if (query) {
		if (name) {
			printf("%s\n", name);
		} else {
			printf("%d\n", level);
		}
	} else {
		if ((new_level = str2log_priority(args)) < 0)
			return 1;
		new_name = log_priority2str(new_level);

		log_msg(LOG_NOTICE, "Change log level: %s (%d) -> %s (%d)",
			(name ? name : ""), level, new_name, new_level);
		set_log_level(new_level);
	}
	return 0;
}

int cmd_syslog_level(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int level = get_syslog_level();
	int new_level;
	const char *name, *new_name;

	name = log_priority2str(level);

	if (query) {
		if (name) {
			printf("%s\n", name);
		} else {
			printf("%d\n", level);
		}
	} else {
		if ((new_level = str2log_priority(args)) < 0)
			return 1;
		new_name = log_priority2str(new_level);
		log_msg(LOG_NOTICE, "Change syslog level: %s (%d) -> %s (%d)",
			(name ? name : "N/A"), level, new_name, new_level);
		set_syslog_level(new_level);
	}
	return 0;
}

int cmd_echo(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->local_echo, "Command Echo");
}

int cmd_reset(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	const char *msg[] = {
		"    Rebooting...",
	};

	if (query)
		return 1;

	log_msg(LOG_ALERT, "Initiating reboot...");
	display_message(1, msg);
	update_persistent_memory();

	watchdog_disable();
	sleep_ms(500);
	watchdog_reboot(0, SRAM_END, 1);
	while (1);

	/* Should never get this far... */
	return 0;
}

int cmd_save_config(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;
	save_config();
	return 0;
}

int cmd_print_config(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;
	print_config();
	return 0;
}

int cmd_upload_config(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;
#if WATCHDOG_ENABLED
	watchdog_disable();
#endif
	upload_config();
#if WATCHDOG_ENABLED
	watchdog_enable(WATCHDOG_REBOOT_DELAY, 1);
#endif

	return 0;
}

int cmd_delete_config(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;
	delete_config();
	return 0;
}

int cmd_one(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		printf("1\n");
	return 0;
}

int cmd_zero(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		printf("0\n");
	return 0;
}

int cmd_annunciators(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	int count = 0;
	for(int i = 0; i < ANNUNCIATOR_COUNT; i++) {
		uint ann = (1 << i);

		if (st->dmm.ann_state & ann) {
			printf("%s%s", (count > 0 ? "," : ""), decoder34401_annunciator_name(i));
			count++;
		}

	}
	printf("\n");

	return 0;
}

int cmd_read(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	char *s = strdup((char*)st->dmm.last_reading);
	if (s) {
		printf("%s\n", trim_str(s));
		free(s);
	}

	return 0;
}

int cmd_main(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	char *s = strdup((char*)st->dmm.main);
	if (s) {
		printf("%s\n", trim_str(s));
		free(s);
	}

	return 0;
}

int cmd_vsensors_read(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int i;

	if (!query)
		return 1;

	for (i = 0; i < VSENSOR_COUNT; i++) {
		printf("vsensor%d,\"%s\",%.1lf,%.0f,%0.0f\n", i+1,
			conf->vsensors[i].name,
			st->vtemp[i],
			st->vhumidity[i],
			st->vpressure[i]);
	}

	return 0;
}


int cmd_vsensor_name(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return array_string_setting(cmd, args, query, prev_cmd, 0, conf->vsensors, VSENSOR_COUNT,
				sizeof(conf->vsensors[0]), offsetof(struct vsensor_input, name),
				sizeof(conf->vsensors[0].name),	"vensor%d: Name", NULL);
}

int cmd_vsensor_source(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	struct vsensor_input *v;
	int sensor, val, i;
	uint8_t vsmode;
	float default_temp;
	int timeout;
	char *tok, *saveptr, *param;
	int ret = 0;

	sensor = get_prev_cmd_index(prev_cmd, 0) - 1;
	if (sensor < 0 || sensor >= VSENSOR_COUNT)
		return 1;
	v = &conf->vsensors[sensor];

	if (query) {
		printf("%s", vsmode2str(v->mode));
		if (v->mode == VSMODE_MANUAL) {
			printf(",%0.2f,%ld", v->default_temp, v->timeout);
		} else if (v->mode == VSMODE_INTERNAL) {
			printf(",%0.2f,%0.5f", v->temp_offset, v->temp_coefficient);
		} else if (v->mode == VSMODE_I2C) {
			printf(",0x%02x,%s", v->i2c_addr, i2c_sensor_type_str(v->i2c_type));
		} else {
			for(i = 0; i < VSENSOR_SOURCE_MAX_COUNT; i++) {
				if (v->sensors[i])
					printf(",%d", v->sensors[i]);
			}
		}
		printf("\n");
	} else {
		ret = 2;
		if ((param = strdup(args)) == NULL)
			return 1;

		if ((tok = strtok_r(param, ",", &saveptr)) != NULL) {
			vsmode = str2vsmode(tok);
			if (vsmode == VSMODE_MANUAL) {
				tok = strtok_r(NULL, ",", &saveptr);
				if (str_to_float(tok, &default_temp)) {
					tok = strtok_r(NULL, ",", &saveptr);
					if (str_to_int(tok, &timeout, 10)) {
						if (timeout < 0)
							timeout = 0;
						log_msg(LOG_NOTICE, "vsensor%d: set source to %s,%0.2f,%d",
							sensor + 1,
							vsmode2str(vsmode),
							default_temp,
							timeout);
						v->mode = vsmode;
						v->default_temp = default_temp;
						v->timeout = timeout;
						ret = 0;
					}
				}
			} else if (vsmode == VSMODE_I2C) {
				tok = strtok_r(NULL, ",", &saveptr);
				if (str_to_int(tok, &val, 16)) {
					if (val > 0 && val < 128 && !i2c_reserved_address(val)) {
						tok = strtok_r(NULL, ",", &saveptr);
						uint type = get_i2c_sensor_type(tok);
						if (type > 0) {
							log_msg(LOG_NOTICE, "vsensor%d: set source to %s,0x%02x,%s",
								sensor + 1,
								vsmode2str(vsmode),
								val,
								i2c_sensor_type_str(type));
							v->mode = vsmode;
							v->i2c_type = type;
							v->i2c_addr = val;
							ret = 0;
						}
					}
				}
			} else if (vsmode == VSMODE_INTERNAL) {
				float temp_o, temp_c;
				tok = strtok_r(NULL, ",", &saveptr);
				if (str_to_float(tok, &temp_o)) {
					tok = strtok_r(NULL, ",", &saveptr);
					if (str_to_float(tok, &temp_c)) {
						if (temp_c != 0.0) {
							log_msg(LOG_NOTICE, "vsensor%d: set source to %s,%0.2f,%0.5f",
								sensor + 1,
								vsmode2str(vsmode),
								temp_o,
								temp_c);
							v->mode =vsmode;
							v->temp_offset = temp_o;
							v->temp_coefficient = temp_c;
						}
					}
				}
			} else {
				log_msg(LOG_NOTICE, "unknown source");
			}
		}
		free(param);
	}

	return ret;
}

int cmd_vsensor_temp(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int sensor;
	float d;

	if (!query)
		return 1;

	if (!strncasecmp(get_prev_cmd(prev_cmd, 0), "vsensor", 7)) {
		sensor = get_prev_cmd_index(prev_cmd, 0) - 1;
	} else {
		sensor = get_cmd_index(cmd) - 1;
	}

	if (sensor >= 0 && sensor < VSENSOR_COUNT) {
		d = st->vtemp[sensor];
		log_msg(LOG_DEBUG, "vsensor%d temperature = %fC", sensor + 1, d);
		printf("%.1f\n", d);
		return 0;
	}

	return 1;
}

int cmd_vsensor_humidity(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int sensor;
	float d;

	if (!query)
		return 1;

	sensor = get_prev_cmd_index(prev_cmd, 0) - 1;
	if (sensor >= 0 && sensor < VSENSOR_COUNT) {
		d = st->vhumidity[sensor];
		log_msg(LOG_DEBUG, "vsensor%d humidity = %f%%", sensor + 1, d);
		printf("%.1f\n", d);
		return 0;
	}

	return 1;
}

int cmd_vsensor_pressure(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int sensor;
	float d;

	if (!query)
		return 1;

	sensor = get_prev_cmd_index(prev_cmd, 0) - 1;
	if (sensor >= 0 && sensor < VSENSOR_COUNT) {
		d = st->vpressure[sensor];
		log_msg(LOG_DEBUG, "vsensor%d pressure = %fhPa", sensor + 1, d);
		printf("%.1f\n", d);
		return 0;
	}

	return 1;
}


int cmd_vsensor_write(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int sensor;
	float val;

	if (query)
		return 1;

	if (!strncasecmp(get_prev_cmd(prev_cmd, 0), "vsensor", 7)) {
		sensor = get_prev_cmd_index(prev_cmd, 0) - 1;
	} else {
		sensor = get_cmd_index(cmd) - 1;
	}

	if (sensor >= 0 && sensor < VSENSOR_COUNT) {
		if (conf->vsensors[sensor].mode == VSMODE_MANUAL) {
			if (str_to_float(args, &val)) {
				log_msg(LOG_INFO, "vsensor%d: write temperature = %fC", sensor + 1, val);
				conf->vtemp[sensor] = val;
				conf->vtemp_updated[sensor] = get_absolute_time();
				return 0;
			}
		} else {
			return 2;
		}
	}

	return 1;
}

int cmd_wifi(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
#ifdef WIFI_SUPPORT
		if (rp2_is_picow()) {
			printf("1\n");
			return 0;
		}
#endif
		printf("0\n");
		return 0;
	}
	return 1;
}

#ifdef WIFI_SUPPORT
int cmd_wifi_ip(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_change(cmd, args, query, prev_cmd, "IP", &conf->ip);
}

int cmd_wifi_netmask(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_change(cmd, args, query, prev_cmd, "Netmask", &conf->netmask);
}

int cmd_wifi_gateway(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_change(cmd, args, query, prev_cmd, "Default Gateway", &conf->gateway);
}

int cmd_wifi_dns(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_list_change(cmd, args, query, prev_cmd, "DNS Servers", conf->dns_servers,
			DNS_MAX_SERVERS);
}

int cmd_wifi_syslog_client(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,	&conf->syslog_active, "Syslog Client");
}

int cmd_wifi_syslog(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_change(cmd, args, query, prev_cmd, "Syslog Server", &conf->syslog_server);
}

int cmd_wifi_ntp_client(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,	&conf->ntp_active, "NTP Client");
}

int cmd_wifi_ntp(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return ip_list_change(cmd, args, query, prev_cmd, "NTP Servers", conf->ntp_servers,
			SNTP_MAX_SERVERS);
}

int cmd_wifi_mac(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		printf("%s\n", mac_address_str(net_state->mac));
		return 0;
	}
	return 1;
}

int cmd_wifi_ssid(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->wifi_ssid, sizeof(conf->wifi_ssid), "WiFi SSID", NULL);
}

int cmd_wifi_status(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		wifi_status();
		return 0;
	}
	return 1;
}

int cmd_wifi_stats(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		stats_display();
		return 0;
	}
	return 1;
}

int cmd_wifi_info(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		wifi_info_display();
		return 0;
	}
	return 1;
}

int cmd_wifi_rejoin(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query) {
		wifi_rejoin();
		return 0;
	}
	return 1;
}

int cmd_wifi_country(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->wifi_country, sizeof(conf->wifi_country),
			"WiFi Country", valid_wifi_country);
}

int cmd_wifi_password(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->wifi_passwd, sizeof(conf->wifi_passwd), "WiFi Password", NULL);
}

int cmd_wifi_hostname(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->hostname, sizeof(conf->hostname),
			"WiFi Hostname", valid_hostname);
}

int cmd_wifi_auth_mode(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	uint32_t type;

	if (query) {
		printf("%s\n", conf->wifi_auth_mode);
	} else {
		if (!wifi_get_auth_type(args, &type))
			return 1;
		if (strncmp(conf->wifi_auth_mode, args, sizeof(conf->wifi_auth_mode))) {
			log_msg(LOG_NOTICE, "WiFi Auth Type change %s --> %s",
				conf->wifi_auth_mode, args);
			strncopy(conf->wifi_auth_mode, args, sizeof(conf->wifi_auth_mode));
		}
	}

	return 0;
}

int cmd_wifi_mode(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint8_setting(cmd, args, query, prev_cmd,
			&conf->wifi_mode, 0, 1, "WiFi Mode");
}


#if TLS_SUPPORT
int cmd_tls_pkey(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	char *buf;
	uint32_t buf_len = 4096;
	uint32_t file_size;
	int incount = 1;
	int res = 0;

	if (query) {
		res = flash_read_file(&buf, &file_size, "key.pem");
		if (res == 0 && buf != NULL) {
			printf("%s\n", buf);
			free(buf);
			return 0;
		} else {
			printf("No private key present.\n");
		}
		return 2;
	}

	if ((buf = malloc(buf_len)) == NULL) {
		log_msg(LOG_ERR,"cmd_tls_pkey(): not enough memory");
		return 2;
	}
	buf[0] = 0;

#if WATCHDOG_ENABLED
	watchdog_disable();
#endif
	if (!strncasecmp(args, "DELETE", 7)) {
		res = flash_delete_file("key.pem");
		if (res == -2) {
			printf("No private key present.\n");
			res = 0;
		}
		else if (res) {
			printf("Failed to delete private key: %d\n", res);
			res = 2;
		} else {
			printf("Private key successfully deleted.\n");
		}
	}
	else {
		int v;
		if (str_to_int(args, &v, 10)) {
			if (v >= 1 && v <= 3)
				incount = v;
		}
		printf("Paste private key in PEM format:\n");
		for(int i = 0; i < incount; i++) {
			if (read_pem_file(buf, buf_len, 5000, true) != 1) {
				printf("Invalid private key!\n");
				res = 2;
				break;
			}
		}
		if (res == 0) {
			res = flash_write_file(buf, strlen(buf) + 1, "key.pem");
			if (res) {
				printf("Failed to save private key.\n");
				res = 2;
			} else {
				printf("Private key successfully saved. (length=%u)\n",
					strlen(buf));
				res = 0;
			}
		}
	}
#if WATCHDOG_ENABLED
	watchdog_enable(WATCHDOG_REBOOT_DELAY, 1);
#endif

	free(buf);
	return res;
}

int cmd_tls_cert(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	char *buf;
	uint32_t buf_len = 8192;
	uint32_t file_size;
	int res = 0;

	if (query) {
		res = flash_read_file(&buf, &file_size, "cert.pem");
		if (res == 0 && buf != NULL) {
			printf("%s\n", buf);
			free(buf);
			return 0;
		} else {
			printf("No certificate present.\n");
		}
		return 2;
	}

	if ((buf = malloc(buf_len)) == NULL) {
		log_msg(LOG_ERR,"cmd_tls_cert(): not enough memory");
		return 2;
	}
	buf[0] = 0;

#if WATCHDOG_ENABLED
	watchdog_disable();
#endif
	if (!strncasecmp(args, "DELETE", 7)) {
		res = flash_delete_file("cert.pem");
		if (res == -2) {
			printf("No certificate present.\n");
			res = 0;
		}
		else if (res) {
			printf("Failed to delete certificate: %d\n", res);
			res = 2;
		} else {
			printf("Certificate successfully deleted.\n");
		}
	}
	else {
		printf("Paste certificate in PEM format:\n");

		if (read_pem_file(buf, buf_len, 5000, false) != 1) {
			printf("Invalid private key!\n");
			res = 2;
		} else {
			res = flash_write_file(buf, strlen(buf) + 1, "cert.pem");
			if (res) {
				printf("Failed to save certificate.\n");
				res = 2;
			} else {
				printf("Certificate successfully saved. (length=%u)\n",
					strlen(buf));
				res = 0;
			}
		}
	}
#if WATCHDOG_ENABLED
	watchdog_enable(WATCHDOG_REBOOT_DELAY, 1);
#endif

	free(buf);
	return res;
}
#endif /* TLS_SUPPORT */

int cmd_ssh_auth(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->ssh_auth, "SSH Server Authentication");
}

int cmd_ssh_server(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,	&conf->ssh_active, "SSH Server");
}

int cmd_ssh_port(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint16_setting(cmd, args, query, prev_cmd,
			&conf->ssh_port, 0, 65535, "SSH Port");
}

int cmd_ssh_user(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->ssh_user, sizeof(conf->ssh_user),
			"SSH Username", NULL);
}

int cmd_ssh_pass(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		printf("%s\n", cfg->ssh_pwhash);
		return 0;
	}

	if (strlen(args) > 0) {
		strncopy(conf->ssh_pwhash, generate_sha512crypt_pwhash(args),
			sizeof(conf->ssh_pwhash));
	} else {
		conf->ssh_pwhash[0] = 0;
		log_msg(LOG_NOTICE, "SSH password removed.");
	}
	return 0;
}

int cmd_ssh_pkey(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	ssh_list_pkeys();
	return 0;
}

int cmd_ssh_pkey_create(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

	return ssh_create_pkey(args);
}

int cmd_ssh_pkey_del(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

	return ssh_delete_pkey(args);
}

int cmd_ssh_pubkey(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int count = 0;
	char tmp[255];

	if (!query)
		return 1;

	for (int i = 0; i < SSH_MAX_PUB_KEYS; i++) {
		struct ssh_public_key *k = &conf->ssh_pub_keys[i];

		if (k->pubkey_size == 0 || strlen(k->username) < 1)
			continue;
		printf("%d: %s, %s\n", ++count, k->username,
			ssh_pubkey_to_str(k, tmp, sizeof(tmp)));
	}
	if (count < 1) {
		printf("No SSH (authentication) public keys found.\n");
	}

	return 0;
}

int cmd_ssh_pubkey_add(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	struct ssh_public_key pubkey;
	char *s, *username, *pkey, *saveptr;
	int res = 0;

	if (query)
		return 1;

	if (!(s = strdup(args)))
		return 2;

	if ((username = strtok_r(s, " ", &saveptr))) {
		username = trim_str(username);
		if (strlen(username) < 1)
			username = NULL;
	}

	if ((pkey = strtok_r(NULL, ",", &saveptr))) {
		pkey = trim_str(pkey);
		if (str_to_ssh_pubkey(pkey, &pubkey))
			pkey = NULL;
	}

	if (username && pkey) {
		int idx = -1;

		/* Check for first available slot */
		for(int i = 0; i < SSH_MAX_PUB_KEYS; i++) {
			if (conf->ssh_pub_keys[i].pubkey_size == 0) {
				idx = i;
				break;
			}
		}

		if (idx < 0) {
			printf("Maximum number of public keys already added.\n");
			res = 2;
		} else {
			strncopy(pubkey.username, username, sizeof(pubkey.username));
			conf->ssh_pub_keys[idx] = pubkey;
			log_msg(LOG_INFO, "SSH Public key added: slot %d: %s (%s)\n", idx + 1,
				pubkey.type, pubkey.username);
		}
	}

	free(s);

	return res;
}

int cmd_ssh_pubkey_del(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	int idx;

	if (query)
		return 1;

	if (!str_to_int(args, &idx, 10))
		return 2;
	if (idx < 1 || idx > SSH_MAX_PUB_KEYS)
		return 2;

	idx--;
	if (conf->ssh_pub_keys[idx].pubkey_size > 0) {
		log_msg(LOG_INFO, "SSH Public key deleted: slot %d: %s:%s (%s)\n", idx + 1,
			conf->ssh_pub_keys[idx].username,
			conf->ssh_pub_keys[idx].type,
			conf->ssh_pub_keys[idx].name);
		memset(&conf->ssh_pub_keys[idx], 0, sizeof(struct ssh_public_key));
	}

	return 0;
}

int cmd_ssh_acls(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return acl_list_change(cmd, args, query, prev_cmd, "SSH Server ACLs", conf->ssh_acls,
		SSH_MAX_ACL_ENTRIES);
}

int cmd_telnet_auth(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->telnet_auth, "Telnet Server Authentication");
}

int cmd_telnet_rawmode(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->telnet_raw_mode, "Telnet Server Raw Mode");
}

int cmd_telnet_server(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->telnet_active, "Telnet Server");
}

int cmd_telnet_port(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint16_setting(cmd, args, query, prev_cmd,
			&conf->telnet_port, 0, 65535, "Telnet Port");
}

int cmd_telnet_user(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->telnet_user, sizeof(conf->telnet_user),
			"Telnet Username", NULL);
}

int cmd_telnet_pass(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query) {
		printf("%s\n", cfg->telnet_pwhash);
		return 0;
	}

	if (strlen(args) > 0) {
		strncopy(conf->telnet_pwhash, generate_sha512crypt_pwhash(args),
			sizeof(conf->telnet_pwhash));
	} else {
		conf->telnet_pwhash[0] = 0;
		log_msg(LOG_NOTICE, "Telnet password removed.");
	}
	return 0;
}

int cmd_telnet_acls(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return acl_list_change(cmd, args, query, prev_cmd, "Telnet Server ACLs",
			conf->telnet_acls, TELNET_MAX_ACL_ENTRIES);
}


int cmd_http_server(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bool_setting(cmd, args, query, prev_cmd,
			&conf->http_active, "HTTP Server");
}

int cmd_http_port(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint16_setting(cmd, args, query, prev_cmd,
			&conf->http_port, 0, 65535, "HTTP Server Port");
}

int cmd_http_tlsport(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint16_setting(cmd, args, query, prev_cmd,
			&conf->https_port, 0, 65535, "HTTPS Server Port");
}

int cmd_http_mask_vsensor(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return bitmask16_setting(cmd, args, query, prev_cmd,
				&conf->http_vsensor_mask, VSENSOR_MAX_COUNT,
				1, "HTTP Virtual Sensor Mask");
}

#endif /* WIFI_SUPPOERT */

int cmd_time(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	struct timespec ts;
	time_t t;
	char buf[32];

	if (query) {
		if (aon_timer_is_running()) {
			aon_timer_get_time(&ts);
			time_t_to_str(buf, sizeof(buf), timespec_to_time_t(&ts));
			printf("%s\n", buf);
		}
		return 0;
	}

	if (str_to_time_t(args, &t)) {
		time_t_to_timespec(t, &ts);
		if (aon_timer_is_running()) {
			aon_timer_set_time(&ts);
		} else {
			aon_timer_start(&ts);
		}
		time_t_to_str(buf, sizeof(buf), t);
		log_msg(LOG_NOTICE, "Set system clock: %s", buf);
		return 0;
	}

	return 2;
}

int cmd_timezone(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->timezone, sizeof(conf->timezone), "Timezone", NULL);
}

int cmd_uptime(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	const struct persistent_memory_block *m = persistent_mem;
	struct timespec ts;
	uint64_t uptime;
	char buf[32];

	if (!query)
		return 1;


	if (aon_timer_is_running()) {
		if (aon_timer_get_time(&ts)) {
			time_t_to_str(buf, sizeof(buf), timespec_to_time_t(&ts));
			printf("%s ", buf + 11);
		}
	}

	uptime = to_us_since_boot(get_absolute_time());
	uptime_to_str(buf, sizeof(buf), uptime, false);
	printf("up %s%s\n", buf,
		(rebooted_by_watchdog ? " [rebooted by watchdog]" : ""));
	uptime_to_str(buf, sizeof(buf), uptime + m->total_uptime, false);
	printf("since cold boot %s (soft reset count: %lu)\n",
		buf, m->warmstart);
	return 0;
}

int cmd_err(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	for (int i = 0; error_codes[i].error != NULL; i++) {
		if (error_codes[i].error_num == last_error_num) {
			printf("%d,\"%s\"\n", last_error_num, error_codes[i].error);
			last_error_num = 0;
			return 0;
		}
	}
	printf("-1,\"Internal Error\"\n");
	return 0;
}

int cmd_name(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return string_setting(cmd, args, query, prev_cmd,
			conf->name, sizeof(conf->name), "System Name", NULL);
}


int cmd_lfs(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	size_t size, free, used, files, dirs;

	if (!query)
		return 1;
	if (flash_get_fs_info(&size, &free, &files, &dirs, NULL) < 0)
		return 2;

	used = size - free;
	printf("Filesystem size:                       %u\n", size);
	printf("Filesystem used:                       %u\n", used);
	printf("Filesystem free:                       %u\n", free);
	printf("Number of files:                       %u\n", files);
	printf("Number of subdirectories:              %u\n", dirs);

	return 0;
}

int cmd_lfs_del(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

	if (strlen(args) < 1)
		return 2;

	if (flash_delete_file(args))
		return 2;

	return 0;
}

int cmd_lfs_dir(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	flash_list_directory("/", true);

	return 0;
}

int cmd_lfs_format(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

	printf("Formatting flash filesystem...\n");
	if (flash_format(true))
		return 2;
	printf("Filesystem successfully formatted.\n");

	return 0;
}

int cmd_lfs_ren(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	char *saveptr, *oldname, *newname, *arg;
	int res = 2;

	if (query)
		return 1;

	if (!(arg = strdup(args)))
		return 2;

	oldname = strtok_r(arg, " \t", &saveptr);
	if (oldname && strlen(oldname) > 0) {
		newname = strtok_r(NULL, " \t", &saveptr);
		if (newname && strlen(newname) > 0) {
			if (!flash_rename_file(oldname, newname)) {
				res = 0;
			}
		}
	}
	free(arg);

	return res;
}

int cmd_lfs_copy(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	char *saveptr, *srcname, *dstname, *arg;
	int res = 2;

	if (query)
		return 1;

	if (!(arg = strdup(args)))
		return 2;

	srcname = strtok_r(arg, " \t", &saveptr);
	if (srcname && strlen(srcname) > 0) {
		dstname = strtok_r(NULL, " \t", &saveptr);
		if (dstname && strlen(dstname) > 0) {
			if (strcmp(srcname, dstname)) {
				if (!flash_copy_file(srcname, dstname, false)) {
					res = 0;
				}
			}
		}
	}
	free(arg);

	return res;
}

int cmd_flash(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	print_rp2040_flashinfo();
	return 0;
}


/* Disable optimizations since GCC 15.2 causes test for largest available memory block to fail... */
int __attribute__((optimize("O0"))) cmd_memory(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	const int TEST_MEM_SIZE = 1024 * 1024;
	int blocksize;

	if (query) {
		print_rp2_meminfo();
		printf("mallinfo:\n");
		print_mallinfo();
		return 0;
	}

	if (str_to_int(args, &blocksize, 10)) {
		if (blocksize < 512)
			blocksize = 512;
		if (blocksize > 8192)
			blocksize= 8192;
	} else {
		blocksize = 1024;
	}

	/* Test for largest available memory block... */
	void *buf = NULL;
	size_t bufsize = blocksize;
	do {
		if (!(buf = malloc(bufsize)))
			break;
		free(buf);
		bufsize += blocksize;
	} while (bufsize <= TEST_MEM_SIZE);
	printf("Largest available memory block:        %u bytes\n",
		bufsize - blocksize);

	/* Test how much memory available in 'blocksize' blocks... */
	int i = 0;
	int max = TEST_MEM_SIZE / blocksize + 1;
	size_t refbufsize = max * sizeof(void*);
	void **refbuf = malloc(refbufsize);
	if (refbuf) {
		memset(refbuf, 0, refbufsize);
		while (i < max) {
			if (!(refbuf[i] = malloc(blocksize)))
				break;
			i++;
		}
	}
	printf("Total available memory:                %u bytes\n",
		i * blocksize + refbufsize);
	if (refbuf) {
		i = 0;
		while (i < max && refbuf[i]) {
			free(refbuf[i++]);
		}
		free(refbuf);
	}
	return 0;
}

int cmd_memtest(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (query)
		return 1;

#if WATCHDOG_ENABLED
	watchdog_disable();
#endif
	rp2_memtest();
#if WATCHDOG_ENABLED
	watchdog_enable(WATCHDOG_REBOOT_DELAY, 1);
#endif

	return 0;
}


int cmd_i2c(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	display_i2c_status();
	return 0;
}

int cmd_i2c_scan(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	if (!query)
		return 1;

	scan_i2c_bus();
	return 0;
}

int cmd_i2c_speed(const char *cmd, const char *args, int query, struct prev_cmd_t *prev_cmd)
{
	return uint32_setting(cmd, args, query, prev_cmd,
			&conf->i2c_speed, 10000, 3400000, "I2C Bus Speed (Hz)");
}



const struct cmd_t lfs_commands[] = {
	{ "COPY",      4, NULL,              cmd_lfs_copy },
	{ "DELete",    3, NULL,              cmd_lfs_del },
	{ "DIRectory", 3, NULL,              cmd_lfs_dir },
	{ "FORMAT",    6, NULL,              cmd_lfs_format },
	{ "REName",    3, NULL,              cmd_lfs_ren },
	{ 0, 0, 0, 0 }
};

const struct cmd_t wifi_commands[] = {
#ifdef WIFI_SUPPORT
	{ "AUTHmode",  4, NULL,              cmd_wifi_auth_mode },
	{ "COUntry",   3, NULL,              cmd_wifi_country },
	{ "GATEway",   4, NULL,              cmd_wifi_gateway },
	{ "HOSTname",  4, NULL,              cmd_wifi_hostname },
	{ "IPaddress", 2, NULL,              cmd_wifi_ip },
	{ "MAC",       3, NULL,              cmd_wifi_mac },
	{ "NETMask",   4, NULL,              cmd_wifi_netmask },
	{ "DNS",       3, NULL,              cmd_wifi_dns },
	{ "NTPClient", 4, NULL,              cmd_wifi_ntp_client },
	{ "NTP",       3, NULL,              cmd_wifi_ntp },
	{ "MODE",      4, NULL,              cmd_wifi_mode },
	{ "PASSword",  4, NULL,              cmd_wifi_password },
	{ "REJOIN",    6, NULL,              cmd_wifi_rejoin },
	{ "SSID",      4, NULL,              cmd_wifi_ssid },
	{ "STATS",     5, NULL,              cmd_wifi_stats },
	{ "STATus",    4, NULL,              cmd_wifi_status },
	{ "INFO",      4, NULL,              cmd_wifi_info },
	{ "SYSLOGClient", 7, NULL,           cmd_wifi_syslog_client },
	{ "SYSLOG",    6, NULL,              cmd_wifi_syslog },
#endif
	{ 0, 0, 0, 0 }
};

#ifdef WIFI_SUPPORT
const struct cmd_t http_mask_commands[] = {
	{ "VSENSOR",   7, NULL,              cmd_http_mask_vsensor },
	{ 0, 0, 0, 0 }
};

const struct cmd_t http_commands[] = {
	{ "MASK",      4, http_mask_commands, NULL },
	{ "PORT",      4, NULL,              cmd_http_port },
	{ "TLSPORT",   7, NULL,              cmd_http_tlsport },
	{ "SERVer",    4, NULL,              cmd_http_server },
	{ 0, 0, 0, 0 }
};

const struct cmd_t ssh_pkey_commands[] = {
	{ "CREate",    3, NULL,              cmd_ssh_pkey_create },
	{ "DELete",    3, NULL,              cmd_ssh_pkey_del },
	{ "LIST",      4, NULL,              cmd_ssh_pkey },
	{ 0, 0, 0, 0 }
};

const struct cmd_t ssh_pubkey_commands[] = {
	{ "ADD",       3, NULL,              cmd_ssh_pubkey_add },
	{ "DELete",    3, NULL,              cmd_ssh_pubkey_del },
	{ "LIST",      4, NULL,              cmd_ssh_pubkey },
	{ 0, 0, 0, 0 }
};

const struct cmd_t ssh_commands[] = {
	{ "ACLs",      3, NULL,              cmd_ssh_acls },
	{ "AUTH",      4, NULL,              cmd_ssh_auth },
	{ "PORT",      4, NULL,              cmd_ssh_port },
	{ "SERVer",    4, NULL,              cmd_ssh_server },
	{ "PASSword",  4, NULL,              cmd_ssh_pass },
	{ "USER",      4, NULL,              cmd_ssh_user },
	{ "KEY",       3, ssh_pkey_commands, cmd_ssh_pkey },
	{ "PUBKEY",    6, ssh_pubkey_commands, cmd_ssh_pubkey },
	{ 0, 0, 0, 0 }
};

const struct cmd_t telnet_commands[] = {
	{ "ACLs",      3, NULL,              cmd_telnet_acls },
	{ "AUTH",      4, NULL,              cmd_telnet_auth },
	{ "PORT",      4, NULL,              cmd_telnet_port },
	{ "RAWmode",   3, NULL,              cmd_telnet_rawmode },
	{ "SERVer",    4, NULL,              cmd_telnet_server },
	{ "PASSword",  4, NULL,              cmd_telnet_pass },
	{ "USER",      4, NULL,              cmd_telnet_user },
	{ 0, 0, 0, 0 }
};

const struct cmd_t tls_commands[] = {
#if TLS_SUPPORT
	{ "CERT",      4, NULL,              cmd_tls_cert },
	{ "PKEY",      4, NULL,              cmd_tls_pkey },
#endif
	{ 0, 0, 0, 0 }
};
#endif



const struct cmd_t i2c_commands[] = {
	{ "SCAN",      4, NULL,              cmd_i2c_scan },
	{ "SPEED",     5, NULL,              cmd_i2c_speed },
	{ 0, 0, 0, 0 }
};


const struct cmd_t system_commands[] = {
	{ "BOARD",     5, NULL,              cmd_board },
	{ "ECHO",      4, NULL,              cmd_echo },
	{ "ERRor",     3, NULL,              cmd_err },
	{ "FLASH",     5, NULL,              cmd_flash },
	{ "I2C",       3, i2c_commands,      cmd_i2c },
	{ "LED",       3, NULL,              cmd_led },
	{ "BACKLight", 5, NULL,              cmd_backlight },
	{ "LFS",       3, lfs_commands,      cmd_lfs },
	{ "LOG",       3, NULL,              cmd_log_level },
	{ "MEMTEST",   7, NULL,              cmd_memtest },
	{ "MEMory",    3, NULL,              cmd_memory },
	{ "NAME",      4, NULL,              cmd_name },
	{ "SYSLOG",    6, NULL,              cmd_syslog_level },
	{ "TIMEZONE",  8, NULL,              cmd_timezone },
	{ "TIME",      4, NULL,              cmd_time },
	{ "UPGRADE",   7, NULL,              cmd_usb_boot },
	{ "UPTIme",    4, NULL,              cmd_uptime },
	{ "VERsion",   3, NULL,              cmd_version },
	{ "VSENSORS",  8, NULL,              cmd_vsensors },
	{ "WIFI",      4, wifi_commands,     cmd_wifi },
#if WIFI_SUPPORT
	{ "IFCONFIG",  8, NULL,              cmd_wifi_info },
	{ "HTTP",      4, http_commands,     NULL },
	{ "TELNET",    6, telnet_commands,   NULL },
	{ "SSH",       3, ssh_commands,      NULL },
	{ "TLS",       3, tls_commands,      NULL },
#endif
	{ 0, 0, 0, 0 }
};


const struct cmd_t vsensor_c_commands[] = {
	{ "NAME",        4, NULL,            cmd_vsensor_name },
	{ "SOUrce",      3, NULL,            cmd_vsensor_source },
	{ 0, 0, 0, 0 }
};

const struct cmd_t vsensors_c_commands[] = {
	{ "SOUrce",   3, NULL,               cmd_vsensors_sources },
	{ 0, 0, 0, 0 }
};

const struct cmd_t config_commands[] = {
	{ "DELete",    3, NULL,              cmd_delete_config },
	{ "Read",      1, NULL,              cmd_print_config },
	{ "SAVe",      3, NULL,              cmd_save_config },
	{ "UPLOAD",    6, NULL,              cmd_upload_config },
	{ "VSENSORS",  8, vsensors_c_commands, cmd_vsensors_sources },
	{ "VSENSOR",   7, vsensor_c_commands, NULL },
	{ 0, 0, 0, 0 }
};

const struct cmd_t vsensor_commands[] = {
	{ "HUMidity",  3, NULL,              cmd_vsensor_humidity },
	{ "PREssure",  3, NULL,              cmd_vsensor_pressure },
	{ "Read",      1, NULL,              cmd_vsensor_temp },
	{ "TEMP",      4, NULL,              cmd_vsensor_temp },
	{ 0, 0, 0, 0 }
};

const struct cmd_t measure_commands[] = {
	{ "Read",         1, NULL,              cmd_read },
	{ "Main",         1, NULL,              cmd_main },
	{ "ANNunciators", 3, NULL,              cmd_annunciators },
	{ "VSENSORS",     8, NULL,              cmd_vsensors_read },
	{ "VSENSOR",      7, vsensor_commands,  cmd_vsensor_temp },
	{ 0, 0, 0, 0 }
};

const struct cmd_t write_commands[] = {
	{ "VSENSOR",   7, NULL,              cmd_vsensor_write },
	{ 0, 0, 0, 0 }
};

const struct cmd_t commands[] = {
	{ "*CLS",      4, NULL,              cmd_null },
	{ "*ESE",      4, NULL,              cmd_null },
	{ "*ESR",      4, NULL,              cmd_zero },
	{ "*IDN",      4, NULL,              cmd_idn },
	{ "*OPC",      4, NULL,              cmd_one },
	{ "*RST",      4, NULL,              cmd_reset },
	{ "*SRE",      4, NULL,              cmd_zero },
	{ "*STB",      4, NULL,              cmd_zero },
	{ "*TST",      4, NULL,              cmd_zero },
	{ "*WAI",      4, NULL,              cmd_null },
	{ "CONFigure", 4, config_commands,   cmd_print_config },
	{ "DEBUG",     5, NULL,              cmd_debug },
	{ "EXIT",      4, NULL,              cmd_exit },
	{ "MEAsure",   3, measure_commands,  NULL },
	{ "SYStem",    3, system_commands,   NULL },
	{ "Read",      1, NULL,              cmd_read },
	{ "WHO",       3, NULL,              cmd_who },
	{ "WRIte",     3, write_commands,    NULL },
	{ 0, 0, 0, 0 }
};



/**
 * Process command string received from user.
 *
 * Splits command string into multiple commands by ';' character.
 * And executes each command using run_cmd() function.
 *
 * @param state Current system state.
 * @param config Current system configuration.
 * @param command command string
 */
void process_command(const struct system_state *state, struct system_config *config, char *command)
{
	char *saveptr, *cmd;
	struct prev_cmd_t cmd_stack;
	const struct cmd_t *cmd_level = commands;

	if (!state || !config || !command)
		return;

	st = state;
	conf = config;


	cmd = strtok_r(command, ";", &saveptr);
	while (cmd) {
		cmd = trim_str(cmd);
		log_msg(LOG_DEBUG, "command: '%s'", cmd);
		if (cmd && strlen(cmd) > 0) {
			cmd_stack.depth = 0;
			cmd_stack.cmds[0] = NULL;
			cmd_level = run_cmd(cmd, commands, cmd_level, &cmd_stack,
					&last_error_num);
		}
		cmd = strtok_r(NULL, ";", &saveptr);
	}
}

/**
 * Return last command status code.
 *
 * @return status code
 */
int last_command_status()
{
	return last_error_num;
}
