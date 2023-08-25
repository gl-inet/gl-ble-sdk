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

GL_RET silabs_ble_set_sync_parameters(uint16_t skip, uint16_t timeout)
{
    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_sync_set_parameters(skip, timeout, 0);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }
    return GL_SUCCESS;
}

GL_RET silabs_ble_start_sync(BLE_MAC address, uint8_t address_type, uint8_t adv_sid, uint16_t *handle)
{
    bd_addr addr;
    memcpy(addr.addr, address, 6);
    sl_status_t status = SL_STATUS_FAIL;

    status = sl_bt_sync_open(addr, address_type, adv_sid, handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_sync(uint16_t handle)
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
    status = sl_bt_extended_advertiser_start(handle, connect, SL_BT_EXTENDED_ADVERTISER_INCLUDE_TX_POWER);
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

    status = sl_bt_periodic_advertiser_start(handle, interval_min, interval_max, SL_BT_PERIODIC_ADVERTISER_AUTO_START_EXTENDED_ADVERTISING|SL_BT_EXTENDED_ADVERTISER_INCLUDE_TX_POWER);
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

GL_RET silabs_ble_set_gattdb(char *json_cfg_name)
{
    GL_RET ret = silabs_ble_del_gattdb();
    if(ret != GL_SUCCESS)
    {
        // printf("delete local gatt data base failed!\n");
        return GL_UNKNOW_ERR;
    }

    sl_bt_uuid_16_t uuid_2B;
    uuid_128 uuid_16B;

    uint16_t session_handle = 0xffff;
    uint16_t service_handle = 0xffff;
    uint16_t characteristic_handle = 0xffff;
    uint16_t descriptor_handle = 0xffff;
    json_object *root;
    json_object *service_list_obj;
    json_object *service_obj;

    json_object *property_obj;
    uint8_t  service_property;
    uint16_t char_property;
    uint16_t despt_property;

    json_object *uuid_len_obj;
    size_t uuid_len;

    json_object *uuid_obj;
    const char *uuid;
    uint8_t *uuid_u8 = NULL;

    
    json_object *char_list_obj;
    json_object *char_obj;

    json_object *flag_obj;
    uint8_t flag;

    json_object *value_type_obj;
    uint8_t value_type;

    json_object *maxlen_obj;
    uint16_t maxlen;

    json_object *value_len_obj;
    size_t value_len;

    json_object *value_obj;
    const char *value;
    uint8_t *value_u8 = NULL;


    json_object *despt_list_obj;
    json_object *despt_obj;

    json_bool json_ret;


    sl_status_t status = SL_STATUS_FAIL;
    status = sl_bt_gattdb_new_session(&session_handle);
    if (status != SL_STATUS_OK)
    {
        return GL_UNKNOW_ERR;
    }

    root = json_object_from_file(json_cfg_name);
    if(root == NULL)
    {
        sl_bt_gattdb_abort(session_handle);
        return GL_UNKNOW_ERR;
    }
    else
    {
        json_ret = json_object_object_get_ex(root, "service", &service_list_obj);
        if(json_ret == 0)
        {
            // printf("service_list_obj:%s\n",json_object_to_json_string(service_list_obj));   
            
            goto exit; 
        }

        size_t service_num = json_object_array_length(service_list_obj);
        // printf("service_num: %d\n", service_num);
        for(uint8_t i = 0; i < service_num; ++i)
        {
            service_obj = json_object_array_get_idx(service_list_obj, i); 

            json_ret = json_object_object_get_ex(service_obj, "service_property", &property_obj);
            if(json_ret == 0)
            {
                goto exit; 
            }
            service_property = json_object_get_int(property_obj);

            json_ret = json_object_object_get_ex(service_obj, "service_uuid_len", &uuid_len_obj);
            if(json_ret == 0)
            {
                goto exit; 
            }
            uuid_len = json_object_get_int(uuid_len_obj);

            json_ret = json_object_object_get_ex(service_obj, "service_uuid", &uuid_obj);
            if(json_ret == 0)
            {
                goto exit; 
            }
            uuid = json_object_get_string(uuid_obj);
            uuid_u8 = calloc(1, uuid_len);
            str2array(uuid_u8, uuid, uuid_len);
            if(ENDIAN)
            {
                reverse_endian(uuid_u8, uuid_len);
            }

            status = sl_bt_gattdb_add_service(session_handle, sl_bt_gattdb_primary_service, 
                                          service_property, uuid_len, uuid_u8, &service_handle);
            if (status != SL_STATUS_OK)
            {
                goto exit;
            }
            free(uuid_u8);
            uuid_u8 = NULL;



            json_ret = json_object_object_get_ex(service_obj, "characteristic", &char_list_obj);
            if(json_ret == 0)
            {
                goto exit; 
            }

            size_t char_num = json_object_array_length(char_list_obj);
            // printf("char_num: %d\n", char_num);
            for(uint8_t j = 0; j < char_num; ++j)
            {
                char_obj = json_object_array_get_idx(char_list_obj, j); 

                json_ret = json_object_object_get_ex(char_obj, "char_property", &property_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                char_property = json_object_get_int(property_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_flag", &flag_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                flag = json_object_get_int(flag_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_uuid_len", &uuid_len_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                uuid_len = json_object_get_int(uuid_len_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_uuid", &uuid_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                uuid = json_object_get_string(uuid_obj);
                uuid_u8 = calloc(1, uuid_len);
                str2array(uuid_u8, uuid, uuid_len);
                if(ENDIAN)
                {
                    reverse_endian(uuid_u8, uuid_len);
                }

                json_ret = json_object_object_get_ex(char_obj, "char_value_type", &value_type_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                value_type = json_object_get_int(value_type_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_maxlen", &maxlen_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                maxlen = json_object_get_int(maxlen_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_value_len", &value_len_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                value_len = json_object_get_int(value_len_obj);

                json_ret = json_object_object_get_ex(char_obj, "char_value", &value_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                value = json_object_get_string(value_obj);
                value_u8 = calloc(1, value_len);
                str2array(value_u8, value, value_len);

                if(uuid_len == 2)
                {
                    memcpy(uuid_2B.data, uuid_u8, 2);
                    status = sl_bt_gattdb_add_uuid16_characteristic(session_handle, service_handle, char_property, 0, flag, uuid_2B, 
                                                    value_type, maxlen, value_len, value_u8, &characteristic_handle);
                }
                else if(uuid_len == 16)
                {
                    memcpy(uuid_16B.data, uuid_u8, 16);
                    status = sl_bt_gattdb_add_uuid128_characteristic(session_handle, service_handle, char_property, 0, flag, uuid_16B, 
                                                    value_type, maxlen, value_len, value_u8, &characteristic_handle);
                }
                if (status != SL_STATUS_OK)
                {
                    goto exit;
                }
                free(value_u8);
                value_u8 = NULL;
                free(uuid_u8);
                uuid_u8 = NULL;


                json_ret = json_object_object_get_ex(char_obj, "descriptor", &despt_list_obj);
                if(json_ret == 0)
                {
                    goto exit; 
                }
                
                size_t despt_num = json_object_array_length(despt_list_obj);
                // printf("despt_num: %d\n", despt_num);
                for(uint8_t k = 0; k < despt_num; ++k)
                {
                    despt_obj = json_object_array_get_idx(despt_list_obj, k); 

                    json_ret = json_object_object_get_ex(despt_obj, "despt_property", &property_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    despt_property = json_object_get_int(property_obj);

                    json_ret = json_object_object_get_ex(despt_obj, "despt_uuid_len", &uuid_len_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    uuid_len = json_object_get_int(uuid_len_obj);

                    json_ret = json_object_object_get_ex(despt_obj, "despt_uuid", &uuid_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    uuid = json_object_get_string(uuid_obj);
                    uuid_u8 = calloc(1, uuid_len);
                    str2array(uuid_u8, uuid, uuid_len);
                    if(ENDIAN)
                    {
                        reverse_endian(uuid_u8, uuid_len);
                    }

                    json_ret = json_object_object_get_ex(despt_obj, "despt_value_type", &value_type_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    value_type = json_object_get_int(value_type_obj);

                    json_ret = json_object_object_get_ex(despt_obj, "despt_maxlen", &maxlen_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    maxlen = json_object_get_int(maxlen_obj);

                    json_ret = json_object_object_get_ex(despt_obj, "despt_value_len", &value_len_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    value_len = json_object_get_int(value_len_obj);

                    json_ret = json_object_object_get_ex(despt_obj, "despt_value", &value_obj);
                    if(json_ret == 0)
                    {
                        goto exit; 
                    }
                    value = json_object_get_string(value_obj);
                    value_u8 = calloc(1, value_len);
                    str2array(value_u8, value, value_len);
                    if(uuid_len == 2)
                    {
                        memcpy(uuid_2B.data, uuid_u8, 2);
                        status = sl_bt_gattdb_add_uuid16_descriptor(session_handle, characteristic_handle, despt_property, 0, uuid_2B, value_type,
                                                    maxlen, value_len, value_u8, &descriptor_handle);
                    }
                    else if(uuid_len == 16)
                    {
                        memcpy(uuid_16B.data, uuid_u8, 16);
                        status = sl_bt_gattdb_add_uuid128_descriptor(session_handle, characteristic_handle, despt_property, 0, uuid_16B, value_type,
                                                    maxlen, value_len, value_u8, &descriptor_handle);
                    }
                    if (status != SL_STATUS_OK)
                    {
                        goto exit;
                    }
                    free(value_u8);
                    value_u8 = NULL;
                    free(uuid_u8);
                    uuid_u8 = NULL;

                }

            }
            status = sl_bt_gattdb_start_service(session_handle, service_handle);
            if (status != SL_STATUS_OK)
            {
                goto exit;
            }
        }
    }
    status = sl_bt_gattdb_commit(session_handle);
    if (status != SL_STATUS_OK)
    {
        goto exit;
    }
    json_object_put(root);
    return GL_SUCCESS;

exit:
    sl_bt_gattdb_abort(session_handle);
    json_object_put(root);
    if(value_u8 != NULL)
    {
        free(value_u8);
    }
    if(uuid_u8 != NULL)
    {
        free(uuid_u8);
    }
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