/*****************************************************************************
 * @file 
 * @brief Bluetooth driver for silabs EFR32
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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "host_gecko.h"
#include "gl_uart.h"
#include "gl_hal.h"
#include "bg_types.h"
#include "gl_errno.h"
#include "gl_type.h"
#include "silabs_bleapi.h"
#include "gl_common.h"
#include "silabs_msg.h"
#include "gl_dev_mgr.h"

extern struct gecko_cmd_packet* evt;
extern bool wait_reset_flag;
extern bool appBooted; 

GL_RET silabs_ble_enable(int enable)
{
    if(enable)
    {
        system(rston);
    }
    else{
        // wait sub thread recv end
        wait_reset_flag = true;
        // usleep(100*1000);
        while(wait_reset_flag)
        {
            usleep(10*1000);
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
    while(reset_time < 3)
    {
        wait_s = 30; // 3s = 30 * 100ms
        wait_off = 300; // 3s = 300 * 10ms

        wait_reset_flag = true;
        // wait for turn off ble module
        while((wait_reset_flag) && (wait_off > 0))
        {
            wait_off--;
            usleep(10*1000);
        }

        // check turn off end
        if((appBooted) || (wait_off <= 0))
        {
            // error
            reset_time++;
            continue;
        }
        
        //wait 300 ms
        usleep(300*1000);

        // turn on ble module
        system(rston);
        
        // wait for ble module start
        while((!appBooted) && (wait_s > 0))
        {
            wait_s--;
            usleep(100*1000);
        }

        // if ble module start success, break loop
        if(appBooted)
        {
            break;
        }

        // ble module start timeout
        reset_time++;
    }

    if(reset_time < 3)
    {
        return GL_SUCCESS;
    }else{
        return GL_UNKNOW_ERR;
    }
}

GL_RET silabs_ble_local_mac(BLE_MAC mac)
{

    struct gecko_msg_system_get_bt_address_rsp_t* rsp = gecko_cmd_system_get_bt_address();
    if(rsp == NULL)
    {
        return GL_UNKNOW_ERR;
    }

    memcpy(mac, rsp->address.addr, 6);

    return GL_SUCCESS;
}

GL_RET silabs_ble_discovery(int phys, int interval, int window, int type, int mode)
{
    struct gecko_msg_le_gap_set_discovery_timing_rsp_t* set_tim_rsp = gecko_cmd_le_gap_set_discovery_timing(phys,interval,window);
    if(!set_tim_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(set_tim_rsp->result)
    {
        // printf("gecko_cmd_le_gap_set_discovery_timing: %d\n", set_tim_rsp->result);
        return (set_tim_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    struct gecko_msg_le_gap_set_discovery_type_rsp_t* set_type_rsp = gecko_cmd_le_gap_set_discovery_type(phys,type);
    if(!set_type_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(set_type_rsp->result)
    {
        // printf("gecko_cmd_le_gap_set_discovery_type: %d\n", set_type_rsp->result);
        return (set_type_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    struct gecko_msg_le_gap_start_discovery_rsp_t* start_rsp = gecko_cmd_le_gap_start_discovery(phys,mode);
    if(!start_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(start_rsp->result)
    {
        // printf("gecko_cmd_le_gap_start_discovery: %d\n", start_rsp->result);
        return (start_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_discovery(void)
{
    struct gecko_msg_le_gap_end_procedure_rsp_t* rsp = gecko_cmd_le_gap_end_procedure();
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
	if(rsp->result)
	{
        return (rsp->result + MANUFACTURER_ERR_BASE);
	}

    return GL_SUCCESS;
}

GL_RET silabs_ble_adv(int phys, int interval_min, int interval_max, int discover, int adv_conn)
{
    struct gecko_msg_le_gap_set_advertise_phy_rsp_t* set_phy_rsp = gecko_cmd_le_gap_set_advertise_phy(0, phys, phys);
    if(!set_phy_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(set_phy_rsp->result)
    {
        return (set_phy_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    struct gecko_msg_le_gap_set_advertise_timing_rsp_t* set_tim_rsp = gecko_cmd_le_gap_set_advertise_timing(0, interval_min, interval_max, 0, 0);
    if(!set_tim_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(set_tim_rsp->result)
    {
        return (set_tim_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    struct gecko_msg_le_gap_start_advertising_rsp_t* start_rsp = gecko_cmd_le_gap_start_advertising(0, discover, adv_conn);
    if(!start_rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(start_rsp->result)
    {
        return (start_rsp->result + MANUFACTURER_ERR_BASE);       
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_adv_data(int flag, char *data)
{
    if((!data) || (strlen(data)%2))
    {
        return GL_ERR_PARAM;
    }

    int len = strlen(data)/2;
    uint8* adv_data = (uint8*)calloc(len,sizeof(uint8));
    str2array(adv_data,data,len);

    struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t* rsp = gecko_cmd_le_gap_bt5_set_adv_data(0, flag, len, adv_data);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_stop_adv(void)
{
    struct gecko_msg_le_gap_stop_advertising_rsp_t* rsp = gecko_cmd_le_gap_stop_advertising(0);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

    return GL_SUCCESS;
}

GL_RET silabs_ble_send_notify(BLE_MAC address, int char_handle, char *value)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    if((!value) || (strlen(value)%2))
    {
        return GL_ERR_PARAM;
    }

    int len = strlen(value)/2;
    uint8* hex_value = (uint8*)calloc(len,sizeof(uint8));
    str2array(hex_value,value,len);

    struct gecko_msg_gatt_server_send_characteristic_notification_rsp_t* rsp = gecko_cmd_gatt_server_send_characteristic_notification(connection, char_handle, len, hex_value);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

    return GL_SUCCESS;
}


GL_RET silabs_ble_connect(BLE_MAC address, int address_type, int phy)
{
    if(!address)
    {
        return GL_ERR_PARAM;
    }

    bd_addr addr;
    memcpy(addr.addr, address, 6);
    struct gecko_msg_le_gap_connect_rsp_t* rsp = gecko_cmd_le_gap_connect(addr, address_type, phy);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
    ble_dev_mgr_add(address_str, rsp->connection);
    
    return GL_SUCCESS;
}

GL_RET silabs_ble_disconnect(BLE_MAC address)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_le_connection_close_rsp_t* rsp = gecko_cmd_le_connection_close(connection);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
	if(rsp->result)
	{
        return (rsp->result + MANUFACTURER_ERR_BASE);
	}

    return GL_SUCCESS;
}

GL_RET silabs_ble_get_rssi(BLE_MAC address, int32_t *rssi)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_le_connection_get_rssi_rsp_t* rsp = gecko_cmd_le_connection_get_rssi(connection);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

	uint32_t evt_id = gecko_evt_le_connection_rssi_id;
	if(wait_rsp_evt(evt_id, 300) == 0) {
        *rssi = evt->data.evt_le_connection_rssi.rssi;
	}else {
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
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_gatt_discover_primary_services_rsp_t* rsp = gecko_cmd_gatt_discover_primary_services(connection);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);          
    }

	int i = 0;
	uint32_t evt_id = gecko_evt_gatt_procedure_completed_id;
	if(wait_rsp_evt(evt_id, 600) == 0)
	{
        service_list->list_len = special_evt_num;
		while(i < special_evt_num)
		{
			struct gecko_cmd_packet* e = &special_evt[i];
			if(BGLIB_MSG_ID(e->header) == gecko_evt_gatt_service_id && e->data.evt_gatt_service.connection == connection)
			{
                service_list->list[i].handle = e->data.evt_gatt_service.service;
				reverse_endian(e->data.evt_gatt_service.uuid.data, e->data.evt_gatt_service.uuid.len);
				hex2str(e->data.evt_gatt_service.uuid.data, e->data.evt_gatt_service.uuid.len,service_list->list[i].uuid);
			}
			i++;
		}
	}else{
        special_evt_num = 0;
		// evt->header = 0;
		return GL_ERR_EVENT_MISSING;
	}

    // clean evt count
    special_evt_num = 0;

	if(BGLIB_MSG_ID(evt->header) == gecko_evt_gatt_procedure_completed_id)
	{
        evt->header = 0;
		return GL_SUCCESS;
	}else{
		return GL_ERR_EVENT_MISSING;
	}
}


GL_RET silabs_ble_get_char(gl_ble_char_list_t *char_list, BLE_MAC address, int service_handle)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_gatt_discover_characteristics_rsp_t* rsp = gecko_cmd_gatt_discover_characteristics(connection, service_handle);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

	int i = 0;
	uint32_t evt_id = gecko_evt_gatt_procedure_completed_id;
	if(wait_rsp_evt(evt_id, 600) == 0)
	{
        char_list->list_len = special_evt_num;
		while(i < special_evt_num)
		{
			struct gecko_cmd_packet* e = &special_evt[i];
			if(BGLIB_MSG_ID(e->header) == gecko_evt_gatt_characteristic_id && e->data.evt_gatt_characteristic.connection == connection)
			{
                char_list->list[i].handle = e->data.evt_gatt_characteristic.characteristic;
                char_list->list[i].properties = e->data.evt_gatt_characteristic.properties;
				reverse_endian(e->data.evt_gatt_characteristic.uuid.data, e->data.evt_gatt_characteristic.uuid.len);
				hex2str(e->data.evt_gatt_characteristic.uuid.data, e->data.evt_gatt_characteristic.uuid.len,char_list->list[i].uuid);
			}
			i++;
		}
	}else{
        special_evt_num = 0;
		// evt->header = 0;
		return GL_ERR_EVENT_MISSING;
	}

    // clean evt count
    special_evt_num = 0;
    
	if(BGLIB_MSG_ID(evt->header) == gecko_evt_gatt_procedure_completed_id)
	{
        evt->header = 0;
		return GL_SUCCESS;
	}else{
		return GL_ERR_EVENT_MISSING;
	}
}

GL_RET silabs_ble_set_power(int power, int *current_power)
{
    struct gecko_msg_system_set_tx_power_rsp_t* rsp = gecko_cmd_system_set_tx_power(power);
    if(rsp == NULL)
    {
        return GL_UNKNOW_ERR;
    }

    *current_power = rsp->set_power;

    return GL_SUCCESS;
}

GL_RET silabs_ble_read_char(BLE_MAC address, int char_handle)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_gatt_read_characteristic_value_rsp_t* rsp = gecko_cmd_gatt_read_characteristic_value(connection, char_handle);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);       
    }

	return GL_SUCCESS;
}

GL_RET silabs_ble_write_char(BLE_MAC address, int char_handle, char *value, int res)
{
	int connection = 0;
	char address_str[BLE_MAC_LEN] = {0};
	addr2str(address, address_str);
	GL_RET ret = ble_dev_mgr_get_connection(address_str, &connection);
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    if((!value) || (strlen(value)%2))
    {
        return GL_ERR_PARAM;
    }
    int len = strlen(value)/2;
    unsigned char data[256];
    str2array(data,value,len);

    if(res)
    {
        struct gecko_msg_gatt_write_characteristic_value_rsp_t* rsp = gecko_cmd_gatt_write_characteristic_value(connection, char_handle, len, data);
        if(!rsp)
        {
            return GL_ERR_RESP_MISSING;
        }
		if(rsp->result)
		{
            return (rsp->result + MANUFACTURER_ERR_BASE);
		}

    }else{
        struct gecko_msg_gatt_write_characteristic_value_without_response_rsp_t* rsp = gecko_cmd_gatt_write_characteristic_value_without_response(connection, char_handle, len, data);
        if(!rsp)
        {
            return GL_ERR_RESP_MISSING;
        }
		if(rsp->result)
		{
            return (rsp->result + MANUFACTURER_ERR_BASE);
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
	if(ret != GL_SUCCESS)
	{
		return GL_ERR_PARAM;
	}

    struct gecko_msg_gatt_set_characteristic_notification_rsp_t* rsp = gecko_cmd_gatt_set_characteristic_notification(connection, char_handle, flag);
    if(!rsp)
    {
        return GL_ERR_RESP_MISSING;
    }
    if(rsp->result)
    {
        return (rsp->result + MANUFACTURER_ERR_BASE);
    }

    return GL_SUCCESS;
}