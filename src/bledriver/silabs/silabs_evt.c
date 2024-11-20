/*****************************************************************************
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
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "silabs_msg.h"
#include "gl_common.h"
// #include "bg_types.h"
#include "sl_bt_api.h"
#include "gl_log.h"
#include "gl_dev_mgr.h"
#include "gl_type.h"

#include "silabs_evt.h"
#include "sli_bt_api.h"
#include "sl_bt_version.h"

#define ADV_DATA_COMPLETE 0
#define ADV_DATA_INCOMPLETE 1
#define ADV_DATA_TRUNCATED 2

//global var
bool ble_version_match = false;
bool module_boot = false;

static int evt_msgid;
static silabs_msg_queue_t queue_data;
static uint8_t  long_extended_data[MAX_ADV_DATA_LEN];
static uint16_t long_extended_data_len = 0;
static uint8_t  long_sync_data[MAX_ADV_DATA_LEN];
static uint16_t long_sync_data_len = 0;
static bool extended_adv_data_constuct(struct sl_bt_packet *p_in);
static bool periodic_adv_data_constuct(struct sl_bt_packet *p_in);

void *silabs_watcher(void *arg)
{
    watcher_param_t *sbs_param = (watcher_param_t *)arg;
    evt_msgid = sbs_param->evt_msgid;
    gl_ble_cbs *ble_msg_cb = sbs_param->cbs;

    while (1)
    {
        // set cancellation point
        pthread_testcancel();

        // recv evt from msg queue
        if (-1 == msgrcv(evt_msgid, (void *)&queue_data, sizeof(struct sl_bt_packet), 0, 0))
        {
            printf("msgrcv: %s\n", strerror(errno));
            log_err("msgrcv error!!!\n");
            continue;
        }

        struct sl_bt_packet *p = &(queue_data.evt);

        switch (SL_BT_MSG_ID(p->header))
        {
        case sl_bt_evt_system_boot_id:
        {
            gl_ble_module_data_t data;
            data.system_boot_data.major = p->data.evt_system_boot.major;
            data.system_boot_data.minor = p->data.evt_system_boot.minor;
            data.system_boot_data.patch = p->data.evt_system_boot.patch;
            data.system_boot_data.build = p->data.evt_system_boot.build;
            data.system_boot_data.bootloader = p->data.evt_system_boot.bootloader;
            data.system_boot_data.hw = p->data.evt_system_boot.hw;
            hex2str((uint8_t *)&p->data.evt_system_boot.hash, sizeof(uint32_t), data.system_boot_data.ble_hash);

            if(ble_version_match == false)
            {
                if((data.system_boot_data.major == BG_VERSION_MAJOR) && (data.system_boot_data.minor == BG_VERSION_MINOR) 
                    && (data.system_boot_data.patch == BG_VERSION_PATCH))
                {
                    ble_version_match = true;
                }
            }
            module_boot = true;

            if (ble_msg_cb->ble_module_event)
            {
                ble_msg_cb->ble_module_event(MODULE_BLE_SYSTEM_BOOT_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_connection_closed_id:
        {
            gl_ble_gap_data_t data;
            data.disconnect_data.reason = p->data.evt_connection_closed.reason;
            char tmp_address[MAC_STR_LEN] = {0};
            uint16_t ret = ble_dev_mgr_get_address(p->data.evt_connection_closed.connection, tmp_address);
            if (ret != 0)
            {
                log_err("get dev mac from dev-list failed!\n");
                return NULL;
            }
            str2addr(tmp_address, data.disconnect_data.address);

            // delete from dev-list
            ble_dev_mgr_del(p->data.evt_connection_closed.connection);

            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_DISCONNECT_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_gatt_characteristic_value_id:
        {
            gl_ble_gatt_data_t data;
            data.remote_characteristic_value.offset = p->data.evt_gatt_characteristic_value.offset;
            data.remote_characteristic_value.att_opcode = p->data.evt_gatt_characteristic_value.att_opcode;
            data.remote_characteristic_value.characteristic = p->data.evt_gatt_characteristic_value.characteristic;
            hex2str(p->data.evt_gatt_characteristic_value.value.data, p->data.evt_gatt_characteristic_value.value.len, data.remote_characteristic_value.value);

            char tmp_address[MAC_STR_LEN] = {0};
            uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_characteristic_value.connection, tmp_address);
            if (ret != 0)
            {
                log_err("get dev mac from dev-list failed!\n");
                return NULL;
            }
            str2addr(tmp_address, data.remote_characteristic_value.address);

            if (ble_msg_cb->ble_gatt_event)
            {
                ble_msg_cb->ble_gatt_event(GATT_REMOTE_CHARACTERISTIC_VALUE_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_gatt_server_attribute_value_id:
        {
            gl_ble_gatt_data_t data;
            data.local_gatt_attribute.offset = p->data.evt_gatt_server_attribute_value.offset;
            data.local_gatt_attribute.attribute = p->data.evt_gatt_server_attribute_value.attribute;
            data.local_gatt_attribute.att_opcode = p->data.evt_gatt_server_attribute_value.att_opcode;
            hex2str(p->data.evt_gatt_server_attribute_value.value.data, p->data.evt_gatt_server_attribute_value.value.len, data.local_gatt_attribute.value);

            char tmp_address[MAC_STR_LEN] = {0};
            uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_server_attribute_value.connection, tmp_address);
            if (ret != 0)
            {
                log_err("get dev mac from dev-list failed!\n");
                return NULL;
            }
            str2addr(tmp_address, data.local_gatt_attribute.address);

            if (ble_msg_cb->ble_gatt_event)
            {
                ble_msg_cb->ble_gatt_event(GATT_LOCAL_GATT_ATT_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_gatt_server_characteristic_status_id:
        {
            gl_ble_gatt_data_t data;
            data.local_characteristic_status.status_flags = p->data.evt_gatt_server_characteristic_status.status_flags;
            data.local_characteristic_status.characteristic = p->data.evt_gatt_server_characteristic_status.characteristic;
            data.local_characteristic_status.client_config_flags = p->data.evt_gatt_server_characteristic_status.client_config_flags;

            char tmp_address[MAC_STR_LEN] = {0};
            uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_server_characteristic_status.connection, tmp_address);
            if (ret != 0)
            {
                log_err("get dev mac from dev-list failed!\n");
                return NULL;
            }
            str2addr(tmp_address, data.local_characteristic_status.address);

            if (ble_msg_cb->ble_gatt_event)
            {
                ble_msg_cb->ble_gatt_event(GATT_LOCAL_CHARACTERISTIC_STATUS_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_scanner_legacy_advertisement_report_id:
        {
            gl_ble_gap_data_t data;
            data.legacy_scan_rst.rssi = p->data.evt_scanner_legacy_advertisement_report.rssi;
            data.legacy_scan_rst.bonding = p->data.evt_scanner_legacy_advertisement_report.bonding;
            data.legacy_scan_rst.event_flags = ble_get_adv_type(p->data.evt_scanner_legacy_advertisement_report.event_flags);
            data.legacy_scan_rst.ble_addr_type = p->data.evt_scanner_legacy_advertisement_report.address_type;
            data.legacy_scan_rst.ble_adv_len = p->data.evt_scanner_legacy_advertisement_report.data.len;

            memset(data.legacy_scan_rst.ble_adv, 0, MAX_LEGACY_ADV_DATA_LEN);
            memcpy(data.legacy_scan_rst.ble_adv, p->data.evt_scanner_legacy_advertisement_report.data.data, data.legacy_scan_rst.ble_adv_len);
            memcpy(data.legacy_scan_rst.address, p->data.evt_scanner_legacy_advertisement_report.address.addr, 6);

            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_LEGACY_SCAN_RESULT_EVT, &data);
            }
            break;
        }
        case sl_bt_evt_scanner_extended_advertisement_report_id:
        {
            if (extended_adv_data_constuct(p))
            {
                gl_ble_gap_data_t data;
                data.extended_scan_rst.periodic_interval = p->data.evt_scanner_extended_advertisement_report.periodic_interval;
                data.extended_scan_rst.adv_sid = p->data.evt_scanner_extended_advertisement_report.adv_sid;
                data.extended_scan_rst.tx_power = p->data.evt_scanner_extended_advertisement_report.tx_power;
                data.extended_scan_rst.rssi = p->data.evt_scanner_extended_advertisement_report.rssi;
                data.extended_scan_rst.bonding = p->data.evt_scanner_extended_advertisement_report.bonding;
                data.extended_scan_rst.ble_adv_len = long_extended_data_len;
                data.extended_scan_rst.event_flags = ble_get_adv_type(p->data.evt_scanner_extended_advertisement_report.event_flags);
                data.extended_scan_rst.ble_addr_type = p->data.evt_scanner_extended_advertisement_report.address_type;
                memcpy(data.extended_scan_rst.address, p->data.evt_scanner_extended_advertisement_report.address.addr, 6);
                memcpy(data.extended_scan_rst.ble_adv, long_extended_data, data.extended_scan_rst.ble_adv_len);
                if (ble_msg_cb->ble_gap_event)
                {
                    ble_msg_cb->ble_gap_event(GAP_BLE_EXTENDED_SCAN_RESULT_EVT, &data);
                }
            }
            break;
        }
        case sl_bt_evt_sync_opened_id:
        {
            gl_ble_gap_data_t data;
            data.sync_opened_rst.sync_handle = p->data.evt_sync_opened.sync;
            data.sync_opened_rst.adv_sid = p->data.evt_sync_opened.adv_sid;
            data.sync_opened_rst.ble_addr_type = p->data.evt_sync_opened.address_type;
            memcpy(data.sync_opened_rst.address, p->data.evt_sync_opened.address.addr, 6);
            data.sync_opened_rst.adv_phy = p->data.evt_sync_opened.adv_phy;
            data.sync_opened_rst.periodic_interval = p->data.evt_sync_opened.adv_interval;
            data.sync_opened_rst.clock_accuracy = p->data.evt_sync_opened.clock_accuracy;
            data.sync_opened_rst.bonding = p->data.evt_sync_opened.bonding;
            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_SYNC_OPENED_EVT, &data);
            }
            break;
        }
        case sl_bt_evt_sync_data_id:
        {
            if (periodic_adv_data_constuct(p))
            {
                gl_ble_gap_data_t data;
                data.sync_scan_rst.ble_adv_len = long_sync_data_len;
                data.sync_scan_rst.rssi = p->data.evt_sync_data.rssi;
                data.sync_scan_rst.tx_power = p->data.evt_sync_data.tx_power;
                data.sync_scan_rst.sync_handle = p->data.evt_sync_data.sync;
                memcpy(data.sync_scan_rst.ble_adv, long_sync_data, data.sync_scan_rst.ble_adv_len);
                if (ble_msg_cb->ble_gap_event)
                {
                    ble_msg_cb->ble_gap_event(GAP_BLE_SYNC_SCAN_RESULT_EVT, &data);
                }
            }
            break;
        }
        case sl_bt_evt_sync_closed_id:
        {
            gl_ble_gap_data_t data;
            data.sync_closed_rst.sync_handle = p->data.evt_sync_closed.sync;
            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_SYNC_CLOSED_EVT, &data);
            }
            break;
        }
        case sl_bt_evt_connection_parameters_id:
        {
            gl_ble_gap_data_t data;
            data.update_conn_data.txsize = p->data.evt_connection_parameters.txsize;
            data.update_conn_data.latency = p->data.evt_connection_parameters.latency;
            data.update_conn_data.timeout = p->data.evt_connection_parameters.timeout;
            data.update_conn_data.interval = p->data.evt_connection_parameters.interval;
            data.update_conn_data.security_mode = p->data.evt_connection_parameters.security_mode;

            char tmp_address[MAC_STR_LEN] = {0};
            uint16_t ret = ble_dev_mgr_get_address(p->data.evt_connection_parameters.connection, tmp_address);
            if (ret != 0)
            {
                log_err("get dev mac from dev-list failed!\n");
                return NULL;
            }
            str2addr(tmp_address, data.update_conn_data.address);

            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_UPDATE_CONN_EVT, &data);
            }

            break;
        }
        case sl_bt_evt_connection_opened_id:
        {
            char addr[MAC_STR_LEN] = {0};
            addr2str(p->data.evt_connection_opened.address.addr, addr);
            ble_dev_mgr_add(addr, p->data.evt_connection_opened.connection);

            gl_ble_gap_data_t data;
            data.connect_open_data.bonding = p->data.evt_connection_opened.bonding;
            data.connect_open_data.conn_role = p->data.evt_connection_opened.master;
            data.connect_open_data.advertiser = p->data.evt_connection_opened.advertiser;
            data.connect_open_data.ble_addr_type = p->data.evt_connection_opened.address_type;
            memcpy(data.connect_open_data.address, p->data.evt_connection_opened.address.addr, 6);

            if (ble_msg_cb->ble_gap_event)
            {
                ble_msg_cb->ble_gap_event(GAP_BLE_CONNECT_EVT, &data);
            }

            break;
        }
        default:
            break;
        }
    }

    return NULL;
}

static bool extended_adv_data_constuct(struct sl_bt_packet *p_in)
{
    static uint16_t long_extended_data_len_p = 0;
    static uint8_t addr_type_record;
    static bd_addr addr_record;
    static uint8_t sid_record;
    static bool is_record = false;

    if (p_in->data.evt_scanner_extended_advertisement_report.data_completeness == ADV_DATA_TRUNCATED)
    {
        is_record = false;
        memset(long_extended_data, 0, MAX_ADV_DATA_LEN);
        long_extended_data_len_p = 0;
        return false;
    }

    if (p_in->data.evt_scanner_extended_advertisement_report.data_completeness == ADV_DATA_INCOMPLETE)
    {
        if (!is_record)
        {
            is_record = true;
            memset(long_extended_data, 0, MAX_ADV_DATA_LEN);
            long_extended_data_len_p = 0;
            addr_type_record = p_in->data.evt_scanner_extended_advertisement_report.address_type;
            memcpy(&addr_record, &(p_in->data.evt_scanner_extended_advertisement_report.address), sizeof(bd_addr));
            sid_record = p_in->data.evt_scanner_extended_advertisement_report.adv_sid;
        }
        else
        {
            if (addr_type_record != p_in->data.evt_scanner_extended_advertisement_report.address_type ||
                memcmp(&addr_record, &(p_in->data.evt_scanner_extended_advertisement_report.address), sizeof(bd_addr)) ||
                sid_record != p_in->data.evt_scanner_extended_advertisement_report.adv_sid)
            {
                return false;
            }
            else
            {
                if ((long_extended_data_len_p + p_in->data.evt_scanner_extended_advertisement_report.data.len) >= MAX_ADV_DATA_LEN)
                {
                    long_extended_data_len_p += p_in->data.evt_scanner_extended_advertisement_report.data.len;
                    return false;
                }
            }
        }
    }

    memcpy(&long_extended_data[long_extended_data_len_p], p_in->data.evt_scanner_extended_advertisement_report.data.data,
            p_in->data.evt_scanner_extended_advertisement_report.data.len);
    long_extended_data_len_p += p_in->data.evt_scanner_extended_advertisement_report.data.len;

    if (p_in->data.evt_scanner_extended_advertisement_report.data_completeness == ADV_DATA_COMPLETE)
    {
        is_record = false;
        if(long_extended_data_len_p > MAX_ADV_DATA_LEN)
        {
            memset(long_extended_data, 0, MAX_ADV_DATA_LEN);
            long_extended_data_len_p = 0;
            return false;
        }
        else
        {
            long_extended_data_len = long_extended_data_len_p;
            long_extended_data_len_p = 0;
            return true;
        }
    }

    return false;
}

static bool periodic_adv_data_constuct(struct sl_bt_packet *p_in)
{
    static uint16_t long_sync_data_len_p = 0;
    static uint16_t sync_handle_record;
    static bool is_record = false;

    if (p_in->data.evt_sync_data.data_status == ADV_DATA_TRUNCATED)
    {
        is_record = false;
        memset(long_sync_data, 0, MAX_ADV_DATA_LEN);
        long_sync_data_len_p = 0;
        return false;
    }

    if (p_in->data.evt_sync_data.data_status == ADV_DATA_INCOMPLETE)
    {
        if (!is_record)
        {
            is_record = true;
            memset(long_sync_data, 0, MAX_ADV_DATA_LEN);
            long_sync_data_len_p = 0;
            sync_handle_record = p_in->data.evt_sync_data.sync;
        }
        else
        {
            if (sync_handle_record != p_in->data.evt_sync_data.sync)
            {
                return false;
            }
            else
            {
                if ((long_sync_data_len_p + p_in->data.evt_sync_data.data.len) >= MAX_ADV_DATA_LEN)
                {
                    long_sync_data_len_p += p_in->data.evt_sync_data.data.len;
                    return false;
                }
            }
        }
    }

    memcpy(&long_sync_data[long_sync_data_len_p], p_in->data.evt_sync_data.data.data,
            p_in->data.evt_sync_data.data.len);
    long_sync_data_len_p += p_in->data.evt_sync_data.data.len;

    if (p_in->data.evt_sync_data.data_status == ADV_DATA_COMPLETE)
    {
        is_record = false;
        if(long_sync_data_len_p > MAX_ADV_DATA_LEN)
        {
            memset(long_sync_data, 0, MAX_ADV_DATA_LEN);
            long_sync_data_len_p = 0;
            return false;
        }
        else
        {
            long_sync_data_len = long_sync_data_len_p;
            long_sync_data_len_p = 0;
            return true;
        }
    }

    return false;
}