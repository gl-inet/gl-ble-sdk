/*****************************************************************************
 * @file 
 * @brief 
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

#ifndef _SILABS_BLEAPI_H
#define _SILABS_BLEAPI_H

#include "gl_errno.h"


GL_RET silabs_ble_enable(int);

GL_RET silabs_ble_hard_reset(void);

GL_RET silabs_ble_local_mac(BLE_MAC mac);

GL_RET silabs_ble_discovery(int phys, int interval, int window, int type, int mode);

GL_RET silabs_ble_stop_discovery(void);

GL_RET silabs_ble_adv(int phys, int interval_min, int interval_max, int discover, int adv_conn);

GL_RET silabs_ble_adv_data(int flag, char *data);

GL_RET silabs_ble_stop_adv(void);

GL_RET silabs_ble_send_notify(BLE_MAC address, int char_handle, char *value);

GL_RET silabs_ble_connect(BLE_MAC address, int address_type, int phy);

GL_RET silabs_ble_disconnect(BLE_MAC address);

GL_RET silabs_ble_get_rssi(BLE_MAC address, int32_t *rssi);

GL_RET silabs_ble_get_service(gl_ble_service_list_t *service_list, BLE_MAC address);

GL_RET silabs_ble_get_char(gl_ble_char_list_t *char_list, BLE_MAC address, int service_handle);

GL_RET silabs_ble_set_power(int power, int *current_power);

GL_RET silabs_ble_read_char(BLE_MAC address, int char_handle);

GL_RET silabs_ble_write_char(BLE_MAC address, int char_handle, char *value, int res);

GL_RET silabs_ble_set_notify(BLE_MAC address, int char_handle, int flag);


#endif