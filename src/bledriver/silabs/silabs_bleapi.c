/*****************************************************************************
 * @file
 * @brief Bluetooth driver for silabs EFR32
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
#include <unistd.h>
#include <guci.h>

#include "sl_bt_api.h"
#include "gl_uart.h"
#include "gl_hal.h"
#include "gl_errno.h"
#include "gl_type.h"
#include "silabs_bleapi.h"
#include "gl_common.h"
#include "silabs_msg.h"
#include "gl_dev_mgr.h"

extern struct sl_bt_packet *evt;
extern bool wait_reset_flag;
extern bool appBooted;

// static uint8_t handle = 0xff;
// static uint16_t sync_identity = 0xffff;

GL_RET silabs_ble_enable(int enable)
{
    if (enable)
    {
        system(rston);
    }
    else
    {
        // wait sub thread recv end
        wait_reset_flag = true;
        // usleep(100*1000);
        while (wait_reset_flag)
        {
            usleep(10 * 1000);
        }
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_hard_reset(void)
{
    int reset_time = 0;
    int wait_s;
    int wait_off;

    // max retry 3 times
    while (reset_time < 3)
    {
        wait_s = 30;    // 3s = 30 * 100ms
        wait_off = 300; // 3s = 300 * 10ms

        wait_reset_flag = true;
        // wait for turn off ble module
        while ((wait_reset_flag) && (wait_off > 0))
        {
            wait_off--;
            usleep(10 * 1000);
        }

        // check turn off end
        if ((appBooted) || (wait_off <= 0))
        {
            // error
            reset_time++;
            continue;
        }

        // wait 300 ms
        usleep(300 * 1000);

        // turn on ble module
        system(rston);

        // wait for ble module start
        while ((!appBooted) && (wait_s > 0))
        {
            wait_s--;
            usleep(100 * 1000);
        }

        // if ble module start GL_SUCCESS, break loop
        if (appBooted)
        {
            break;
        }

        // ble module start timeout
        reset_time++;
    }

    if (reset_time < 3)
    {
        return GL_SUCCESS;
    }
    else
    {
        return GL_UNKNOW_ERR;
    }
}

GL_RET silabs_ble_local_mac(BLE_MAC mac)
{
    uint8_t type = 0; // Not open to the user layer
    bzero(mac, sizeof(BLE_MAC));

    sl_status_t status = sl_bt_system_get_identity_address((bd_addr *)mac, &type);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_start_discovery(uint8_t phys, uint16_t interval, uint16_t window, uint8_t type, uint8_t mode)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_scanner_set_parameters(type, interval, window);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    status = sl_bt_scanner_start(phys, mode);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_discovery(void)
{
    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_scanner_stop();
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_start_synchronize(uint16_t skip, uint16_t timeout, BLE_MAC address, uint8_t address_type, uint8_t adv_sid, uint16_t *handle)
{
    bd_addr addr;
    memcpy(addr.addr, address, 6);
    sl_status_t status = SL_STATUS_FAIL;

    // // stop scanner, in case scanning is enabled in the stack
    // status = sl_bt_scanner_stop();
    // if (status != SL_STATUS_OK)
    // {
    //     return GL_UNKNOW_ERR;
    // }

    // set sync and start
    status = sl_bt_sync_set_parameters(skip, timeout, 0);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    status = sl_bt_sync_open(addr, address_type, adv_sid, handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_synchronize(uint16_t handle)
{
    sl_status_t status = SL_STATUS_FAIL;
    
    status = sl_bt_sync_close(handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_create_adv_handle(uint8_t *handle)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_advertiser_create_set(handle);
    if (status != SL_STATUS_OK)
    {
        *handle = 0xff;
        return GL_UNKNOW_ERR;
    }
    return GL_SUCCESS;
}

GL_RET silabs_ble_delete_adv_handle(uint8_t handle)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_advertiser_delete_set(handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }
    return GL_SUCCESS;
}

GL_RET silabs_ble_start_legacy_adv(uint8_t handle, uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect)
{
    sl_status_t status = SL_STATUS_FAIL;
    // status = sl_bt_advertiser_create_set(handle);
    // if (status != SL_STATUS_OK)
    // {
    //     *handle = 0xff;
    //     return GL_UNKNOW_ERR;
    // }

    status = sl_bt_extended_advertiser_set_phy(handle, sl_bt_gap_phy_1m, sl_bt_gap_phy_1m);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    status = sl_bt_advertiser_set_timing(handle, interval_min, interval_max, 0, 0);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    if(discover >= 0 && discover < 3)//if not use custom data, it will set 
    {
        status = sl_bt_legacy_advertiser_generate_data(handle, discover);
        if (status != SL_STATUS_OK)
        {
            goto exit;
        }
    }

    status = sl_bt_legacy_advertiser_start(handle, connect);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }
    return GL_SUCCESS;

exit:
    // sl_bt_advertiser_delete_set(handle);
    return GL_UNKNOW_ERR;
}

GL_RET silabs_ble_start_extended_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, 
                        uint32_t interval_min, uint32_t interval_max, uint8_t discover, uint8_t connect)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_extended_advertiser_set_phy(handle, primary_phy, secondary_phy);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    status = sl_bt_advertiser_set_timing(handle, interval_min, interval_max, 0, 0);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    if(discover >= 0 && discover < 3)//if not use custom data, it will set 
    {
        status = sl_bt_extended_advertiser_generate_data(handle, discover);
        if (status != SL_STATUS_OK)
        {
            goto exit;
        }
    }
    status = sl_bt_extended_advertiser_start(handle, connect, 0);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }
    return GL_SUCCESS;

exit:
    // sl_bt_advertiser_delete_set(handle);
    return GL_UNKNOW_ERR;
}

GL_RET silabs_ble_start_periodic_adv(uint8_t handle, uint8_t primary_phy, uint8_t secondary_phy, 
                        uint16_t interval_min, uint16_t interval_max)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_extended_advertiser_set_phy(handle, primary_phy, secondary_phy);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

    status = sl_bt_periodic_advertiser_start(handle, interval_min, interval_max, SL_BT_PERIODIC_ADVERTISER_AUTO_START_EXTENDED_ADVERTISING);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }
    return GL_SUCCESS;

exit:
    // sl_bt_advertiser_delete_set(handle);
    return GL_UNKNOW_ERR;
}

GL_RET silabs_ble_set_legacy_adv_data(uint8_t handle, uint8_t flag, const char *data)
{
    if ((!data) || (strlen(data) % 2) || (strlen(data) / 2) > MAX_LEGACY_ADV_DATA_LEN)
    {
        return GL_ERR_PARAM;
    }

    uint8_t len = strlen(data) / 2;
    uint8_t *adv_data = (uint8_t *)calloc(len, sizeof(uint8_t));
    str2array(adv_data, data, len);

    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_legacy_advertiser_set_data(handle, flag, len, adv_data);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_set_extended_adv_data(uint8_t handle, const char *data)
{
    if ((!data) || (strlen(data) % 2) || (strlen(data) / 2) > MAX_ADV_DATA_LEN)
    {
        return GL_ERR_PARAM;
    }

    uint16_t len = strlen(data) / 2;
    uint8_t *adv_data = (uint8_t *)calloc(len, sizeof(uint8_t));
    str2array(adv_data, data, len);

    sl_status_t status = SL_STATUS_FAIL;
    if (len <= 254) 
    {
        status = sl_bt_extended_advertiser_set_data(handle, len, adv_data);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }
    else 
    {
        if (len == (SL_BGAPI_MAX_PAYLOAD_SIZE - 1))
        {
            status = sl_bt_system_data_buffer_write(len, adv_data);
            if (status != SL_STATUS_OK)
            {
                return GL_UNKNOW_ERR;
            }
        }
        else
        {
            status = ble_write_long_data(len, adv_data);
            if (status != SL_STATUS_OK)
            {
                return GL_UNKNOW_ERR;
            }
        }

        status = sl_bt_extended_advertiser_set_long_data(handle);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_set_periodic_adv_data(uint8_t handle, const char *data)
{
    if ((!data) || (strlen(data) % 2) || (strlen(data) / 2) > MAX_ADV_DATA_LEN)
    {
        return GL_ERR_PARAM;
    }

    uint16_t len = strlen(data) / 2;
    uint8_t *adv_data = (uint8_t *)calloc(len, sizeof(uint8_t));
    str2array(adv_data, data, len);

    sl_status_t status = SL_STATUS_FAIL;
    if (len <= 254) 
    {
        status = sl_bt_periodic_advertiser_set_data(handle, len, adv_data);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }
    else 
    {
        if (len == (SL_BGAPI_MAX_PAYLOAD_SIZE - 1))
        {
            status = sl_bt_system_data_buffer_write(len, adv_data);
            if (status != SL_STATUS_OK)
            {
                return GL_UNKNOW_ERR;
            }
        }
        else
        {
            status = ble_write_long_data(len, adv_data);
            if (status != SL_STATUS_OK)
            {
                return GL_UNKNOW_ERR;
            }
        }

        status = sl_bt_periodic_advertiser_set_long_data(handle);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_adv(uint8_t handle)
{
    sl_status_t status = SL_STATUS_FAIL;

    if (handle != 0xff)
    {
        status = sl_bt_advertiser_stop(handle);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }

        status = sl_bt_periodic_advertiser_stop(handle);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }

        // status = sl_bt_advertiser_delete_set(handle);
        // if (status != SL_STATUS_OK)
        // {
        //     return GL_UNKNOW_ERR;
        // }
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_send_notify(BLE_MAC address, int char_handle, char *value)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    if ((!value) || (strlen(value) % 2))
    {
        return GL_ERR_PARAM;
    }

    size_t len = strlen(value) / 2;
    uint8_t *hex_value = (uint8_t *)calloc(len + 1, sizeof(uint8_t));
    str2array(hex_value, value, len);

    sl_status_t status = SL_STATUS_FAIL;
    uint16_t send_len = 0;

    status = sl_bt_gatt_write_characteristic_value_without_response((uint8_t)connection, (uint16_t)char_handle, (size_t)len, (uint8_t *)hex_value, (uint16_t *)&send_len);
    if (status != SL_STATUS_OK)
    {
        free(hex_value);
        return GL_UNKNOW_ERR;
    }
    if (send_len == 0)
    {
        free(hex_value);
        return GL_UNKNOW_ERR;
    }

    free(hex_value);
    return GL_SUCCESS;
}

GL_RET silabs_ble_connect(BLE_MAC address, int address_type, int phy)
{
    if (!address)
    {
        return GL_ERR_PARAM;
    }

    uint8_t connection = 0;

    bd_addr addr;
    memcpy(addr.addr, address, 6);
    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_connection_open(addr, (uint8_t)address_type, (uint8_t)phy, (uint8_t *)&connection);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    ble_dev_mgr_add(address_str, (uint16_t)connection);

    return GL_SUCCESS;
}

GL_RET silabs_ble_disconnect(BLE_MAC address)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_connection_close((uint8_t)connection);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_get_rssi(BLE_MAC address, int32_t *rssi)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_connection_get_rssi((uint8_t)connection);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    uint32_t evt_id = sl_bt_evt_connection_rssi_id;
    if (wait_rsp_evt(evt_id, 300) == 0)
    {
        *rssi = evt->data.evt_connection_rssi.rssi;
    }
    else
    {
        return GL_ERR_EVENT_MISSING;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_get_service(gl_ble_service_list_t *service_list, BLE_MAC address)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_gatt_discover_primary_services((uint8_t)connection);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    int i = 0;
    uint32_t evt_id = sl_bt_evt_gatt_procedure_completed_id;
    if (wait_rsp_evt(evt_id, 600) == 0)
    {
        service_list->list_len = special_evt_num;
        while (i < special_evt_num)
        {
            struct sl_bt_packet *e = &special_evt[i];
            if (SL_BT_MSG_ID(e->header) == sl_bt_evt_gatt_service_id && e->data.evt_gatt_service.connection == connection)
            {
                service_list->list[i].handle = e->data.evt_gatt_service.service;
                reverse_endian(e->data.evt_gatt_service.uuid.data, e->data.evt_gatt_service.uuid.len);
                hex2str(e->data.evt_gatt_service.uuid.data, e->data.evt_gatt_service.uuid.len, service_list->list[i].uuid);
            }
            i++;
        }
    }
    else
    {
        special_evt_num = 0;
        // evt->header = 0;
        return GL_ERR_EVENT_MISSING;
    }

    // clean evt count
    special_evt_num = 0;

    if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
    {
        evt->header = 0;
        return GL_SUCCESS;
    }
    else
    {
        return GL_ERR_EVENT_MISSING;
    }
}

GL_RET silabs_ble_get_char(gl_ble_char_list_t *char_list, BLE_MAC address, int service_handle)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_gatt_discover_characteristics((uint8_t)connection, (uint32_t)service_handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    int i = 0;
    uint32_t evt_id = sl_bt_evt_gatt_procedure_completed_id;
    if (wait_rsp_evt(evt_id, 600) == 0)
    {
        char_list->list_len = special_evt_num;
        while (i < special_evt_num)
        {
            struct sl_bt_packet *e = &special_evt[i];
            if (SL_BT_MSG_ID(e->header) == sl_bt_evt_gatt_characteristic_id && e->data.evt_gatt_characteristic.connection == connection)
            {
                char_list->list[i].handle = e->data.evt_gatt_characteristic.characteristic;
                char_list->list[i].properties = e->data.evt_gatt_characteristic.properties;
                reverse_endian(e->data.evt_gatt_characteristic.uuid.data, e->data.evt_gatt_characteristic.uuid.len);
                hex2str(e->data.evt_gatt_characteristic.uuid.data, e->data.evt_gatt_characteristic.uuid.len, char_list->list[i].uuid);
            }
            i++;
        }
    }
    else
    {
        special_evt_num = 0;
        // evt->header = 0;
        return GL_ERR_EVENT_MISSING;
    }

    // clean evt count
    special_evt_num = 0;

    if (SL_BT_MSG_ID(evt->header) == sl_bt_evt_gatt_procedure_completed_id)
    {
        evt->header = 0;
        return GL_SUCCESS;
    }
    else
    {
        return GL_ERR_EVENT_MISSING;
    }
}

GL_RET silabs_ble_set_power(int power, int *current_power)
{
    sl_status_t status = SL_STATUS_FAIL;
    int16_t set_min = 0, set_max = 0;

    status = sl_bt_system_set_tx_power(0, (int16_t)power, (int16_t *)&set_min, (int16_t *)&set_max);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }
    
    *current_power = set_max;
    return GL_SUCCESS;
}

GL_RET silabs_ble_read_char(BLE_MAC address, int char_handle)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_gatt_read_characteristic_value((uint8_t)connection, (uint16_t)char_handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_write_char(BLE_MAC address, int char_handle, char *value, int res)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    if ((!value) || (strlen(value) % 2))
    {
        return GL_ERR_PARAM;
    }
    size_t len = strlen(value) / 2;
    unsigned char data[256];
    str2array(data, value, len);

    sl_status_t status = SL_STATUS_FAIL;

    if (res)
    {
        status = sl_bt_gatt_write_characteristic_value((uint8_t)connection, (uint16_t)char_handle, (size_t)len, (uint8_t *)data);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }
    else
    {
        uint16_t sent_len = 0;
        status = sl_bt_gatt_write_characteristic_value_without_response((uint8_t)connection, (uint16_t)char_handle, (size_t)len, (uint8_t *)data, (uint16_t *)&sent_len);
        if (status != SL_STATUS_OK)
        {
            return GL_UNKNOW_ERR;
        }
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_set_notify(BLE_MAC address, int char_handle, int flag)
{
    int connection = 0;
    char address_str[BLE_MAC_LEN] = {0};
    addr2str(address, address_str);
    GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
    if (ret != GL_SUCCESS)
    {
        return GL_ERR_PARAM;
    }

    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_gatt_set_characteristic_notification((uint8_t)connection, (uint16_t)char_handle, (uint8_t)flag);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_set_gattdb(char *uci_cfg_name)
{
    GL_RET ret = silabs_ble_del_gattdb();
    if(ret != GL_SUCCESS)
    {
        printf("delete local gatt data base failed!\n");
        return GL_UNKNOW_ERR;
    }

    uint16_t session_handle = 0xffff;
    uint16_t service_handle = 0xffff;
    uint16_t characteristic_handle = 0xffff;
    uint16_t descriptor_handle = 0xffff;

    static char tmp[64];
    const char s[2] = " ";
    int i, j, k;
    char *buf;
    char *end_str;
    char str_section[64];

    sl_bt_uuid_16_t uuid_2B;
    uuid_128 uuid_16B;

    uint8_t service_num = 0;
    gl_ble_gattdb_service_param_t *service_param = NULL;
    char *service_name = NULL;
    uint8_t *uuid_service = NULL;

    uint8_t char_num;
    gl_ble_gattdb_char_param_t *char_param = NULL;
    char *char_name = NULL;
    uint8_t *char_value = NULL;
    
    uint8_t descriptor_num;
    gl_ble_gattdb_descriptor_param_t *descriptor_param = NULL;
    char *descriptor_name = NULL;
    uint8_t *descriptor_value = NULL;

    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_gattdb_new_session(&session_handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }
    
    static struct uci_context* ctx = NULL;
    ctx = guci_init();
    if(ctx == NULL)
    {
        printf("uci init fail.\n");
        status = SL_STATUS_FAIL;
        goto exit;
    }

    memset(tmp, 0, sizeof(tmp));
    memset(str_section, 0, sizeof(str_section));
    sprintf(str_section, "%s.gattdb.service_num", uci_cfg_name);
    if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
    {
        status = SL_STATUS_FAIL;
        goto exit;
    }
    service_num = atoi(tmp);
    
    if(service_num == 0)
    {
        printf("local gatt dataBase no service\n");
        status = SL_STATUS_FAIL;
        goto exit;
    }

    service_param = (gl_ble_gattdb_service_param_t*)calloc(service_num, sizeof(gl_ble_gattdb_service_param_t));

    memset(tmp, 0, sizeof(tmp));
    memset(str_section, 0, sizeof(str_section));
    sprintf(str_section, "%s.gattdb.service", uci_cfg_name);
    if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
    {
        status = SL_STATUS_FAIL;
        goto exit;
    }

    buf = tmp;
    for(i = 0; i < service_num; ++i)
    {
        service_name = strtok_r(buf, s, &end_str);
        if(service_name != NULL)
        {
            memcpy(service_param[i].service_name, service_name, 64);
        }else{
            printf("Split service name error\n");
            status = SL_STATUS_FAIL;
            goto exit;
        }
        buf = NULL;
    }
    service_name = NULL;
    
    for(i = 0; i < service_num; ++i)
    {
        memset(str_section, 0, sizeof(str_section));
        sprintf(str_section, "%s.%s.property", uci_cfg_name, service_param[i].service_name);
        memset(tmp, 0, sizeof(tmp));
        if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
        {
            status = SL_STATUS_FAIL;
            goto exit;
        }
        service_param[i].property = atoi(tmp);

        memset(str_section, 0, sizeof(str_section));
        sprintf(str_section, "%s.%s.uuid_len", uci_cfg_name, service_param[i].service_name);
        memset(tmp, 0, sizeof(tmp));
        if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
        {
            status = SL_STATUS_FAIL;
            goto exit;
        }
        service_param[i].uuid_len = atoi(tmp);
        

        memset(str_section, 0, sizeof(str_section));
        sprintf(str_section, "%s.%s.uuid", uci_cfg_name, service_param[i].service_name);
        memset(tmp, 0, sizeof(tmp));
        if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
        {
            status = SL_STATUS_FAIL;
            goto exit;
        }
        uuid_service = calloc(1, service_param[i].uuid_len);
        str2array(uuid_service, tmp, service_param[i].uuid_len);
        if(ENDIAN)
        {
            reverse_endian(uuid_service, service_param[i].uuid_len);
        }
        service_param[i].uuid = uuid_service;

        status = sl_bt_gattdb_add_service(session_handle, sl_bt_gattdb_primary_service, 
                                          service_param[i].property, service_param[i].uuid_len, service_param[i].uuid, &service_handle);
        if (status != SL_STATUS_OK)
        {
            goto exit;
        }
        free(uuid_service);
        uuid_service = NULL;

        memset(str_section, 0, sizeof(str_section));
        sprintf(str_section, "%s.%s.characteristic_num", uci_cfg_name, service_param[i].service_name);
        memset(tmp, 0, sizeof(tmp));
        if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
        {
            status = SL_STATUS_FAIL;
            goto exit;
        }
        char_num = atoi(tmp);

        if(char_num == 0)
        {
            continue;
        }

        char_param = (gl_ble_gattdb_char_param_t*)calloc(char_num, sizeof(gl_ble_gattdb_char_param_t));
        
        memset(str_section, 0, sizeof(str_section));
        sprintf(str_section, "%s.%s.characteristic", uci_cfg_name, service_param[i].service_name);
        memset(tmp, 0, sizeof(tmp));
        if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
        {
            status = SL_STATUS_FAIL;
            goto exit;
        }
        buf = tmp;
        for(j = 0; j < char_num; ++j)
        {
            char_name = strtok_r(buf, s, &end_str);
            if(char_name != NULL)
            {
                memcpy(char_param[j].char_name, char_name, 64);
            }else{
                printf("Split characteristic name error\n");
                status = SL_STATUS_FAIL;
                goto exit;
            }
            buf = NULL;
        }
        char_name = NULL;

        for(j = 0; j < char_num; ++j)
        {
            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.property", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].property = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.flag", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].flag = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.uuid_len", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].uuid_len = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.uuid", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            if(char_param[j].uuid_len == 2)
            {
                str2array(char_param[j].uuid_16, tmp, char_param[j].uuid_len);
                if(ENDIAN)
                {
                    reverse_endian(char_param[j].uuid_16, char_param[j].uuid_len);
                }
            }
            else if(char_param[j].uuid_len == 16)
            {
                str2array(char_param[j].uuid_128, tmp, char_param[j].uuid_len);
                if(ENDIAN)
                {
                    reverse_endian(char_param[j].uuid_128, char_param[j].uuid_len);
                }
            }

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.value_type", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].value_type = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.maxlen", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].maxlen = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.value_len", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_param[j].value_len = atoi(tmp);

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.value", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            char_value = calloc(1, char_param[j].value_len);
            str2array(char_value, tmp, char_param[j].value_len);
            char_param[j].value = char_value;

            if(char_param[j].uuid_len == 2)
            {
                memcpy(uuid_2B.data, char_param[j].uuid_16, 2);
                status = sl_bt_gattdb_add_uuid16_characteristic(session_handle, service_handle, char_param[j].property, 0, char_param[j].flag, uuid_2B, 
                                                char_param[j].value_type, char_param[j].maxlen, char_param[j].value_len, char_param[j].value, &characteristic_handle);
            }
            else if(char_param[j].uuid_len == 16)
            {
                memcpy(uuid_16B.data, char_param[j].uuid_128, 16);
                status = sl_bt_gattdb_add_uuid128_characteristic(session_handle, service_handle, char_param[j].property, 0, char_param[j].flag, uuid_16B, 
                                                char_param[j].value_type, char_param[j].maxlen, char_param[j].value_len, char_param[j].value, &characteristic_handle);
            }
            if (status != SL_STATUS_OK)
            {
                goto exit;
            }
            free(char_value);
            char_value = NULL;

            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.descriptor_num", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            descriptor_num = atoi(tmp);
            if(descriptor_num == 0)
            {
                continue;
            }

            descriptor_param = (gl_ble_gattdb_descriptor_param_t*)calloc(descriptor_num, sizeof(gl_ble_gattdb_descriptor_param_t));
            memset(str_section, 0, sizeof(str_section));
            sprintf(str_section, "%s.%s.descriptor", uci_cfg_name, char_param[j].char_name);
            memset(tmp, 0, sizeof(tmp));
            if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
            {
                status = SL_STATUS_FAIL;
                goto exit;
            }
            buf = tmp;
            for(k = 0; k < descriptor_num; ++k)
            {
                descriptor_name = strtok_r(buf, s, &end_str);
                if(descriptor_name != NULL)
                {
                    memcpy(descriptor_param[k].descriptor_name, descriptor_name, 64);
                    // descriptor_param[k].descriptor_name = descriptor_name;
                }else{
                    printf("Split group name error\n");
                    goto exit;
                }
                buf = NULL;
            }
            descriptor_name = NULL;

            for(k = 0; k < descriptor_num; ++k)
            {
                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.property", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_param[k].property = atoi(tmp);

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.uuid_len", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_param[k].uuid_len = atoi(tmp);

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.uuid", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                if(descriptor_param[k].uuid_len == 2)
                {
                    str2array(descriptor_param[k].uuid_16, tmp, descriptor_param[k].uuid_len);
                    if(ENDIAN)
                    {
                        reverse_endian(descriptor_param[k].uuid_16, descriptor_param[k].uuid_len);
                    }
                }
                else if(descriptor_param[k].uuid_len == 16)
                {
                    str2array(descriptor_param[k].uuid_128, tmp, descriptor_param[k].uuid_len);
                    if(ENDIAN)
                    {
                        reverse_endian(descriptor_param[k].uuid_128, descriptor_param[k].uuid_len);
                    }
                }

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.value_type", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_param[k].value_type = atoi(tmp);

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.maxlen", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_param[k].maxlen = atoi(tmp);

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.value_len", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_param[k].value_len = atoi(tmp);

                memset(str_section, 0, sizeof(str_section));
                sprintf(str_section, "%s.%s.value", uci_cfg_name, descriptor_param[k].descriptor_name);
                memset(tmp, 0, sizeof(tmp));
                if(GL_SUCCESS != guci_get(ctx, str_section, tmp))
                {
                    status = SL_STATUS_FAIL;
                    goto exit;
                }
                descriptor_value = calloc(1, descriptor_param[k].value_len);
                str2array(descriptor_value, tmp, descriptor_param[k].value_len);
                descriptor_param[k].value = descriptor_value;

                sl_bt_gattdb_commit(session_handle);
                sl_bt_gattdb_new_session(&session_handle);
                if(descriptor_param[k].uuid_len == 2)
                {
                    memcpy(uuid_2B.data, descriptor_param[k].uuid_16, 2);
                    status = sl_bt_gattdb_add_uuid16_descriptor(session_handle, characteristic_handle, descriptor_param[k].property, 0, uuid_2B, descriptor_param[k].value_type,
                                                descriptor_param[k].maxlen, descriptor_param[k].value_len, descriptor_param[k].value, &descriptor_handle);
                }
                else if(descriptor_param[k].uuid_len == 16)
                {
                    memcpy(uuid_16B.data, descriptor_param[k].uuid_128, 16);
                    status = sl_bt_gattdb_add_uuid128_descriptor(session_handle, characteristic_handle, descriptor_param[k].property, 0, uuid_16B, descriptor_param[k].value_type,
                                                descriptor_param[k].value_len, descriptor_param[k].value_len, descriptor_param[k].value, &descriptor_handle);
                }
                if (status != SL_STATUS_OK)
                {
                    goto exit;
                }
                free(descriptor_value);
                descriptor_value = NULL;
            }
        }
        status = sl_bt_gattdb_start_service(session_handle, service_handle);
        if (status != SL_STATUS_OK)
        {
            goto exit;
        }
    }
    status = sl_bt_gattdb_commit(session_handle);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }

exit:

    if(service_param != NULL)
    {
        free(service_param);
    }
    if(uuid_service != NULL)
    {
        free(uuid_service);
    }
    if(char_param != NULL)
    {
        free(char_param);
    }
    if(char_value != NULL)
    {
        free(char_value);
    }
    if(descriptor_param != NULL)
    {
        free(descriptor_param);
    }
    if(descriptor_value != NULL)
    {
        free(descriptor_value);
    }

    if(status == GL_SUCCESS)
    {
        return GL_SUCCESS;
    }
    
    sl_bt_gattdb_abort(session_handle);
    return GL_UNKNOW_ERR;
}

GL_RET silabs_ble_del_gattdb(void)
{
    uint16_t session_handle = 0xffff;
    uint16_t att_handle = 0xffff;
    uint16_t service_handle[16];

    uint16_t start = 1;
    uint8_t  count = 0;
    uint8_t  primary_type[2] = {0x00, 0x28};

    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_gattdb_new_session(&session_handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    int i = 0;
    while(1)
    {
        status = sl_bt_gatt_server_find_attribute(start, 2, primary_type, &att_handle);
        if (status == SL_STATUS_BT_ATT_ATT_NOT_FOUND)
        {
            ++count;
            if(count == 2)
            {
                break;              
            }
        }
        else if (status == SL_STATUS_OK)
        {
            size_t value_len;
            uint8_t value[2];
            status = sl_bt_gatt_server_read_attribute_value(att_handle, 0, 2, &value_len, value);
            // if(status == SL_STATUS_OK && value_len == 2)
            // {
            //     if(value[0] == 0x01 && value[1] == 0x18) //remain Generic Attribute Service
            //     {
            //         start = att_handle + 1;
            //         continue;
            //     }
            // }
            service_handle[i++] = att_handle;
            start = att_handle + 1;
            continue;
        }
        else
        {
            sl_bt_gattdb_abort(session_handle);
            return GL_UNKNOW_ERR;
        }   
    }

    for(; i > 0;)
    {
        status = sl_bt_gattdb_remove_service(session_handle, service_handle[--i]);
        if (status != SL_STATUS_OK)
        {
            sl_bt_gattdb_abort(session_handle);
            return GL_UNKNOW_ERR;
        }
    }
    
    status = sl_bt_gattdb_commit(session_handle);
    if (status != SL_STATUS_OK)
    {
        sl_bt_gattdb_abort(session_handle);
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}