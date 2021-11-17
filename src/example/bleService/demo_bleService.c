/*****************************************************************************
 * @file  demo_bleService.c
 * @brief Start the BLE discovery and subscribe the BLE event
 *******************************************************************************
 Copyright 2020 GL-iNet. https://www.gl-inet.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <ctype.h>
#include <json-c/json.h>

#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"

static void sigal_hander(int sig);
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static int addr2str(BLE_MAC adr, char *str);
static int gl_tools_hexStr2bytes(const char *hexstr, int strlen, unsigned char *bytes);

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

static bool module_work = false;

static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	switch (event)
	{
	case GAP_BLE_SCAN_RESULT_EVT:
	{
		addr2str(data->scan_rst.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("scan_result"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "address_type", json_object_new_int(data->scan_rst.ble_addr_type));
		json_object_object_add(o, "rssi", json_object_new_int(data->scan_rst.rssi));
		json_object_object_add(o, "packet_type", json_object_new_int(data->scan_rst.packet_type));
		json_object_object_add(o, "bonding", json_object_new_int(data->scan_rst.bonding));
		json_object_object_add(o, "data", json_object_new_string(data->scan_rst.ble_adv));
		const char *temp = json_object_to_json_string(o);
		printf("GAP_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}

	case GAP_BLE_UPDATE_CONN_EVT:
	{
		addr2str(data->update_conn_data.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("connect_update"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "interval", json_object_new_int(data->update_conn_data.interval));
		json_object_object_add(o, "latency", json_object_new_int(data->update_conn_data.latency));
		json_object_object_add(o, "timeout", json_object_new_int(data->update_conn_data.timeout));
		json_object_object_add(o, "security_mode", json_object_new_int(data->update_conn_data.security_mode));
		json_object_object_add(o, "txsize", json_object_new_int(data->update_conn_data.txsize));
		const char *temp = json_object_to_json_string(o);
		printf("GAP_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}

	case GAP_BLE_CONNECT_EVT:
	{
		addr2str(data->connect_open_data.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("connect_open"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "address_type", json_object_new_int(data->connect_open_data.ble_addr_type));
		json_object_object_add(o, "connect_role", json_object_new_int(data->connect_open_data.conn_role));
		json_object_object_add(o, "bonding", json_object_new_int(data->connect_open_data.bonding));
		json_object_object_add(o, "advertiser", json_object_new_int(data->connect_open_data.advertiser));
		const char *temp = json_object_to_json_string(o);
		printf("GAP_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}

	case GAP_BLE_DISCONNECT_EVT:
	{
		addr2str(data->disconnect_data.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("connect_close"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "reason", json_object_new_int(data->disconnect_data.reason));
		const char *temp = json_object_to_json_string(o);
		printf("GAP_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}
	default:
		break;
	}

	return 0;
}

static int ble_gatt_cb(gl_ble_gatt_event_t event, gl_ble_gatt_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	switch (event)
	{
	case GATT_REMOTE_CHARACTERISTIC_VALUE_EVT:
	{
		addr2str(data->remote_characteristic_value.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("remote_characteristic_value"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "characteristic", json_object_new_int(data->remote_characteristic_value.characteristic));
		json_object_object_add(o, "att_opcode", json_object_new_int(data->remote_characteristic_value.att_opcode));
		json_object_object_add(o, "offset", json_object_new_int(data->remote_characteristic_value.offset));
		json_object_object_add(o, "value", json_object_new_string(data->remote_characteristic_value.value));
		const char *temp = json_object_to_json_string(o);
		printf("GATT_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}
	case GATT_LOCAL_GATT_ATT_EVT:
	{
		addr2str(data->local_gatt_attribute.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("local_gatt_attribute"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "attribute", json_object_new_int(data->local_gatt_attribute.attribute));
		json_object_object_add(o, "att_opcode", json_object_new_int(data->local_gatt_attribute.att_opcode));
		json_object_object_add(o, "offset", json_object_new_int(data->local_gatt_attribute.offset));
		json_object_object_add(o, "value", json_object_new_string(data->local_gatt_attribute.value));
		const char *temp = json_object_to_json_string(o);
		// printf("GATT_CB_MSG >> %s\n", temp);

		json_object_put(o);

		uint8_t bytes[128] = {0};
		gl_tools_hexStr2bytes(data->local_gatt_attribute.value, strlen(data->local_gatt_attribute.value), bytes);
		printf("Data from the client : %.*s\n", strlen(bytes), bytes);

		GL_RET ret = gl_ble_write_char(data->local_gatt_attribute.address, data->local_gatt_attribute.attribute, "48656C6C6F20436C69656E74202E", 0);
		if (GL_SUCCESS != ret)
		{
			printf("gl_ble_write_char_%d failed: %d\n", data->local_gatt_attribute.attribute, ret);
		}
		break;
	}
	case GATT_LOCAL_CHARACTERISTIC_STATUS_EVT:
	{
		addr2str(data->local_characteristic_status.address, address);

		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("remote_set"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "characteristic", json_object_new_int(data->local_characteristic_status.characteristic));
		json_object_object_add(o, "status_flags", json_object_new_int(data->local_characteristic_status.status_flags));
		json_object_object_add(o, "client_config_flags", json_object_new_int(data->local_characteristic_status.client_config_flags));
		const char *temp = json_object_to_json_string(o);
		printf("GATT_CB_MSG >> %s\n", temp);

		json_object_put(o);
		break;
	}

	default:
		break;
	}

	return 0;
}

static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data)
{
	switch (event)
	{
	case MODULE_BLE_SYSTEM_BOOT_EVT:
	{
		module_work = true;
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("module_start"));
		json_object_object_add(o, "major", json_object_new_int(data->system_boot_data.major));
		json_object_object_add(o, "minor", json_object_new_int(data->system_boot_data.minor));
		json_object_object_add(o, "patch", json_object_new_int(data->system_boot_data.patch));
		json_object_object_add(o, "build", json_object_new_int(data->system_boot_data.build));
		json_object_object_add(o, "bootloader", json_object_new_int(data->system_boot_data.bootloader));
		json_object_object_add(o, "hw", json_object_new_int(data->system_boot_data.hw));
		json_object_object_add(o, "ble_hash", json_object_new_string(data->system_boot_data.ble_hash));
		const char *temp = json_object_to_json_string(o);
		printf("MODULE_CB_MSG >> %s\n", temp);

		json_object_put(o);
		if (data->system_boot_data.build != 151)
		{
			printf("The device does not support the use of this example, so stay tuned\n");
			exit(-1);
		}
		break;
	}
	default:
		break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	signal(SIGTERM, sigal_hander);
	signal(SIGINT, sigal_hander);
	signal(SIGQUIT, sigal_hander);

	// init msg callback
	gl_ble_cbs ble_cb;
	memset(&ble_cb, 0, sizeof(gl_ble_cbs));

	ble_cb.ble_gap_event = ble_gap_cb;
	ble_cb.ble_gatt_event = ble_gatt_cb;
	ble_cb.ble_module_event = ble_module_cb;

	int phys = 1, interval_min = 160, interval_max = 160, discover = 2, adv_conn = 2;

	// init ble module
	GL_RET ret;
	ret = gl_ble_init();
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_init failed: %d\n", ret);
		exit(-1);
	}

	ret = gl_ble_subscribe(&ble_cb);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_subscribe failed: %d\n", ret);
		exit(-1);
	}

	// wait for module reset
	while (!module_work)
	{
		usleep(100000);
	}

	ret = gl_ble_adv(phys, interval_min, interval_max, discover, adv_conn);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_adv failed: %d\n", ret);
		exit(-1);
	}

	uint8_t mac_buf[6];
	ret = gl_ble_get_mac(mac_buf);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_get_mac failed: %d\n", ret);
		exit(-1);
	}
	printf("Waiting to connect, the mac of the current device is > " MACSTR "\n", MAC2STR(mac_buf));

	while (1)
	{
		sleep(1000);
	}

	return 0;
}

static int addr2str(BLE_MAC adr, char *str)
{
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
			adr[3], adr[2], adr[1], adr[0]);
	return 0;
}

static void sigal_hander(int sig)
{
	printf("\nbleService exit!\n");

	gl_ble_stop_adv();
	gl_ble_unsubscribe();
	gl_ble_destroy();

	exit(0);
}

static int gl_tools_hexStr2bytes(const char *hexstr, int strlen, unsigned char *bytes)
{
	int i;
	int cnt = 0;

	for (i = 0; i < strlen; i++)
	{
		if (!isxdigit((int)hexstr[i]))
		{
			continue;
		}

		if ((hexstr[i] >= '0') && (hexstr[i] <= '9'))
			*bytes = (hexstr[i] - '0') << 4;
		else
			*bytes = (toupper((int)hexstr[i]) - 'A' + 10) << 4;

		i++;

		if ((hexstr[i] >= '0') && (hexstr[i] <= '9'))
			*bytes |= hexstr[i] - '0';
		else
			*bytes |= toupper((int)hexstr[i]) - 'A' + 10;

		bytes++;
		cnt++;
	}
	return cnt;
}
