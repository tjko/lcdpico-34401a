/* config.c
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
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/mutex.h"
#include "cJSON.h"
#include "pico_sensor_lib.h"
#ifdef WIFI_SUPPORT
#include "lwip/ip_addr.h"
#endif

#include "lcd-pico.h"

/* Default configuration embedded using  default_config.s */
extern const char lcdpico_default_config[];


struct system_config system_config;
const struct system_config *cfg = &system_config;
auto_init_mutex(config_mutex_inst);
mutex_t *config_mutex = &config_mutex_inst;


int str2vsmode(const char *s)
{
	int ret = VSMODE_MANUAL;

	if (s) {
		if (!strncasecmp(s, "max", 3))
			ret = VSMODE_MAX;
		else if (!strncasecmp(s, "min", 3))
			ret = VSMODE_MIN;
		else if (!strncasecmp(s, "avg", 3))
			ret = VSMODE_AVG;
		else if (!strncasecmp(s, "delta", 5))
			ret = VSMODE_DELTA;
		else if (!strncasecmp(s, "onewire", 7))
			ret = VSMODE_ONEWIRE;
		else if (!strncasecmp(s, "i2c", 3))
			ret = VSMODE_I2C;
	}

	return ret;
}

const char* vsmode2str(enum vsensor_modes mode)
{
	if (mode == VSMODE_MAX)
		return "max";
	else if (mode == VSMODE_MIN)
		return "min";
	else if (mode == VSMODE_AVG)
		return "avg";
	else if (mode == VSMODE_DELTA)
		return "delta";
	else if (mode == VSMODE_ONEWIRE)
		return "onewire";
	else if (mode == VSMODE_I2C)
		return "i2c";

	return "manual";
}




#ifdef WIFI_SUPPORT
static void json2iplist(cJSON *item, ip_addr_t *list, uint32_t len)
{
	cJSON *o;
	char *val;
	ip_addr_t tmpip;
	int count = 0;

	for (int i = 0; i < len; i++)
		ip_addr_set_any(0, &list[i]);

	cJSON_ArrayForEach(o, item) {
		val = cJSON_GetStringValue(o);
		if (val && count < len) {
			if (ipaddr_aton(val, &tmpip)) {
				ip_addr_copy(list[count++], tmpip);
			}
		}
	}
}

static cJSON* iplist2json(const ip_addr_t *list, uint32_t len)
{
	cJSON *o;

	if ((o = cJSON_CreateArray()) == NULL)
		return NULL;

	for (int i = 0; i < len; i++) {
		if (!ip_addr_isany(&list[i]))
			cJSON_AddItemToArray(o, cJSON_CreateString(ipaddr_ntoa(&list[i])));
	}

	return o;
}

static void json2acllist(cJSON *item, acl_entry_t *list, uint32_t len)
{
	cJSON *o;
	char *val;
	ip_addr_t tmpip;
	int count = 0;
	char *s, *tok, *saveptr;
	int prefix;

	for (int i = 0; i < len; i++) {
		ip_addr_set_any(0, &list[i].ip);
		list[i].prefix = 0;
	}

	cJSON_ArrayForEach(o, item) {
		val = cJSON_GetStringValue(o);
		if (val && count < len && (s = strdup(val))) {
			tok = strtok_r(s, "/", &saveptr);
			if (tok && ipaddr_aton(tok, &tmpip)) {
				tok = strtok_r(NULL, "/", &saveptr);
				if (tok && str_to_int(tok, &prefix, 10)) {
					ip_addr_copy(list[count].ip, tmpip);
					list[count].prefix = clamp_int(prefix, 0,
								IP_IS_V6(tmpip) ? 128 : 32);
					count++;
				}
			}
			free(s);
		}
	}

}

