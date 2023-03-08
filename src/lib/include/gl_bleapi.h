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

#ifndef _GLBLEAPI_H
#define _GLBLEAPI_H

/**
 * @addtogroup USER API
 * @{
 */

#include "gl_type.h"
#include "gl_errno.h"

/**
 *  @brief  This function will init ble thread. 
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_init(void);

/**
 *  @brief  This function will destroy the ble thread and wait for thread resources to be received.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_destroy(void);


/**
 *  @brief  This function will subscribe events generate from BLE module. 
 * 
 *  @warning  Note that it will create a new thread(watcher) to get event. If you want to destroy watcher thread, please call gl_ble_unsubscribe().
 * 
 *  @param callback: This callback will be called when module receive a system boot, GAP and GATT event. 
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_subscribe(gl_ble_cbs *callback);

/**
 *  @brief  This function will unsubscribe events generate from BLE module. 
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_unsubscribe(void);

/**
 *  @brief  Enable or disable the BLE module.
 * 
 *  @param enable : The value to enable or disable the BLE module.
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   When you need to use the BLE, you should call this API first to make sure the BLE chip works properly.
 * 			This function actually controls the ble chip through the Reset IO. \n
 * 			Every time when the ble chip start, "ble_module_event" callback will receive
 *  		a "MODULE_BLE_SYSTEM_BOOT_EVT" event.
 */
GL_RET gl_ble_enable(int enable);

/**
 *  @brief  Hardware reset the ble modele
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   Every time when the ble chip start, "ble_module_event" callback will receive
 *  		a "MODULE_BLE_SYSTEM_BOOT_EVT" event.
 */
GL_RET gl_ble_hard_reset(void);

/**
 *  @brief  This command can be used to read the Bluetooth public address used by the device. 
 * 
 *  @param mac: Device's BLE MAC address.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_get_mac(BLE_MAC mac);

/**
 *  @brief  This command can be used to set the global maximum TX power for Bluetooth. 
 * 
 *  @param power : TX power in 0.1 dBm steps, for example the value of 10 is 1dBm and 55 is 
 *                 5.5 dBm.
 *  @param current_power : The selected maximum output power level after applying RF path compensation. 
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   By default, the global maximum TX power value is 8 dBm. This command should not
 *          be used while advertising, scanning or during connection. 
 */
GL_RET gl_ble_set_power(int power, int *current_power);

/**
 *  @brief  Act as BLE slave, set user defined data in legacy advertising packets.
 * 
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param flag : Legacy advertising packets type. \n
 *                    0: Legacy advertising packets, the maximum size is 31 bytes. \n 
 *                    1: Scan response packets, the maximum size is 31 bytes. \n
 * 
 *  @param data : Customized packet data. Must be hexadecimal ASCII. Like “020106” 
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  Before calling this function, make sure that the Advertising set handle is created.
 */
GL_RET gl_ble_set_legacy_adv_data(uint8_t handle, uint8_t flag, const char *data);

/**
 *  @brief  Act as BLE slave, set user defined data in extended advertising packets.
 * 
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param data : Customized Extended packet data. Must be hexadecimal ASCII. Like “020106”. The maximum size is 1024 bytes. \n 
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  Before calling this function, make sure that the Advertising set handle is created.
 */
GL_RET gl_ble_set_extended_adv_data(uint8_t handle, const char *data);

/**
 *  @brief  Act as BLE slave, set user defined data in periodic advertising packets.
 * 
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param data : Customized Periodic packet data. Must be hexadecimal ASCII. Like “020106”. The maximum size is 1024 bytes. \n 
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  Before calling this function, make sure that the Advertising set handle is created.
 */
GL_RET gl_ble_set_periodic_adv_data(uint8_t handle, const char *data);

/**
 *  @brief  Act as BLE slave, create Advertising set handle.
 * 
 *  @param handle : It is a output parameter. The handle of the created advertising set is returned in response if the operation succeeds. \n
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  Up to four Advertising set handle can be created.
 */
GL_RET gl_ble_create_adv_handle(uint8_t *handle);

/**
 *  @brief  Act as BLE slave, delete Advertising set handle.
 * 
 *  @param handle : The Advertising set handle which you want to delete. \n
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  If the Advertising set handle that it is broadcasting, and then call this function to delete it, the broadcast is stopped.
 */
GL_RET gl_ble_delete_adv_handle(uint8_t handle);

