/*****************************************************************************
 * @file  demo_bleSynchronize.c
 * @brief Start the BLE Synchronize and subscribe the BLE event
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
static int hex2str(uint8_t* head, int len, char* value);
static int str2addr(char* str, BLE_MAC address) ;

static bool module_work = false;
static bool start_sync = false;
static bool is_specified = false;

static uint16_t skip = 0;
static uint16_t timeout = 100;
static BLE_MAC address_u8;
static uint8_t address_type;
static uint8_t adv_sid;
static uint16_t handle = 0xffff;

// store the ble module version when ble module init success, For subsequent version check
static int major = 0;
static int minor = 0;
static int patch = 0;

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

	// get synchronize param

	if((argc != 1) && (argc != 6))
	{
		printf("param err!");
		return GL_ERR_PARAM;
	}
	
	
	if(argc == 6)
	{
		is_specified = true;
		char *addr = NULL;
		skip = atoi(argv[1]);
		timeout = atoi(argv[2]);
		addr = argv[3];
		str2addr(addr, address_u8);
		address_type = atoi(argv[4]);
		adv_sid = atoi(argv[5]);
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
	ret = gl_ble_check_module_version(major, minor, patch);
	if(ret != GL_SUCCESS)
	{
		module_work = false;
		// Deinit first, and the serial port is occupied anyway
		gl_ble_unsubscribe();
		gl_ble_destroy();

		ret = gl_ble_module_dfu();
		if(ret == GL_SUCCESS)
		{
			// Reinit if dfu success
			gl_ble_init();
			gl_ble_subscribe(&ble_cb);

			// wait for module reset
			while (!module_work)
			{
				usleep(100000);
			}
		}
		else
		{
			printf("The ble module firmware version is not 4_2_0, please switch it.\n");
			exit(-1);
		}
	}

	// start scan before sync
	ret = gl_ble_start_discovery(5, 16, 16, 0, 2);
	if (ret != GL_SUCCESS)
	{
		printf("Start ble scanner error!! Err code: %d\n", ret);
		exit(-1);
	}
	
	// Start synchronizing the specified address
	if(argc == 6)
	{
		ret = gl_ble_start_synchronize(skip, timeout, address_u8, address_type, adv_sid, &handle);
		if (ret != GL_SUCCESS)
		{
			printf("Start ble synchronize error!! Err code: %d\n", ret);
			exit(-1);
		}
		gl_ble_stop_discovery();
	}
	start_sync = true;

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
		case GAP_BLE_EXTENDED_SCAN_RESULT_EVT:
		{	
			if(is_specified)
			{
				break;
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

				// if no synchronization address is specified, the first periodic broadcast packet scanned synchronously
				if(!is_specified && start_sync)
				{
					is_specified = true;
					memcpy(address_u8, data->extended_scan_rst.address, DEVICE_MAC_LEN);
					address_type = data->extended_scan_rst.ble_addr_type;
					adv_sid = data->extended_scan_rst.adv_sid;
					
					GL_RET ret = gl_ble_start_synchronize(skip, timeout, address_u8, address_type, adv_sid, &handle);
					if (ret != GL_SUCCESS)
					{
						printf("Start ble synchronize error!! Err code: %d\n", ret);
						exit(-1);
					}

					gl_ble_stop_discovery();
				}
				break;
			}
			break;
		}

		case  GAP_BLE_SYNC_SCAN_RESULT_EVT:
		{
			hex2str(data->sync_scan_rst.ble_adv, data->sync_scan_rst.ble_adv_len, ble_adv);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("sync_result"));
			json_object_object_add(o, "rssi", json_object_new_int(data->sync_scan_rst.rssi));
			json_object_object_add(o, "data", json_object_new_string(ble_adv));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);

			json_object_put(o);
			break;
		}

		// case GAP_BLE_SYNC_CLOSED_EVT:
		// {
		// 	printf("sync closed\n");
		// 	// If the synchronous closed is not ordered, it will to restart synchronize
		// 	int status = -1;
		// 	if (!is_stop_sync)
		// 	{
		// 		status = gl_ble_start_synchronize(skip, timeout, address_u8, address_type, adv_sid, &handle);
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

		// For subsequent version check
		major = data->system_boot_data.major;
		minor = data->system_boot_data.minor;
		patch = data->system_boot_data.patch;

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

	start_sync = false;
	is_specified = false;
	gl_ble_stop_synchronize(handle);

	gl_ble_unsubscribe();
	gl_ble_destroy();

	exit(0);
}