static cJSON* acllist2json(const acl_entry_t *list, uint32_t len)
{
	cJSON *o;
	char tmp[128];
	int count = 0;

	if ((o = cJSON_CreateArray()) == NULL)
		return NULL;

	for (int i = 0; i < len; i++) {
		if (list[i].prefix > 0) {
			snprintf(tmp, sizeof(tmp), "%s/%u", ipaddr_ntoa(&list[i].ip),
				list[i].prefix);
			cJSON_AddItemToArray(o, cJSON_CreateString(tmp));
			count++;
		}
	}

	if (count < 1) {
		cJSON_Delete(o);
		o = NULL;
	}

	return o;
}

static void json2sshpubkeys(cJSON *list, struct ssh_public_key *keys)
{
	cJSON *r, *user, *pkey;
	struct ssh_public_key *k;
	int idx = 0;

	for (int i = 0; i < SSH_MAX_PUB_KEYS; i++) {
		keys[i].username[0] = 0;
		keys[i].type[0] = 0;
		keys[i].name[0] = 0;
		keys[i].pubkey_size = 0;
	}

	cJSON_ArrayForEach(r, list) {
		k = &keys[idx];
		user = cJSON_GetObjectItem(r, "user");
		pkey = cJSON_GetObjectItem(r, "pubkey");
		if (user && pkey) {
			if (str_to_ssh_pubkey(cJSON_GetStringValue(pkey), k) == 0) {
				strncopy(k->username, cJSON_GetStringValue(user),
					sizeof(k->username));
				idx++;
			}
		}
		if (idx >= SSH_MAX_PUB_KEYS)
			break;
	}
}

static cJSON* sshpubkeys2json(const struct ssh_public_key *keys)
{
	const size_t buf_len = 256;
	char *buf;
	int count = 0;
	cJSON *o, *r;

	if (!(buf = calloc(1, buf_len)))
		return NULL;

	if ((o = cJSON_CreateArray())) {
		for (int i = 0; i < SSH_MAX_PUB_KEYS; i++) {
			const struct ssh_public_key *k = &keys[i];
			if (k->pubkey_size > 0 && strlen(k->username) > 0) {
				if (ssh_pubkey_to_str(k, buf, buf_len)) {
					if ((r = cJSON_CreateObject())) {
						cJSON_AddItemToObject(r, "pubkey",
								cJSON_CreateString(buf));
						cJSON_AddItemToObject(r, "user",
								cJSON_CreateString(k->username));
						cJSON_AddItemToArray(o, r);
						count++;
					}
				}
			}
		}
	}

	free(buf);
	if (count < 1) {
		cJSON_Delete(o);
		o = NULL;
	}

	return o;
}
#endif