/**
 *  @brief  Act as BLE slave, Set and Start Legacy Avertising.
 *
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param interval_min : Minimum Legacy advertising interval.Value in units of 0.625 ms \n
 *                         Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s \n
 *  @param interval_max : Maximum Legacy/Extended advertising interval. Value in units of 0.625 ms \n
 *                         Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s \n
 *  @param discover :  Define the discoverable mode. \n
 *                     0: Not discoverable \n
 *                     1: Discoverable by both limited and general discovery procedures \n
 *                     2: Discoverable by the general discovery procedure \n
 *                     3: Send legacy advertising and/or scan response data defined by the user. \n 
 *                        The limited/general discoverable flags are defined by the user. \n
 *  @param connect : Define the Legacy Advertising connectable mode. \n
 *                     0: Undirected non-connectable and non-scannable legacy advertising \n
 *                     2: Undirected connectable and scannable legacy advertising \n
 *                     3: Undirected scannable and non-connectable legacy advertising \n
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   1.Before calling this function, make sure that the Advertising set handle is created. \n
 *          2.Interval_max should be bigger than interval_min. \n
 *          3.Legacy Avertising is transmitted on LE 1M PHY by default. \n 
 *          4.If you want multiple Avertising to run at the same time, make sure their advertising interval are different. 
 */
GL_RET gl_ble_start_legacy_adv(uint8_t handle, uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect);

/**
 *  @brief  Act as BLE slave, Set and Start Extended Avertising.
 *
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param primary_phy : The PHY on which the advertising packets are transmitted on the primary advertising channel. \n
 *                        1: LE 1M PHY, 4: LE Coded PHY(125k, S=8)
 *  @param secondary_phy : The PHY on which the advertising packets are transmitted on the secondary advertising channel. \n
 *                          1: LE 1M PHY, 2: LE 2M PHY, 4: LE Coded PHY(125k, S=8)
 *  @param interval_min : Minimum Extended advertising interval.Value in units of 0.625 ms \n
 *                         Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s \n
 *  @param interval_max : Maximum Extended advertising interval. Value in units of 0.625 ms \n
 *                         Range: 0x20 to 0xFFFFFF, Time range: 20 ms to 10485.759375 s \n
 *  @param discover :  Define the discoverable mode. \n
 *                     0: Not discoverable \n
 *                     1: Discoverable by both limited and general discovery procedures \n
 *                     2: Discoverable by the general discovery procedure \n
 *                     3: Send extended advertising data defined by the user. \n 
 *                        The limited/general discoverable flags are defined by the user. \n
 *  @param connect :   Define the Extended Advertising connectable mode. \n
 *                     0: Non-connectable and non-scannable extended advertising \n
 *                     3: Scannable extended advertising \n
 *                     4: Connectable extended advertising 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   1.Before calling this function, make sure that the Advertising set handle is created. \n
 *          2.Interval_max should be bigger than interval_min. \n
 *          3.If you want multiple Avertising to run at the same time, make sure their advertising interval are different. \n
 *          4.Maximum 191 bytes of data can be set for connectable extended advertising. \n
 *          5.When Extended advertising packet is more than 254 bytes and short advertising interval you set, it will cause lose packet problem. \n
 *            Here's recommend advertising interval of no less than 200ms.
 */
GL_RET gl_ble_start_extended_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, 
                        uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect);

/**
 *  @brief  Act as BLE slave, Set and Start Periodic Avertising.
 *
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @param primary_phy : The PHY on which the advertising packets are transmitted on the primary advertising channel. \n
 *                        1: LE 1M PHY, 4: LE Coded PHY(125k, S=8)
 *  @param secondary_phy : The PHY on which the advertising packets are transmitted on the secondary advertising channel. \n
 *                          1: LE 1M PHY, 2: LE 2M PHY, 4: LE Coded PHY(125k, S=8)
 *  @param interval_min : Minimum periodic advertising interval. Value in units of 1.25 ms \n 
 *                         Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s \n
 *  @param interval_max : Maximum periodic advertising interval. Value in units of 1.25 ms \n
 *                         Range: 0x06 to 0xFFFF, Time range: 7.5 ms to 81.92 s \n
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   1.Before calling this function, make sure that the Advertising set handle is created. \n
 *          2.Interval_max should be bigger than interval_min. \n
 *          3.If you want multiple Avertising to run at the same time, make sure their advertising interval are different. \n
 *          4.In case to lose packet problem, periodic advertising interval no less than 100m. \n
 *          5.The Periodic advertising interval you set should be smaller than synchronize timeout. If not, it will breaks the established synchronization. \n
 */
GL_RET gl_ble_start_periodic_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, uint16_t interval_min, uint16_t interval_max);

