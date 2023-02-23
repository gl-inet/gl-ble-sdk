/*****************************************************************************
 * @file  demo_bleScanner.c
 * @brief Start the BLE discovery and subscribe the BLE event
 *******************************************************************************
 Copyright 2022 GL-iNet. https://www.gl-inet.com/

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
#include <json-c/json.h>

#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"

static void sigal_hander(int sig);
static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data);
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static int addr2str(BLE_MAC adr, char *str);
static int str2addr(char* str, BLE_MAC address); 
static int hex2str(uint8_t* head, int len, char* value);

static bool module_work = false;
static BLE_MAC mac_filter = {0};
static bool mac_filter_flag = false;



int main(int argc, char *argv[])
{
	signal(SIGTERM, sigal_hander);
	signal(SIGINT, sigal_hander);
	signal(SIGQUIT, sigal_hander);

	// init msg callback
	gl_ble_cbs ble_cb;
	memset(&ble_cb, 0, sizeof(gl_ble_cbs));

	ble_cb.ble_gap_event = ble_gap_cb;
	ble_cb.ble_gatt_event = NULL;
	ble_cb.ble_module_event = ble_module_cb;

	uint8_t phys = 1;
	uint16_t interval = 16, window = 16;
	uint8_t type = 0, mode = 2;
	char *address = NULL;
	// get scanner param
	if ((argc != 1) && (argc != 2) && (argc != 6) && (argc != 7))
	{
		printf("param err!");
		return GL_ERR_PARAM;
	}

	if (argc == 2)
	{
		address = argv[1];
		if (strlen(address) != (BLE_MAC_LEN - 1))
		{
			printf("param err!");
			return GL_ERR_PARAM;
		}

		str2addr(address, mac_filter);
		mac_filter_flag = true;
	}

	if (argc >= 6)
	{
		phys = atoi(argv[1]);
		interval = atoi(argv[2]);
		window = atoi(argv[3]);
		type = atoi(argv[4]);
		mode = atoi(argv[5]);
	}
	
	if (argc == 7)
	{
		address = argv[6];
		if (strlen(address) != (BLE_MAC_LEN - 1))
		{
			printf("param err!");
			return GL_ERR_PARAM;
		}

		str2addr(address, mac_filter);
		mac_filter_flag = true;
	}

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

// check ble module version, if not match will dfu
ble_module_check:
	ret = gl_ble_check_module_version();
	if(GL_SUCCESS != ret)
	{
		module_work = false;
		// Deinit first, and the serial port is occupied anyway
		gl_ble_unsubscribe();
		gl_ble_destroy();

		ret = gl_ble_module_dfu();
		if(GL_SUCCESS == ret)
		{
			// Reinit if dfu success
			gl_ble_init();
			gl_ble_subscribe(&ble_cb);

			// wait for module reset
			while (!module_work)
			{
				usleep(100000);
			}

			goto ble_module_check;
		}
		else
		{
			printf("The ble module firmware version is not 4_2_0, please switch it.\n");
			exit(-1);
		}
	}

	// start scan
	ret = gl_ble_start_discovery(phys, interval, window, type, mode);
	if (ret != GL_SUCCESS)
	{
		printf("Start ble discovery error!! Err code: %d\n", ret);
		exit(-1);
	}

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

static int str2addr(char* str, BLE_MAC address) 
{
	int mac[6] = {0};
    sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[5], &mac[4], &mac[3],
           &mac[2], &mac[1], &mac[0]);

	uint8_t i = 0;
	for(;i < 6; i++)
	{
		address[i] = mac[i];
	}
    return 0;
}

static int hex2str(uint8_t* head, int len, char* value)
{
    int i = 0;

    // FIXME: (Sometime kernel don't mask all uart print) When wifi network up/down, it will recv a big message
    // if(len >= 256/2)    
    // {    
    //     strcpy(value,"00");
    //     // printf("recv a err msg! err len = %d\n",len);
    //     return -1;
    // }
    
    while (i < len) {
        sprintf(value + i * 2, "%02x", head[i]);
        i++;
    }
    return 0;
}

static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	char ble_adv[MAX_ADV_DATA_LEN * 2 + 1] = {0};
	switch (event)
	{
		case GAP_BLE_LEGACY_SCAN_RESULT_EVT:
		{
			if(mac_filter_flag)
			{
				if(0 != memcmp(data->legacy_scan_rst.address, mac_filter, 6))
				{
					break;
				}
			}
			addr2str(data->legacy_scan_rst.address, address);
			hex2str(data->legacy_scan_rst.ble_adv, data->legacy_scan_rst.ble_adv_len, ble_adv);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();

			json_object_object_add(o, "type", json_object_new_string("legacy_adv_result"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "address_type", json_object_new_int(data->legacy_scan_rst.ble_addr_type));
			json_object_object_add(o, "rssi", json_object_new_int(data->legacy_scan_rst.rssi));
			json_object_object_add(o, "event_flags", json_object_new_int(data->legacy_scan_rst.event_flags));
			json_object_object_add(o, "bonding", json_object_new_int(data->legacy_scan_rst.bonding));
			json_object_object_add(o, "data", json_object_new_string(ble_adv));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);

			json_object_put(o);
			break;
		}

		case GAP_BLE_EXTENDED_SCAN_RESULT_EVT:
		{	
			if(mac_filter_flag)
			{
				if(0 != memcmp(data->legacy_scan_rst.address, mac_filter, 6))
				{
					break;
				}
			}
			addr2str(data->extended_scan_rst.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();

			//periodic adv exist
			if (data->extended_scan_rst.periodic_interval)
			{
				json_object_object_add(o, "type", json_object_new_string("periodic_adv_result"));
				json_object_object_add(o, "mac", json_object_new_string(address));
				json_object_object_add(o, "address_type", json_object_new_int(data->extended_scan_rst.ble_addr_type));
				json_object_object_add(o, "rssi", json_object_new_int(data->extended_scan_rst.rssi));
				json_object_object_add(o, "adv_sid", json_object_new_int(data->extended_scan_rst.adv_sid));
				json_object_object_add(o, "periodic_interval", json_object_new_int(data->extended_scan_rst.periodic_interval));
				const char *temp = json_object_to_json_string(o);
				printf("GAP_CB_MSG >> %s\n",temp);

				json_object_put(o);
				break;
			}

			hex2str(data->extended_scan_rst.ble_adv, data->extended_scan_rst.ble_adv_len, ble_adv);

			json_object_object_add(o, "type", json_object_new_string("extended_adv_result"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "address_type", json_object_new_int(data->extended_scan_rst.ble_addr_type));
			json_object_object_add(o, "rssi", json_object_new_int(data->extended_scan_rst.rssi));
			json_object_object_add(o, "event_flags", json_object_new_int(data->extended_scan_rst.event_flags));
			json_object_object_add(o, "bonding", json_object_new_int(data->extended_scan_rst.bonding));
			json_object_object_add(o, "data", json_object_new_string(ble_adv));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);

			json_object_put(o);
			break;
		}

		// case  GAP_BLE_SYNC_SCAN_RESULT_EVT:
		// {
		// 	char ble_adv[data->sync_scan_rst.ble_adv_len * 2];
		// 	memset(ble_adv, 0, data->sync_scan_rst.ble_adv_len * 2);
		// 	hex2str(data->sync_scan_rst.ble_adv, data->sync_scan_rst.ble_adv_len, ble_adv);

		// 	// json format
		// 	json_object* o = NULL;
		// 	o = json_object_new_object();
		// 	json_object_object_add(o, "type", json_object_new_string("sync_result"));
		// 	json_object_object_add(o, "rssi", json_object_new_int(data->sync_scan_rst.rssi));
		// 	json_object_object_add(o, "data", json_object_new_string(sync_data));
		// 	const char *temp = json_object_to_json_string(o);
		// 	printf("GAP_CB_MSG >> %s\n",temp);

		// 	json_object_put(o);
		// 	break;
		// }

		// case GAP_BLE_SYNC_CLOSED_EVT:
		// {
		// 	printf("sync closed\n");
		// 	// If the synchronous closed is not ordered, it will to restart synchronize
		// 	int status = -1;
		// 	if (!is_stop_sync)
		// 	{
		// 		status = gl_ble_start_synchronize(&synchronize_param);
		// 		printf("restart sync status: %d\n", status);
		// 	}
				
		// 	break;
		// }
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

		module_work = true;

		break;
	}
	default:
		break;
	}

	return 0;
}

static void sigal_hander(int sig)
{
	printf("\nbleScanner exit!\n");

	gl_ble_stop_discovery();
	gl_ble_unsubscribe();
	gl_ble_destroy();

	exit(0);
}