void clear_config(struct system_config *cfg)
{
	int i, j;
	struct sensor_input *s;
	struct vsensor_input *vs;

	memset(cfg, 0, sizeof(struct system_config));

	for (i = 0; i < SENSOR_MAX_COUNT; i++) {
		s = &cfg->sensors[i];

		s->name[0] = 0;
		s->type = TEMP_INTERNAL;
		s->thermistor_nominal = 0.0;
		s->beta_coefficient = 0.0;
		s->temp_nominal = 0.0;
		s->temp_offset = 0.0;
		s->temp_coefficient = 0.0;
//		s->map.points = 0;
		s->filter = FILTER_NONE;
		s->filter_ctx = NULL;
	}

	for (i = 0; i < VSENSOR_MAX_COUNT; i++) {
		vs = &cfg->vsensors[i];

		vs->name[0] = 0;
		vs->mode = VSMODE_MANUAL;
		vs->default_temp = 0.0;
		vs->timeout = 60;
		for (j = 0; j < VSENSOR_SOURCE_MAX_COUNT; j++)
			vs->sensors[j] = 0;
		vs->onewire_addr = 0;
		vs->i2c_type = 0;
		vs->i2c_addr = 0;
		vs->filter = FILTER_NONE;
		vs->filter_ctx = NULL;

		cfg->vtemp[i] = 0.0;
		cfg->vhumidity[i] = 0.0;
		cfg->vpressure[i] = 0.0;
		cfg->vtemp_updated[i] = from_us_since_boot(0);
		cfg->i2c_context[i] = NULL;
	}


	cfg->local_echo = false;
	cfg->i2c_speed = I2C_DEFAULT_SPEED;
	cfg->adc_vref = ADC_REF_VOLTAGE;
	cfg->led_mode = 0;
	cfg->bl_brightness = 100;
	strncopy(cfg->name, "lcdpico1", sizeof(cfg->name));
	strncopy(cfg->timezone, "", sizeof(cfg->timezone));
#ifdef WIFI_SUPPORT
	cfg->wifi_ssid[0] = 0;
	cfg->wifi_passwd[0] = 0;
	strncopy(cfg->wifi_auth_mode, "default", sizeof(cfg->wifi_auth_mode));
	cfg->wifi_mode = 0;
	cfg->hostname[0] = 0;
	strncopy(cfg->wifi_country, "XX", sizeof(cfg->wifi_country));
	for (int i = 0; i < DNS_MAX_SERVERS; i++) {
		ip_addr_set_any(0, &cfg->dns_servers[i]);
	}
	cfg->syslog_active = true;
	ip_addr_set_any(0, &cfg->syslog_server);
	cfg->ntp_active = true;
	for (int i = 0; i < SNTP_MAX_SERVERS; i++) {
		ip_addr_set_any(0, &cfg->ntp_servers[i]);
	}
	ip_addr_set_any(0, &cfg->ip);
	ip_addr_set_any(0, &cfg->netmask);
	ip_addr_set_any(0, &cfg->gateway);
	cfg->telnet_active = false;
	cfg->telnet_auth = true;
	cfg->telnet_raw_mode = false;
	cfg->telnet_port = 0;
	cfg->telnet_user[0] = 0;
	cfg->telnet_pwhash[0] = 0;
	cfg->ssh_active = false;
	cfg->ssh_auth = true;
	cfg->ssh_port = 0;
	cfg->ssh_user[0] = 0;
	cfg->ssh_pwhash[0] = 0;
	for (int i = 0; i < SSH_MAX_PUB_KEYS; i++) {
		cfg->ssh_pub_keys[i].username[0] = 0;
		cfg->ssh_pub_keys[i].type[0] = 0;
		cfg->ssh_pub_keys[i].name[0] = 0;
		cfg->ssh_pub_keys[i].pubkey_size = 0;
	}
	cfg->http_active = true;
	cfg->http_port = 0;
	cfg->https_port = 0;
#endif

}


#define NUM_TO_JSON(name, var) {					\
		cJSON_AddItemToObject(config, name,			\
				cJSON_CreateNumber(var));		\
	}


#define STRING_TO_JSON(name, var) {					\
		if (strlen(var) > 0)					\
			cJSON_AddItemToObject(config, name,		\
					cJSON_CreateString(var));	\
	}

#define PASSWD_TO_JSON(name, var) {					\
		if (strlen(var) > 0) {					\
			char *p = base64encode(var);			\
			if (p)						\
				cJSON_AddItemToObject(config, name,	\
						cJSON_CreateString(p));	\
			free(p);					\
		}							\
	}

#define IP_TO_JSON(name, var) {						\
		if (!ip_addr_isany(var))				\
			cJSON_AddItemToObject(config, name,		\
					cJSON_CreateString(ipaddr_ntoa(var))); \
	}

#define BITMASK_TO_JSON(name, var, max_count) {				\
		if (var)						\
			cJSON_AddItemToObject(config, name,		\
					cJSON_CreateString(		\
						bitmask_to_str(var, max_count, 1, true))); \
	}


