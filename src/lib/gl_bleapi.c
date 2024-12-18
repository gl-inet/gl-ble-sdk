/*****************************************************************************
 * @file  libglbleapi.c
 * @brief Shared library for API interface
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
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "gl_bleapi.h"
#include "gl_dev_mgr.h"
#include "gl_log.h"
#include "gl_common.h"
#include "gl_hal.h"
#include "gl_methods.h"
#include "gl_thread.h"
#include "silabs_msg.h"
#include "silabs_evt.h"

extern hw_cfg_t* ble_hw_cfg;
extern bool ble_version_match;
extern bool module_boot;

gl_ble_cbs ble_msg_cb;

/************************************************************************************************************************************/

void *ble_driver_thread_ctx = NULL;
void *ble_watcher_thread_ctx = NULL;

static int* msqid = NULL;

static driver_param_t* _driver_param = NULL;
static watcher_param_t* _watcher_param = NULL;
static pthread_mutex_t _ble_mutex;

/************************************************************************************************************************************/
GL_RET gl_ble_init(void)
{
	// err return if ble driver thread exist
	if((NULL != _driver_param) || (NULL != ble_driver_thread_ctx))
	{
		return GL_ERR_INVOKE;
	}

	// init work thread param
	_driver_param = (driver_param_t*)malloc(sizeof(driver_param_t));
	
	// create an event message queue if it not exist
	if(NULL == msqid)
	{
		msqid = (int*)malloc(sizeof(int));
		*msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
		if(*msqid == -1)
		{
			log_err("create msg queue error!!!\n");
			return GL_UNKNOW_ERR;
		}
	}
	_driver_param->evt_msgid = *msqid;

	/* Init device manage */
	ble_dev_mgr_init();

	int ret;
	// init hal
	ret = hal_init();
	if (ret == GL_UNKNOW_ERR)
	{
		log_err("hal init failed!\n");
		// free driver_param_t & driver ctx
		free(_driver_param);
		_driver_param = NULL;
		ble_driver_thread_ctx = NULL;
		
		// destroy device list
		ble_dev_mgr_destroy();
        return GL_UNKNOW_ERR;
	}

	// create a thread to recv module message
    ret = HAL_ThreadCreate(&ble_driver_thread_ctx, ble_driver, _driver_param, NULL, NULL);
    if (ret != 0) {
        log_err("pthread_create ble_driver_thread_ctx failed!\n");
		// free driver_param_t & driver ctx
		free(_driver_param);
		_driver_param = NULL;
		ble_driver_thread_ctx = NULL;

		// close hal fd
		hal_destroy();
		
		// destroy device list
		ble_dev_mgr_destroy();
        return GL_UNKNOW_ERR;
    }

	// create a thread mutex to protect thread data chaos
	pthread_mutex_init(&_ble_mutex, NULL);

	// reset ble module to make sure it is a usable mode
	gl_ble_hard_reset();

	return GL_SUCCESS;
}

GL_RET  gl_ble_destroy(void)
{
	// close msg thread
	HAL_ThreadDelete(ble_driver_thread_ctx);
	ble_driver_thread_ctx = NULL;

	// free driver_param_t
	if(_driver_param)
	{
		free(_driver_param);
		_driver_param = NULL;
	}
	
	// close hal fd
	hal_destroy();

	// close thread mutex
	pthread_mutex_destroy(&_ble_mutex);

	// destroy device list
	ble_dev_mgr_destroy();

	// destroy evt msg queue
	if(-1 == msgctl(*msqid, IPC_RMID, NULL))
	{
		log_err("msgctl error");
		return GL_UNKNOW_ERR;
	}

	if(msqid)
	{
		free(msqid);
		msqid = NULL;
	}
		
	return GL_SUCCESS;
}

GL_RET gl_ble_subscribe(gl_ble_cbs *callback)
{
	if(NULL == callback)
	{
		return GL_ERR_PARAM;
	}

	// error return if watcher thread exist
	if((NULL != _watcher_param) || (NULL != ble_watcher_thread_ctx))
	{
		return GL_ERR_INVOKE;
	}

	_watcher_param = (watcher_param_t*)malloc(sizeof(watcher_param_t));

	// create an event message queue if it not exist
	if(NULL == msqid)
	{
		msqid = (int*)malloc(sizeof(int));
		*msqid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
		if(*msqid == -1)
		{
			log_err("create msg queue error!!!\n");
			return GL_UNKNOW_ERR;
		}
	}
	_watcher_param->evt_msgid = *msqid;
	_watcher_param->cbs = callback;

    int ret;
    ret = HAL_ThreadCreate(&ble_watcher_thread_ctx, ble_watcher, _watcher_param, NULL, NULL);
    if (ret != 0) {
        log_err("pthread_create failed!\n");
		// free watcher_param_t
		free(_watcher_param);
		_watcher_param = NULL;
        return GL_UNKNOW_ERR;
    }

	return GL_SUCCESS;

}

GL_RET gl_ble_unsubscribe(void)
{
	HAL_ThreadDelete(ble_watcher_thread_ctx);
	ble_watcher_thread_ctx = NULL;

	// free watcher_param_t
	free(_watcher_param);
	_watcher_param = NULL;

	return GL_SUCCESS;
}

