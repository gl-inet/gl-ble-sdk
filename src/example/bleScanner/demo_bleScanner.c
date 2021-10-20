/*****************************************************************************
 * @file  test.c
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
#include <json-c/json.h>

#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"

static void sigal_hander(int sig);
static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data);
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static int addr2str(BLE_MAC adr, char* str);

static bool module_work = false;

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

	int phys = 1, interval = 16, window = 16, type = 0, mode = 1;
	// get scanner param
	if((argc != 1) && (argc != 6))
	{
		printf("param err!");
		return GL_ERR_PARAM;
	}
	if(argc == 6)
	{
		phys = atoi(argv[1]);
		interval = atoi(argv[2]);
		window = atoi(argv[3]);
		type = atoi(argv[4]);
		mode = atoi(argv[5]);
	}

	// init ble module
	GL_RET ret;
	ret = gl_ble_init();
	if(GL_SUCCESS != ret)
	{
		printf("gl_ble_init failed: %d\n", ret);
		exit(-1);
	}

	ret = gl_ble_subscribe(&ble_cb);
	if(GL_SUCCESS != ret)
	{
		printf("gl_ble_subscribe failed: %d\n", ret);
		exit(-1);
	}

	// wait for module reset
	while(!module_work)
	{
		usleep(500000);
	}

	// start scan
	ret = gl_ble_discovery(phys, interval, window, type, mode);
	if(ret != GL_SUCCESS)
	{
		printf("Start ble discovery error!! Err code: %d\n", ret);
		exit(-1);
	}
	

	while(1)
	{
		sleep(1000);
	}

	return 0;
}


static int addr2str(BLE_MAC adr, char* str) 
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
            adr[3], adr[2], adr[1], adr[0]);
    return 0;
}


static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	switch (event)
	{
		case GAP_BLE_SCAN_RESULT_EVT:
		{
			addr2str(data->scan_rst.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("scan_result"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "address_type", json_object_new_int(data->scan_rst.ble_addr_type));
			json_object_object_add(o, "rssi", json_object_new_int(data->scan_rst.rssi));
			json_object_object_add(o, "packet_type", json_object_new_int(data->scan_rst.packet_type));
			json_object_object_add(o, "bonding", json_object_new_int(data->scan_rst.bonding));
			json_object_object_add(o, "data", json_object_new_string(data->scan_rst.ble_adv));
			const char *temp = json_object_to_json_string(o);
			printf("%s\n",temp);

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