cJSON *config_to_json(const struct system_config *cfg)
{
	cJSON *config = cJSON_CreateObject();
	cJSON *vsensors, *o;
	int i;

	if (!config)
		return NULL;

	cJSON_AddItemToObject(config, "id", cJSON_CreateString("lcdpico-config-v1"));
	cJSON_AddItemToObject(config, "debug", cJSON_CreateNumber(get_debug_level()));
	cJSON_AddItemToObject(config, "log_level", cJSON_CreateNumber(get_log_level()));
	cJSON_AddItemToObject(config, "syslog_level", cJSON_CreateNumber(get_syslog_level()));
	cJSON_AddItemToObject(config, "local_echo", cJSON_CreateBool(cfg->local_echo));
	cJSON_AddItemToObject(config, "led_mode", cJSON_CreateNumber(cfg->led_mode));
	cJSON_AddItemToObject(config, "bl_brightness", cJSON_CreateNumber(cfg->bl_brightness));
	cJSON_AddItemToObject(config, "i2c_speed", cJSON_CreateNumber(cfg->i2c_speed));
	cJSON_AddItemToObject(config, "adc_vref", cJSON_CreateNumber(cfg->adc_vref)); //Zitt
	STRING_TO_JSON("name", cfg->name);
	STRING_TO_JSON("timezone", cfg->timezone);
#ifdef WIFI_SUPPORT
	STRING_TO_JSON("hostname", cfg->hostname);
	STRING_TO_JSON("wifi_country", cfg->wifi_country);
	STRING_TO_JSON("wifi_ssid", cfg->wifi_ssid);
	PASSWD_TO_JSON("wifi_passwd", cfg->wifi_passwd);
	STRING_TO_JSON("wifi_auth_mode", cfg->wifi_auth_mode);
	if (cfg->wifi_mode != 0)
		NUM_TO_JSON("wifi_mode", cfg->wifi_mode);
	if (!ip_addr_isany(&cfg->dns_servers[0]))
		cJSON_AddItemToObject(config, "dns_servers",
				iplist2json(cfg->dns_servers, DNS_MAX_SERVERS));
	if (cfg->syslog_active != true)
		NUM_TO_JSON("ntp_active", cfg->syslog_active);
	IP_TO_JSON("syslog_server", &cfg->syslog_server);
	if (cfg->ntp_active != true)
		NUM_TO_JSON("ntp_active", cfg->ntp_active);
	if (!ip_addr_isany(&cfg->ntp_servers[0]))
		cJSON_AddItemToObject(config, "ntp_servers",
				iplist2json(cfg->ntp_servers, SNTP_MAX_SERVERS));
	IP_TO_JSON("ip", &cfg->ip);
	IP_TO_JSON("netmask", &cfg->netmask);
	IP_TO_JSON("gateway", &cfg->gateway);
	if (cfg->telnet_active)
		NUM_TO_JSON("telnet_active", cfg->telnet_active);
	if (cfg->telnet_auth != true)
		NUM_TO_JSON("telnet_auth", cfg->telnet_auth);
	if (cfg->telnet_raw_mode)
		NUM_TO_JSON("telnet_raw_mode", cfg->telnet_raw_mode);
	if (cfg->telnet_port > 0)
		NUM_TO_JSON("telnet_port", cfg->telnet_port);
	STRING_TO_JSON("telnet_user", cfg->telnet_user);
	STRING_TO_JSON("telnet_pwhash", cfg->telnet_pwhash);
	if ((o = acllist2json(cfg->telnet_acls, TELNET_MAX_ACL_ENTRIES)))
		cJSON_AddItemToObject(config, "telnet_acls", o);
	if (cfg->ssh_active)
		NUM_TO_JSON("ssh_active", cfg->ssh_active);
	if (cfg->ssh_auth != true)
		NUM_TO_JSON("ssh_auth", cfg->ssh_auth);
	if (cfg->ssh_port > 0)
		NUM_TO_JSON("ssh_port", cfg->ssh_port);
	STRING_TO_JSON("ssh_user", cfg->ssh_user);
	STRING_TO_JSON("ssh_pwhash", cfg->ssh_pwhash);
	if ((o = sshpubkeys2json(cfg->ssh_pub_keys)))
		cJSON_AddItemToObject(config, "ssh_pubkeys", o);
	if ((o = acllist2json(cfg->ssh_acls, SSH_MAX_ACL_ENTRIES)))
		cJSON_AddItemToObject(config, "ssh_acls", o);
	if (!cfg->http_active)
		NUM_TO_JSON("http_active", cfg->http_active);
	if (cfg->http_port > 0)
		NUM_TO_JSON("http_port", cfg->http_port);
	if (cfg->https_port > 0)
		NUM_TO_JSON("https_port", cfg->https_port);
#endif


	/* Virtual Sensors */
	vsensors = cJSON_CreateArray();
	if (!vsensors)
		goto panic;
	for (i = 0; i < VSENSOR_COUNT; i++) {
		const struct vsensor_input *s = &cfg->vsensors[i];

		o = cJSON_CreateObject();
		if (!o)
			goto panic;
		cJSON_AddItemToObject(o, "id", cJSON_CreateNumber(i));
		cJSON_AddItemToObject(o, "name", cJSON_CreateString(s->name));
		cJSON_AddItemToObject(o, "mode", cJSON_CreateString(vsmode2str(s->mode)));
		if (s->mode == VSMODE_MANUAL) {
			cJSON_AddItemToObject(o, "default_temp", cJSON_CreateNumber(s->default_temp));
			cJSON_AddItemToObject(o, "timeout", cJSON_CreateNumber(s->timeout));
		} else if (s->mode == VSMODE_I2C) {
			cJSON_AddItemToObject(o, "i2c_type",
					cJSON_CreateString(i2c_sensor_type_str(s->i2c_type)));
			cJSON_AddItemToObject(o, "i2c_addr",
					cJSON_CreateNumber(s->i2c_addr));
		}
		cJSON_AddItemToArray(vsensors, o);
	}
	cJSON_AddItemToObject(config, "vsensors", vsensors);

	return config;

panic:
	cJSON_Delete(config);
	return NULL;
}