/**
 *  @brief  Act as BLE slave, stop the advertising of the given advertising set.
 * 
 *  @param handle : Advertising set handle, it be created by gl_ble_create_adv_handle. \n
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  1.Before calling this function, make sure that the Advertising set handle is created. \n
 *         2.It can stop Legacy/Extended/Periodic Avertising by Advertising set handle. \n
 *         3.Advertising set handle is still valid. You can continue to Advertising through it.
 */
GL_RET gl_ble_stop_adv(uint8_t handle);

/**
 *  @brief  Act as BLE slave, send notifications or indications to one or more remote GATT clients.
 * 
 *  @param address : Address of the connection over which the notification or indication is sent.
 *                   Like “11:22:33:44:55:66”.
 *  @param char_handle : GATT characteristic handle. 
 *  @param value : Value to be notified or indicated. Must be hexadecimal ASCII. Like “020106”
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_send_notify(BLE_MAC address, int char_handle, char *value);

/**
 *  @brief  Act as master, Set and start the BLE discovery.
 * 
 *  @param phys : The PHY on which the advertising packets are transmitted on. \n
 *                    1: LE 1M PHY, 4: LE Coded PHY, 5: Simultaneous LE 1M and Coded PHY alternatively
 *  @param interval : Scan interval. Time = Value x 0.625 ms. \n
 *                        Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
 *  @param window : Scan window. Time = Value x 0.625 ms. \n
 *                        Range: 0x0004 to 0xFFFF, Time Range: 2.5 ms to 40.96 s.
 *  @param type : Scan type.   \n
 *                        0: Passive scanning, 1: Active scanning. \n
 *                        In passive scanning mode, the device only listens to advertising \n
 *                        packets and does not transmit packets.
 *  @param mode : Bluetooth discovery Mode. \n
 *                    0: Discover only limited discoverable devices. \n
 *                    1: Discover limited and general discoverable devices. \n
 *                    2: Discover non-discoverable, limited and general discoverable devices. \n
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   In active scanning mode, the device sends out a scan request packet upon \n
 *          receiving an advertising packet from a remote device. Then, it listens to \n
 *          the scan response packet from the remote device. \n 
 */
GL_RET gl_ble_start_discovery(uint8_t phys, uint16_t interval, uint16_t window, uint8_t type, uint8_t mode);

/**
 *  @brief  Act as master, End the current GAP discovery procedure.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_stop_discovery(void);

/**
 *  @brief  Act as master, Set and start the BLE synchronize. 
 * 
 *  @param skip : The maximum number of periodic advertising packets that can be skipped after a successful receive. \n
 *                 Range: 0x0000 to 0x01F3. \n
 *  @param timeout : The maximum permitted time between successful receives. If this time is exceeded, synchronization is lost. Unit: 10 ms. \n 
 *                    Range: 0x0A to 0x4000, Time Range: 100 ms to 163.84 s. \n 
 *  @param address : Address of the device to synchronize to. Like “11:22:33:44:55:66”. \n
 *  @param address_type : Address type of the device to connect to. \n
 *                         0: Public address. \n
 *                         1: Random address.   
 *  @param adv_sid : Advertising set identifiers. \n
 *                   The adv_sid of a periodic advertising packet can be obtained by GAP_BLE_EXTENDED_SCAN_RESULT_EVT whether periodic_interval exists. \n
 *                   Periodic_interval equal 0 indicates no periodic advertising packet.
 *  @param handle : It is a output parameter. A handle that will be assigned to the periodic advertising synchronization after the synchronization is established. 
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note  The synchronize timeout you set should bigger than Periodic advertising interval. If not, it will breaks the established synchronization.
 */
GL_RET gl_ble_start_synchronize(uint16_t skip, uint16_t timeout, BLE_MAC address, uint8_t address_type, uint8_t adv_sid, uint16_t *handle);

/**
 *  @brief  Act as master, End the current GAP synchronize procedure.
 * 
 *  @param handle : Periodic advertising synchronization handle which want to stop.
 * 
 *  @param  GL-RETURN-CODE
 */
GL_RET gl_ble_stop_synchronize(uint16_t handle);