GL_RET gl_ble_enable(int32_t enable)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_enable(enable);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_hard_reset(void)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_hard_reset();
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_get_mac(BLE_MAC mac)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_local_mac(mac);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_power(int power, int *current_power)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_power(power, current_power);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_create_adv_handle(uint8_t *handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_create_adv_handle(handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_delete_adv_handle(uint8_t handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_delete_adv_handle(handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_start_legacy_adv(uint8_t handle, uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_start_legacy_adv(handle, interval_min, interval_max, discover, connect);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_start_extended_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, 
                        uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_start_extended_adv(handle, primary_phy, secondary_phy, interval_min, interval_max,
							 discover, connect);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_start_periodic_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, uint16_t interval_min, uint16_t interval_max)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_start_periodic_adv(handle, primary_phy, secondary_phy, interval_min, interval_max);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_stop_adv(uint8_t handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_stop_adv(handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_legacy_adv_data(uint8_t handle, uint8_t flag, const char *data)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_legacy_adv_data(handle, flag, data);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_extended_adv_data(uint8_t handle, const char *data)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_extended_adv_data(handle, data);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_periodic_adv_data(uint8_t handle, const char *data)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_periodic_adv_data(handle, data);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_send_notify(BLE_MAC address, int char_handle, char *value)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_send_notify(address, char_handle, value);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_start_discovery(uint8_t phys, uint16_t interval, uint16_t window, uint8_t type, uint8_t mode)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_start_discovery(phys, interval, window, type, mode);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_stop_discovery(void)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_stop_discovery();
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_sync_parameters(uint16_t skip, uint16_t timeout)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_sync_parameters(skip, timeout);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_start_sync(BLE_MAC address, uint8_t address_type, uint8_t adv_sid, uint16_t *handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_start_sync(address, address_type, adv_sid, handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_stop_sync(uint16_t handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_stop_sync(handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_connect(BLE_MAC address, int address_type, int phy)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_connect(address, address_type, phy);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_disconnect(BLE_MAC address)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_disconnect(address);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_get_rssi(BLE_MAC address, int32_t *rssi)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_get_rssi(address, rssi);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_get_service(gl_ble_service_list_t *service_list, BLE_MAC address)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_get_service(service_list, address);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_get_char(gl_ble_char_list_t *char_list, BLE_MAC address, int service_handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_get_char(char_list, address, service_handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_read_char(BLE_MAC address, int char_handle)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_read_char(address, char_handle);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_write_char(BLE_MAC address, int char_handle, char *value, int res)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_write_char(address, char_handle, value, res);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_set_notify(BLE_MAC address, int char_handle, int flag)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_notify(address, char_handle, flag);
	pthread_mutex_unlock(&_ble_mutex);
	return ret; 
}

GL_RET gl_ble_set_gattdb(char *json_cfg_name)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_gattdb(json_cfg_name);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_module_dfu(void)
{
	pthread_mutex_lock(&_ble_mutex);
	
    char command[128] = {0};

	// The dfu io recognized by the device is 0xff, which means that the device cannot be upgraded in this way
	if(ble_hw_cfg->dfu_gpio == 0xff) 
	{
		pthread_mutex_unlock(&_ble_mutex);
		return GL_UNKNOW_ERR;
	}

	if(access("/sys/class/gpio/ble_rst", F_OK) == 0 && 
		access("/sys/class/gpio/dfu_rst", F_OK) == 0)
	{
		sprintf(command, "/usr/bin/gl-ble-dfu %s %s ble_rst dfu_rst", ble_hw_cfg->model, ble_hw_cfg->port);
	}
	else
	{
		sprintf(command, "/usr/bin/gl-ble-dfu %s %s %d %d", ble_hw_cfg->model, ble_hw_cfg->port, ble_hw_cfg->rst_gpio, ble_hw_cfg->dfu_gpio);
	}

    if( system(command) != 0 )
	{
		perror("system");
		pthread_mutex_unlock(&_ble_mutex);
		return GL_UNKNOW_ERR;
    }

	pthread_mutex_unlock(&_ble_mutex);
    return GL_SUCCESS;
}

GL_RET gl_ble_check_module(gl_ble_cbs *callback, bool force_dfu)
{
	GL_RET ret;
	uint8_t count = 0;

	while (!module_boot)
	{
		usleep(100000);
		if(++count == 30) //timeout 3 sec
		{
			goto start_dfu;
		}
	}

	if(force_dfu)
	{
		goto start_dfu;
	}
	
	if(ble_version_match)
	{
		module_boot = false;
		return GL_SUCCESS;
	}
	else
	{
		goto start_dfu;
	}

start_dfu:	
	module_boot = false;
	// Deinit first, and the serial port is occupied anyway
	gl_ble_unsubscribe();
	gl_ble_destroy();

	//force to DFU
	ret = gl_ble_module_dfu();
	printf("dfu ret: %d\n", ret);
	if(GL_SUCCESS == ret)
	{
		gl_ble_init();
		gl_ble_subscribe(callback);

		return GL_SUCCESS;
	}
	else 
	{
		return GL_UNKNOW_ERR;
	}
}

// GL_RET gl_ble_del_gattdb(void)
// {
// 	return ble_del_gattdb();
// }

GL_RET gl_ble_set_identity_address(BLE_MAC address, int address_type)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_set_identity_address(address, address_type);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}

GL_RET gl_ble_get_identity_address(BLE_MAC address, int *address_type)
{
	GL_RET ret;
	pthread_mutex_lock(&_ble_mutex);
	ret = ble_get_identity_address(address, address_type);
	pthread_mutex_unlock(&_ble_mutex);
	return ret;
}