#define JSON_TO_NUM(obj, name, var) {					\
		cJSON *ref;						\
		if ((ref = cJSON_GetObjectItem(obj, name))) {		\
			var = cJSON_GetNumberValue(ref);		\
		}							\
	}

#define JSON_TO_IP(obj, name, var) {					\
		cJSON *ref;						\
		if ((ref = cJSON_GetObjectItem(obj, name))) {		\
			if ((val = cJSON_GetStringValue(ref)))		\
				ipaddr_aton(val, var);			\
		}							\
	}

#define JSON_TO_BITMASK(obj, name, var, max_count) {			\
		cJSON *ref;						\
		if ((ref = cJSON_GetObjectItem(obj, name))) {		\
			uint32_t m;					\
			if (!str_to_bitmask(cJSON_GetStringValue(ref),	\
						max_count, &m, 1))	\
				var = m;				\
		}							\
	}

#define JSON_TO_STRING(obj, name, var) {				\
		cJSON *ref;						\
		if ((ref = cJSON_GetObjectItem(obj, name))) {		\
			if ((val = cJSON_GetStringValue(ref)))		\
				strncopy(var, val, sizeof(var));	\
		}							\
	}

#define JSON_TO_PASSWD(obj, name, var) {				\
		cJSON *ref;						\
		if ((ref = cJSON_GetObjectItem(obj, name))) {		\
			if ((val = cJSON_GetStringValue(ref))) {	\
				char *p = base64decode(val);		\
				if (p) {				\
					strncopy(var, p, sizeof(var));	\
					free(p);			\
				}					\
			}						\
		}							\
	}

