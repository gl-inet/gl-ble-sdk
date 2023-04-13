/*****************************************************************************
 * @file  demo_bleAdvExtended.c
 * @brief Enable BLE Extended broadcast sending function
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
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

static bool module_work = false;
static uint8_t handle = 0xff;



int main(int argc, char *argv[])
{
	signal(SIGTERM, sigal_hander);
	signal(SIGINT, sigal_hander);
	signal(SIGQUIT, sigal_hander);

	// init msg callback
	gl_ble_cbs ble_cb;
	memset(&ble_cb, 0, sizeof(gl_ble_cbs));

	ble_cb.ble_gap_event = NULL;
	ble_cb.ble_gatt_event = NULL;
	ble_cb.ble_module_event = ble_module_cb;

	// init adv param
    uint8_t primary_phy = 1;
    uint8_t secondary_phy = 1;
    uint32_t interval_min = 160;
    uint32_t interval_max = 320;
    uint8_t discover = 2;
    uint8_t connect = 4;
	//init adv data param
	const char *data = NULL;

	json_object *file_o = NULL;

	// get scanner param
	if ((argc != 1)  && (argc != 2))
	{
		goto exit;
	}
	
	if (argc == 2)
	{
		file_o = json_object_from_file(argv[1]);
		if(file_o == NULL)
		{
			goto exit;
		}

		json_object *primary_phy_o = json_object_object_get(file_o, "primary_phy");
		if(primary_phy_o == NULL)
		{
			goto exit;
		}
		primary_phy = json_object_get_int(primary_phy_o);

		json_object *secondary_phy_o = json_object_object_get(file_o, "secondary_phy");
		if(secondary_phy_o == NULL)
		{
			goto exit;
		}
		secondary_phy = json_object_get_int(secondary_phy_o);

		json_object *interval_min_o = json_object_object_get(file_o, "interval_min");
		if(interval_min_o == NULL)
		{
			goto exit;
		}
		interval_min = json_object_get_int(interval_min_o);

		json_object *interval_max_o = json_object_object_get(file_o, "interval_max");
		if(interval_max_o == NULL)
		{
			goto exit;
		}
		interval_max = json_object_get_int(interval_max_o);

		json_object *discover_o = json_object_object_get(file_o, "discover");
		if(discover_o == NULL)
		{
			goto exit;
		}
		discover = json_object_get_int(discover_o);

		json_object *connect_o = json_object_object_get(file_o, "connect");
		if(connect_o == NULL)
		{
			goto exit;
		}
		connect = json_object_get_int(connect_o);

		json_object *extended_adv_data_o = json_object_object_get(file_o, "extended_adv_data");
		if(extended_adv_data_o != NULL)
		{
			data = json_object_get_string(extended_adv_data_o);
		}
		
		json_object_put(file_o);
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

	// ble module check, will auto update firmware if module firmware not work. 
	// after update firmware if not work, will exit the program.
	ret = gl_ble_check_module(&ble_cb, false);
	if(ret != GL_SUCCESS)
	{
		printf("The ble module firmware not work.\n");
		exit(-1);
	}

	// wait for module reset
	while (!module_work)
	{
		usleep(100000);
	}

	// create adv handle
	ret = gl_ble_create_adv_handle(&handle);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_create_adv_handle failed: %d\n", ret);
		exit(-1);
	}

	// set advertising data
	if (data)
	{
		ret = gl_ble_set_extended_adv_data(handle, data);
		if (GL_SUCCESS != ret)
		{
			printf("gl_ble_set_extended_adv_data failed: %d\n", ret);
			exit(-1);
		}
	}

	// start advertising
	ret = gl_ble_start_extended_adv(handle, primary_phy, secondary_phy, interval_min, interval_max, discover, connect);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_start_extended_adv failed: %d\n", ret);
		exit(-1);
	}

	//get ble mac
	uint8_t mac_buf[6];
	ret = gl_ble_get_mac(mac_buf);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_get_mac failed: %d\n", ret);
		exit(-1);
	}
	printf("Broadcasting data, the mac of the current device is > " MACSTR "\n", MAC2STR(mac_buf));

	while (1)
	{
		sleep(1000);
	}

	return 0;

exit:
	if(file_o != NULL)
	{
		json_object_put(file_o);
	}
	printf("param err!");
	return GL_ERR_PARAM;
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
	printf("\bleAdvertiserExtended exit!\n");

	gl_ble_stop_adv(handle);
	gl_ble_delete_adv_handle(handle);
	gl_ble_unsubscribe();
	gl_ble_destroy();

	exit(0);
}
