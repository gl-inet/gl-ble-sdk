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

#ifndef GL_METHODS_H
#define GL_METHODS_H

#define SILABS_EFR32
#ifdef SILABS_EFR32

#include "silabs_bleapi.h"
#include "silabs_msg.h"

#define ble_driver						silabs_driver
#define ble_watcher                     silabs_watcher

#define ble_enable                      silabs_ble_enable
#define ble_hard_reset                  silabs_ble_hard_reset
#define ble_local_mac                   silabs_ble_local_mac
#define ble_set_power                   silabs_ble_set_power
#define ble_start_discovery             silabs_ble_start_discovery
#define ble_stop_discovery              silabs_ble_stop_discovery
#define ble_set_sync_parameters         silabs_ble_set_sync_parameters
#define ble_start_sync                  silabs_ble_start_sync
#define ble_stop_sync                   silabs_ble_stop_sync
#define ble_create_adv_handle           silabs_ble_create_adv_handle
#define ble_delete_adv_handle           silabs_ble_delete_adv_handle
#define ble_start_legacy_adv            silabs_ble_start_legacy_adv
#define ble_start_extended_adv          silabs_ble_start_extended_adv
#define ble_start_periodic_adv          silabs_ble_start_periodic_adv
#define ble_stop_adv                    silabs_ble_stop_adv
#define ble_set_legacy_adv_data         silabs_ble_set_legacy_adv_data
#define ble_set_extended_adv_data       silabs_ble_set_extended_adv_data
#define ble_set_periodic_adv_data       silabs_ble_set_periodic_adv_data
#define ble_send_notify                 silabs_ble_send_notify
#define ble_connect                     silabs_ble_connect
#define ble_disconnect                  silabs_ble_disconnect
#define ble_get_rssi                    silabs_ble_get_rssi
#define ble_get_service                 silabs_ble_get_service
#define ble_get_char                    silabs_ble_get_char
#define ble_read_char                   silabs_ble_read_char
#define ble_write_char                  silabs_ble_write_char
#define ble_set_notify                  silabs_ble_set_notify
#define ble_set_gattdb                  silabs_ble_set_gattdb
// #define ble_del_gattdb                  silabs_ble_del_gattdb
#define ble_set_identity_address        silabs_ble_set_identity_address
#define ble_get_identity_address        silabs_ble_get_identity_address

#endif

#ifdef TLK_8251
#endif


#endif