int json_to_config(cJSON *config, struct system_config *cfg)
{
	cJSON *ref, *item, *r;
	int id;
	const char *val;


	if (!config || !cfg)
		return -1;


	/* Parse JSON configuration */

	if ((ref = cJSON_GetObjectItem(config, "id")))
		log_msg(LOG_INFO, "Config version: %s", ref->valuestring);
	if ((ref = cJSON_GetObjectItem(config, "debug")))
		set_debug_level(cJSON_GetNumberValue(ref));
	if ((ref = cJSON_GetObjectItem(config, "log_level")))
		set_log_level(cJSON_GetNumberValue(ref));
	if ((ref = cJSON_GetObjectItem(config, "syslog_level")))
		set_syslog_level(cJSON_GetNumberValue(ref));
	if ((ref = cJSON_GetObjectItem(config, "local_echo")))
		cfg->local_echo = (cJSON_IsTrue(ref) ? true : false);
	JSON_TO_NUM(config, "led_mode", cfg->led_mode);
	JSON_TO_NUM(config, "bl_brightness", cfg->bl_brightness);
	JSON_TO_NUM(config, "i2c_speed", cfg->i2c_speed);
	JSON_TO_NUM(config, "adc_vref", cfg->adc_vref);
	JSON_TO_STRING(config, "name", cfg->name);
	JSON_TO_STRING(config, "timezone", cfg->timezone);

#ifdef WIFI_SUPPORT
	JSON_TO_STRING(config, "hostname", cfg->hostname);
	JSON_TO_STRING(config, "wifi_country", cfg->wifi_country);
	JSON_TO_STRING(config, "wifi_ssid", cfg->wifi_ssid);
	JSON_TO_PASSWD(config, "wifi_passwd", cfg->wifi_passwd);
	JSON_TO_STRING(config, "wifi_auth_mode", cfg->wifi_auth_mode);
	JSON_TO_NUM(config, "wifi_mode", cfg->wifi_mode);
	if ((ref = cJSON_GetObjectItem(config, "dns_servers"))) {
		json2iplist(ref, cfg->dns_servers, DNS_MAX_SERVERS);
	}
	JSON_TO_NUM(config, "syslog_active", cfg->syslog_active);
	JSON_TO_IP(config, "syslog_server", &cfg->syslog_server);
	JSON_TO_NUM(config, "ntp_active", cfg->ntp_active);
	if ((ref = cJSON_GetObjectItem(config, "ntp_servers"))) {
		json2iplist(ref, cfg->ntp_servers, SNTP_MAX_SERVERS);
	} else {
		JSON_TO_IP(config, "ntp_server", &cfg->ntp_servers[0]);
	}
	JSON_TO_IP(config, "ip", &cfg->ip);
	JSON_TO_IP(config, "netmask", &cfg->netmask);
	JSON_TO_IP(config, "gateway", &cfg->gateway);

	JSON_TO_NUM(config, "telnet_active", cfg->telnet_active);
	JSON_TO_NUM(config, "telnet_auth", cfg->telnet_auth);
	JSON_TO_NUM(config, "telnet_raw_mode", cfg->telnet_raw_mode);
	JSON_TO_NUM(config, "telnet_port", cfg->telnet_port);
	JSON_TO_STRING(config, "telnet_user", cfg->telnet_user);
	JSON_TO_STRING(config, "telnet_pwhash", cfg->telnet_pwhash);
	if ((ref = cJSON_GetObjectItem(config, "telnet_acls")))
		json2acllist(ref, cfg->telnet_acls, TELNET_MAX_ACL_ENTRIES);

	JSON_TO_NUM(config, "ssh_active", cfg->ssh_active);
	JSON_TO_NUM(config, "ssh_auth", cfg->ssh_auth);
	JSON_TO_NUM(config, "ssh_port", cfg->ssh_port);
	JSON_TO_STRING(config, "ssh_user", cfg->ssh_user);
	JSON_TO_STRING(config, "ssh_pwhash", cfg->ssh_pwhash);
	if ((ref = cJSON_GetObjectItem(config, "ssh_pubkeys")))
		json2sshpubkeys(ref, cfg->ssh_pub_keys);
	if ((ref = cJSON_GetObjectItem(config, "ssh_acls")))
		json2acllist(ref, cfg->ssh_acls, SSH_MAX_ACL_ENTRIES);
	JSON_TO_NUM(config, "http_active", cfg->http_active);
	JSON_TO_NUM(config, "http_port", cfg->http_port);
	JSON_TO_NUM(config, "https_port", cfg->https_port);
	JSON_TO_BITMASK(config, "http_sensor_mask", cfg->http_sensor_mask, SENSOR_MAX_COUNT);
	JSON_TO_BITMASK(config, "http_vsensor_mask", cfg->http_vsensor_mask, VSENSOR_MAX_COUNT);
#endif


	/* Virtual Sensor configurations */
	ref = cJSON_GetObjectItem(config, "vsensors");
	cJSON_ArrayForEach(item, ref) {
		id = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "id"));
		if (id >= 0 && id < VSENSOR_COUNT) {
			struct vsensor_input *s = &cfg->vsensors[id];

			JSON_TO_STRING(item, "name", s->name);
			if (( r = cJSON_GetObjectItem(item, "mode")))
				s->mode = str2vsmode(cJSON_GetStringValue(r));
			if (s->mode == VSMODE_MANUAL) {
				JSON_TO_NUM(item, "default_temp", s->default_temp);
				JSON_TO_NUM(item, "timeout", s->timeout);
			} else if (s->mode == VSMODE_I2C) {
				if ((r = cJSON_GetObjectItem(item, "i2c_type")))
					s->i2c_type = get_i2c_sensor_type(cJSON_GetStringValue(r));
				JSON_TO_NUM(item, "i2c_addr", s->i2c_addr);
			}
		}
	}

	return 0;
}