/**
 *  @brief  Act as master, Start connect to a remote BLE device.
 * 
 *  @param address : Address of the device to connect to. Like “11:22:33:44:55:66”.
 * 
 *  @param address_type : Address type of the device to connect to. \n
 *                            0: Public device address. \n
 *                            1: Static device address. \n
 *                            2: Resolvable private random address. \n
 *                            3: Non-resolvable private random address.
 *  @param phys : The PHY on which the advertising packets are transmitted on. \n
 *                    1: LE 1M PHY, 4: LE Coded PHY.
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   If connect success, it will report a GAP_event_callback "GAP_BLE_CONNECT_EVT".
 *          If a connection cannot be established at all for some reason (for example, the remote 
 *          device has gone out of range, has entered into deep sleep, or is not advertising), 
 *          the stack will try to connect forever. In this case the application will not get any event 
 *          related to the connection request. To recover from this situation, application can implement 
 *          a timeout and call gl_ble_disconnect() to cancel the connection request.
 */
GL_RET gl_ble_connect(BLE_MAC address, int address_type, int phy);

/**
 *  @brief  Act as master, disconnect with remote device.
 * 
 *  @param address : Address of the device to disconnect. Like “11:22:33:44:55:66”.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_disconnect(BLE_MAC address);

/**
 *  @brief  Act as master, get the latest RSSI value of a Bluetooth connection.
 * 
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 *  @param rssi : The RSSI value for the remote device.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_get_rssi(BLE_MAC address, int32_t *rssi);

/**
 *  @brief  Act as master, Get service list of a remote GATT server.
 * 
 *  @param service_list : The service list of the remote GATT server.
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_get_service(gl_ble_service_list_t *service_list, BLE_MAC address);

/**
 *  @brief  Act as master, Get characteristic list of a remote GATT server.
 * 
 *  @param char_list : The characteristic list of the remote GATT server.
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 *  @param service_handle : The service handle of connection with remote device.
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_get_char(gl_ble_char_list_t *char_list, BLE_MAC address, int service_handle);

/**
 *  @brief  Act as master, Read value of specified characteristic in a remote gatt server.
 * 
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 * 
 *  @param char_handle : The characteristic handle of connection with remote device.
 * 
 *  @retval  GL-RETURN-CODE
 * 
 *  @note   The value will report in gatt_event_callback "GATT_BLE_REMOTE_NOTIFY_EVT"
 */
GL_RET gl_ble_read_char(BLE_MAC address, int char_handle);

/**
 *  @brief  Act as master, Write value to specified characteristic in a remote gatt server.
 * 
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 *  @param char_handle : The characteristic handle of connection with remote device.
 *  @param value : Data value to be wrote. Must be hexadecimal ASCII. Like “020106”
 *  @param res : Response flag. \n
 * 					0: Write with no response \n
 * 					1: Write with response
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_write_char(BLE_MAC address, int char_handle, char *value, int res);

/**
 *  @brief  Act as master, Enable or disable the notification or indication of a remote gatt server.
 * 
 *  @param address : Remote BLE device MAC address. Like “11:22:33:44:55:66”.
 *  @param char_handle : The characteristic handle of connection with remote device.
 *  @param flag : Notification flag. \n
 *                      0: disable \n
 * 						1: notification \n
 * 						2: indication
 * 
 *  @retval  GL-RETURN-CODE
 */
GL_RET gl_ble_set_notify(BLE_MAC address, int char_handle, int flag);

/**
 *  @brief  Act as BLE slave, Set Local GATT DataBase and make it visible to remote GATT clients.
 * 
 *  @param  uci_cfg_name : The UCI file name that you want to set GATT database. \n
 * 
 *  @return GL-RETURN-CODE
 * 
 *  @note   Please refer to template of UCI file "/etc/config/gl_gattdb_cfg". \n
 *          After calling this function, the local GATT database will be cleared firstly. And then configure it according to the UCI file.
 */
GL_RET gl_ble_set_gattdb(char *uci_cfg_name);

/**
 * @brief  This function will dfu the ble module firmware to appropriate version.
 * 
 * @return GL-RETURN-CODE 
 * 
 * @note   Before using this function, please check the ble module firmware version firstly.
 *         It will not support some early devices.
 */
GL_RET gl_ble_module_dfu(void);

/**
 * @brief  This function will check the ble module whether boot and check the firmware version whether match the gl-ble-sdk version.
 *         If not, it will call the function gl_ble_module_dfu to update the ble module.
 * @param  callback : This callback will be called when module receive a system boot, GAP and GATT event. 
 * 
 * @return GL-RETURN-CODE 
 * 
 * @note   If the ble module can booted and version matched, will return GL_SUCCESS. Other return situation are not matched.
 */
GL_RET gl_ble_check_module(gl_ble_cbs *callback);

// /**
//  *  @brief  Act as BLE slave, Delete Local GATT DataBase and make it unvisible to remote GATT clients.
//  * 
//  *  @return GL-RETURN-CODE 
//  */
// GL_RET gl_ble_del_gattdb(void);

#endif