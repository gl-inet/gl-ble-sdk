/*****************************************************************************
 * @file  demo_bleIbeacon.c
 * @brief Enable BLE broadcast or discovery ibeacon packet
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
#include <pthread.h>

#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>

#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"
#include "ibeacon.h"
#include "ubus.h"
#include "run_mode.h"

bool ibeacon_module_work = false;

ibeacon_ringArray_t *ibeacon_array;
uint8_t adv_handle = 0xff;

static pthread_t tid_ble;
static void *ble_start(void *arg);
static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data);
static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static void sigal_ibeacon_hander(int sig);
static int mode = -1;

int main(int argc, char *argv[])
{
	int ret = -1;
	
	ret = pthread_create(&tid_ble, NULL, ble_start, NULL);
    if (ret) {
        fprintf(stderr, "ubus_pthread_create: %s\n", strerror(ret));
        return -1;
    }

	mode = mode_check(argc);

	signal(SIGTERM, sigal_ibeacon_hander);
	signal(SIGINT, sigal_ibeacon_hander);
	signal(SIGQUIT, sigal_ibeacon_hander);
	if(mode == BACKGROUNG_MODE)
	{
		ret = background();
		if(ret != GL_SUCCESS)
		{
			printf("background mode open failed\n");
			exit(-1);
		}
	}
	else if(mode == FOREGROUND_MODE)
	{
		ret = foreground_param_check(argc, argv);
		if(ret != GL_SUCCESS)
		{
			exit(-1);
		}
		ret = foreground(argv);
		if(ret != GL_SUCCESS)
		{
			printf("foreground mode open failed\n");
			exit(-1);
		}
	}

}

static void *ble_start(void *arg)
{
	GL_RET ret = -1;
	ibeacon_array = calloc(1, sizeof(ibeacon_ringArray_t));

	// init msg callback
	gl_ble_cbs ble_ibeacon_cb;
	memset(&ble_ibeacon_cb, 0, sizeof(gl_ble_cbs));

	ble_ibeacon_cb.ble_gap_event = ble_gap_cb;
	ble_ibeacon_cb.ble_gatt_event = NULL;
	ble_ibeacon_cb.ble_module_event = ble_module_cb;

	// init ble module
	ret = gl_ble_init();
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_init failed: %d\n", ret);
		exit(-1);
	}

	ret = gl_ble_subscribe(&ble_ibeacon_cb);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_subscribe failed: %d\n", ret);
		exit(-1);
	}

	// wait for module reset
	while (!ibeacon_module_work)
	{
		usleep(100000);
	}

// check ble module version, if not match will dfu
ble_module_check:
	ret = gl_ble_check_module_version();
	if(GL_SUCCESS != ret)
	{
		ibeacon_module_work = false;
		// Deinit first, and the serial port is occupied anyway
		gl_ble_unsubscribe();
		gl_ble_destroy();

		ret = gl_ble_module_dfu();
		if(GL_SUCCESS == ret)
		{
			// Reinit if dfu success
			gl_ble_init();
			gl_ble_subscribe(&ble_ibeacon_cb);

			// wait for module reset
			while (!ibeacon_module_work)
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

	pthread_join(tid_ble, NULL);
	return NULL;
}

static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data)
{
	
	switch (event)
	{
	case GAP_BLE_LEGACY_SCAN_RESULT_EVT:
	{
		ibeacon_data_collect(data);
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

		ibeacon_module_work = true;

		break;
	}
	default:
		break;
	}

	return 0;
}

static void sigal_ibeacon_hander(int sig)
{
	printf("\nbleIbeacon exit!\n");
	if(mode == FOREGROUND_MODE)
	{
		goto exit_foreground;
	}
	else if(mode == BACKGROUNG_MODE)
	{
		goto exit_background;
	}
	else
	{
		exit(0);
	}

exit_background:

	pthread_cancel(ubus_tid);
	pthread_join(ubus_tid, NULL);
	if (_ctx_event)
		ubus_free(_ctx_event);

	if (_ctx_invoke)
		ubus_free(_ctx_invoke);
		
	uloop_done();

exit_foreground:	

	gl_ble_stop_adv(adv_handle);
	gl_ble_stop_discovery();
	gl_ble_destroy();
	pthread_cancel(tid_ble);
	
	free(ibeacon_array);
	exit(0);
}