void read_config(bool use_default_config)
{
	const char *default_config = lcdpico_default_config;
	uint32_t default_config_size = strlen(default_config);
	cJSON *config = NULL;
	int res;
	uint32_t file_size;
	char  *buf = NULL;


	if (!use_default_config) {
		log_msg(LOG_INFO, "Reading configuration...");

		res = flash_read_file(&buf, &file_size, "lcdpico.cfg");
		if (res == 0 && buf != NULL) {
			/* parse saved config... */
			config = cJSON_Parse(buf);
			if (!config) {
				const char *error_str = cJSON_GetErrorPtr();
				log_msg(LOG_ERR, "Failed to parse saved config: %s",
					(error_str ? error_str : "") );
			}
			free(buf);
		}
	}

	if (!config) {
		log_msg(LOG_NOTICE, "Using default configuration...");
		log_msg(LOG_DEBUG, "config size = %lu", default_config_size);
		/* printf("default config:\n---\n%s\n---\n", default_config); */
		config = cJSON_Parse(default_config);
		if (!config) {
			const char *error_str = cJSON_GetErrorPtr();
			panic("Failed to parse default config: %s\n",
				(error_str ? error_str : "") );
		}
	}


        /* Parse JSON configuration */
	mutex_enter_blocking(config_mutex);
	clear_config(&system_config);
	if (json_to_config(config, &system_config) < 0) {
		log_msg(LOG_ERR, "Error parsing JSON configuration");
	}
	if (use_default_config) {
		/* Enable more verbose logging if in "safe-mode" ... */
		set_log_level(LOG_INFO);
		system_config.local_echo = true;
	}
	mutex_exit(config_mutex);

	cJSON_Delete(config);
}


void save_config()
{
	cJSON *config;
	char *str;

	log_msg(LOG_NOTICE, "Saving configuration...");

	config = config_to_json(cfg);
	if (!config) {
		log_msg(LOG_ALERT, "Out of memory!");
		return;
	}

	if ((str = cJSON_Print(config)) == NULL) {
		log_msg(LOG_ERR, "Failed to generate JSON output");
	} else {
		uint32_t config_size = strlen(str) + 1;
		flash_write_file(str, config_size, "lcdpico.cfg");
		free(str);
	}

	cJSON_Delete(config);
}


