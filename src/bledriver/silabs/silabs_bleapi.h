/*****************************************************************************
 * @file 
 * @brief 
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

#ifndef _SILABS_BLEAPI_H
#define _SILABS_BLEAPI_H

#include "gl_errno.h"

GL_RET silabs_ble_enable(int);

GL_RET silabs_ble_hard_reset(void);

GL_RET silabs_ble_local_mac(BLE_MAC mac);

GL_RET silabs_ble_start_discovery(uint8_t phys, uint16_t interval, uint16_t window, uint8_t type, uint8_t mode);

GL_RET silabs_ble_stop_discovery(void);

GL_RET silabs_ble_set_sync_parameters(uint16_t skip, uint16_t timeout);

GL_RET silabs_ble_start_sync(BLE_MAC address, uint8_t address_type, uint8_t adv_sid, uint16_t *handle);

GL_RET silabs_ble_stop_sync(uint16_t handle);

GL_RET silabs_ble_create_adv_handle(uint8_t *handle);

GL_RET silabs_ble_delete_adv_handle(uint8_t handle);

GL_RET silabs_ble_start_legacy_adv(uint8_t handle, uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect);

GL_RET silabs_ble_start_extended_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, 
                        uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect);

GL_RET silabs_ble_start_periodic_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, uint16_t interval_min, uint16_t interval_max);

GL_RET silabs_ble_stop_adv(uint8_t handle);

GL_RET silabs_ble_set_legacy_adv_data(uint8_t handle, uint8_t flag, const char *data);

GL_RET silabs_ble_set_extended_adv_data(uint8_t handle, const char *data);

GL_RET silabs_ble_set_periodic_adv_data(uint8_t handle, const char *data);

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

GL_RET silabs_ble_set_gattdb(char *json_cfg_name);

GL_RET silabs_ble_del_gattdb(void);

GL_RET silabs_ble_set_identity_address(BLE_MAC address, int address_type);

GL_RET silabs_ble_get_identity_address(BLE_MAC address, int *address_type);

#endif