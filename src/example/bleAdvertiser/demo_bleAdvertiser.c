/*****************************************************************************
 * @file  bleAdvertiser.c
 * @brief Enable BLE broadcast sending function
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
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static int addr2str(BLE_MAC adr, char *str);

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

static bool module_work = false;

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

	int phys = 1, interval_min = 160, interval_max = 160, discover = 2, adv_conn = 2, flag = 0;
	char *data = NULL;
	// get scanner param
	if ((argc != 1) && (argc != 8))
	{
		printf("param err!");
		return GL_ERR_PARAM;
	}
	if (argc == 8)
	{
		phys = atoi(argv[1]);
		interval_min = atoi(argv[2]);
		interval_max = atoi(argv[3]);
		discover = atoi(argv[4]);
		adv_conn = atoi(argv[5]);
		flag = atoi(argv[6]);
		data = argv[7];
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
		usleep(500000);
	}

	// set advertising data
	if (data != NULL)
	{
		ret = gl_ble_adv_data(flag, data);
		if (GL_SUCCESS != ret)
		{
			printf("gl_ble_adv_data failed: %d\n", ret);
			exit(-1);
		}
	}

	// start advertising
	ret = gl_ble_adv(phys, interval_min, interval_max, discover, adv_conn);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_adv failed: %d\n", ret);
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
}

static int addr2str(BLE_MAC adr, char *str)
{
	sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
			adr[3], adr[2], adr[1], adr[0]);
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
	printf("\nbleAdvertising exit!\n");

	gl_ble_stop_adv();
	gl_ble_unsubscribe();
	gl_ble_destroy();

	exit(0);
}