void print_config()
{
	cJSON *config;
	char *str;

	config = config_to_json(cfg);
	if (!config) {
		log_msg(LOG_ALERT, "Out of memory");
		return;
	}

	if ((str = cJSON_Print(config)) == NULL) {
		log_msg(LOG_ERR, "Failed to generate JSON output");
	} else {
		printf("Current Configuration:\n%s\n\n", str);
		free(str);
	}

	cJSON_Delete(config);
}




#define CONFIG_READ_TIMEOUT 10000 // 10s
#define CONFIG_READ_BUF_SIZE 2048
#define BLANK_LINE_COUNT 2  // Number of blank lines to signify end of config...

void upload_config()
{
	absolute_time_t t_timeout = get_absolute_time();
	cJSON *config = NULL;
	cJSON *ref;
	char *buf = NULL;
	char tmp[256];
	uint32_t buf_len = CONFIG_READ_BUF_SIZE * 3;
	uint32_t buf_used = 0;
	int state = 0;
	int blank_count = 0;


	if (!(buf = malloc(buf_len))) {
		log_msg(LOG_ERR,"upload_config(): not enough memory (%lu)", buf_len);
		return;
	}

	tmp[0] = 0;
	printf("Paste LcdPico configuration in JSON format:\n");
	while (1) {
		char *line;
		uint32_t line_len;
		int r;

		if ((r = getstring_timeout_ms(tmp, sizeof(tmp), 100)) < 0)
			break;
		if (r > 0) {
			line = trim_str(tmp);
			line_len = strnlen(line, sizeof(tmp));

			blank_count = (line_len ? 0 : blank_count + 1);
			if (blank_count >= BLANK_LINE_COUNT)
				break;

			if (state == 0) {
				if (line_len)
					state = 1;
				else
					continue;
			}
			if (state == 1) {
				if (!line_len) {
					if (blank_count >= 1)
						break;
					continue;
				}
				if ((buf_len - buf_used) < line_len + 1) {
					char *new_buf;

					buf_len += CONFIG_READ_BUF_SIZE;
					if (!(new_buf = realloc(buf, buf_len))) {
						log_msg(LOG_ERR,"upload_config(): not enough memory (%lu)", buf_len);
						goto panic;
					}
					buf = new_buf;
				}
				memcpy(&buf[buf_used], line, line_len);
				buf_used += line_len;
				buf[buf_used++] = '\n';
				tmp[0] = 0;
			}
		}
		if (time_passed(&t_timeout, CONFIG_READ_TIMEOUT)) {
			printf("Timeout!\n");
			break;
		}
	}

	buf[buf_used] = 0;
	printf("[Received %lu bytes]\n\n", buf_used);

	if (buf_used < 1) {
		printf("No configuration received.\n");
		goto panic;
	}
	if (!(config = cJSON_Parse(buf))) {
		printf("Failed to parse uploaded config");
		goto panic;
	}
	if (!(ref = cJSON_GetObjectItem(config, "id"))) {
		printf("Uploaded JSON object missing 'id' field.\n");
		goto panic;
	}
	if (strncmp(ref->valuestring, "lcdpico-config-v", 16)) {
		printf("Invalid configuration uploaded.\n");
		goto panic;
	}

	printf("Clearing config...\n");
	clear_config(&system_config);
	printf("Loading config...\n");
	if (json_to_config(config, &system_config) < 0) {
		printf("Error parsing JSON configuration\n");
	} else {
		printf("Configuration successfully loaded.\n");
	}

panic:
	if (buf)
		free(buf);
	if (config)
		cJSON_Delete(config);
}

void delete_config()
{
	int res;

	res = flash_delete_file("lcdpico.cfg");
	if (res) {
		log_msg(LOG_ERR, "Failed to delete configuration.");
	}
}
