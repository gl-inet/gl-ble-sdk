/*****************************************************************************
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

#ifndef _GL_TYPE_H_
#define _GL_TYPE_H_

/**
 * @addtogroup  TYPE
 * @{
 */

#include <stdbool.h>
#include <stdint.h>


#define UUID_MAX                    128
#define LIST_LENGTHE_MAX            16
#define CHAR_VALUE_MAX              256
#define DEVICE_MAC_LEN              6
#define BLE_MAC_LEN                 18
#define MAX_VALUE_DATA_LEN          255
#define MAX_ADV_DATA_LEN            255
#define MAX_HASH_DATA_LEN           255

/**
 * @brief service node.
 */
typedef struct {
    int32_t handle;
    char uuid[UUID_MAX];
} ble_service_node_t;

/**
 * @brief characteristic node.
 */
typedef struct {
    int32_t handle;
    char uuid[UUID_MAX];
    uint8_t properties;
} ble_characteristic_node_t;

/**
 * @brief BLE 48-bit MAC.
 */
typedef uint8_t BLE_MAC[DEVICE_MAC_LEN];

/**
 * @brief service list.
 */
typedef struct {
    uint8_t list_len; ///< length of service list
    ble_service_node_t list[LIST_LENGTHE_MAX]; ///< array of service node
} gl_ble_service_list_t;

/**
 * @brief characteristic list.
 */
typedef struct {
    uint8_t list_len; ///< length of characteristic list
    ble_characteristic_node_t list[LIST_LENGTHE_MAX]; ///< array of characteristic node
} gl_ble_char_list_t;


/**
 * @brief module callback event type.
 */
typedef enum {
    // MODULE_BOOT = 0,
    MODULE_BLE_SYSTEM_BOOT_EVT = 0,
    MODULE_EVT_MAX,
} gl_ble_module_event_t;


typedef union {
    struct ble_system_boot_data {
        int32_t major;
        int32_t minor;
        int32_t patch;
        int32_t build;
        int32_t bootloader;
        int32_t hw;
        char ble_hash[MAX_HASH_DATA_LEN];
    } system_boot_data;

} gl_ble_module_data_t;


/**
 * @brief GAP BLE callback event type.
 */
typedef enum {
    GAP_BLE_SCAN_RESULT_EVT = 0,
    GAP_BLE_UPDATE_CONN_EVT,
    GAP_BLE_CONNECT_EVT,
    GAP_BLE_DISCONNECT_EVT,
    GAP_BLE_EVT_MAX,
} gl_ble_gap_event_t;


/**
 * @brief BLE device address type.
 */
typedef enum {
    BLE_ADDR_TYPE_PUBLIC = 0x00,
    BLE_ADDR_TYPE_RANDOM = 0x01,
    BLE_ANONYMOUS_ADVERTISING = 0xff,

} gl_ble_addr_type_t;

typedef union {
    struct ble_scan_result_evt_data {
        BLE_MAC address;
        gl_ble_addr_type_t ble_addr_type; 
        int32_t packet_type;  
        int32_t rssi;  
        uint8_t ble_adv_len;
        uint8_t ble_adv[MAX_ADV_DATA_LEN];
        int32_t bonding;
    } scan_rst;

    struct ble_update_conn_evt_data {
        BLE_MAC address;
        int32_t interval; 
        int32_t latency;
        int32_t timeout;
        int32_t security_mode;
        int32_t txsize;
    } update_conn_data;

    struct ble_connect_open_evt_data {
        BLE_MAC address; 
        gl_ble_addr_type_t ble_addr_type;
        int32_t conn_role;
        int32_t bonding;
        int32_t advertiser;
    } connect_open_data;

    struct ble_disconnect_evt_data {
        BLE_MAC address;
        int32_t reason;
    } disconnect_data;
} gl_ble_gap_data_t;


/**
 * @brief GATT BLE callback event type.
 */
typedef enum {
    GATT_REMOTE_CHARACTERISTIC_VALUE_EVT = 0,
    GATT_LOCAL_GATT_ATT_EVT,
    GATT_LOCAL_CHARACTERISTIC_STATUS_EVT,
    GATT_EVT_MAX,
} gl_ble_gatt_event_t;


typedef enum {
    GATT_READ_BY_TYPE_REQ                                       = 0x8,
    GATT_READ_BY_TYPE_RESP                                      = 0x9,
    GATT_READ_REQ                                               = 0xa,
    GATT_READ_RESP                                              = 0xb,
    GATT_READ_BLOB_REQ                                          = 0xc,
    GATT_READ_BLOB_RESP                                         = 0xd,
    GATT_READ_MULTIPLE_REQ                                      = 0xe,
    GATT_READ_MULTIPLE_RESP                                     = 0xf,
    GATT_WRITE_REQ                                              = 0x12,
    GATT_WRITE_RESP                                             = 0x13,
    GATT_WRITE_CMD                                              = 0x52,
    GATT_PREPARE_WRITE_REQ                                      = 0x16,
    GATT_PREPARE_WRITE_RESP                                     = 0x17,
    GATT_EXECUTE_WRITE_REQ                                      = 0x18,
    GATT_EXECUTE_WRITE_RESP                                     = 0x19,
    GATT_HANDLE_VALUE_NOTIFICATION                              = 0x1b,
    GATT_HANDLE_VALUE_INDICATION                                = 0x1d,
} gl_ble_att_opcode_t;

typedef enum {
    GATT_SERVER_CLIENT_CONFIG                                    = 0x1,
    GATT_SERVER_CONFIRMATION                                     = 0x2,
} gl_ble_local_characteristic_status_flags_t;

typedef enum {
    GATT_DISABLE                                                 = 0x0,
    GATT_NOTIFICATION                                            = 0x1,
    GATT_INDICATION                                              = 0x2,
}gl_ble_gatt_client_config_flag_t;

typedef union {
    struct ble_remote_characteristic_value_evt_data {
        BLE_MAC address;
        int32_t characteristic;
        gl_ble_att_opcode_t att_opcode;
        int32_t offset;
        char value[MAX_VALUE_DATA_LEN];

    } remote_characteristic_value;
    struct ble_local_gatt_att_evt_data {
        BLE_MAC address;
        int32_t attribute;
        gl_ble_att_opcode_t att_opcode;
        int32_t offset;
        char value[MAX_VALUE_DATA_LEN];

    } local_gatt_attribute;
    struct ble_local_characteristic_status_evt_data {
        BLE_MAC address;
        int32_t characteristic;
        gl_ble_local_characteristic_status_flags_t status_flags;
        gl_ble_gatt_client_config_flag_t client_config_flags;
    } local_characteristic_status;

} gl_ble_gatt_data_t;


/**
 * @brief callback func.
 */
typedef struct {
    int32_t (*ble_module_event)(gl_ble_module_event_t event, gl_ble_module_data_t *data);
    int32_t (*ble_gap_event)(gl_ble_gap_event_t event, gl_ble_gap_data_t *data);
    int32_t (*ble_gatt_event)(gl_ble_gatt_event_t event, gl_ble_gatt_data_t *data);
} gl_ble_cbs;

#endif