#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "silabs_msg.h"
#include "gl_common.h"
#include "bg_types.h"
#include "host_gecko.h"
#include "gl_log.h"
#include "gl_dev_mgr.h"
#include "gl_type.h"

#include "silabs_evt.h"


static int evt_msgid;

void* silabs_watcher(void* arg)
{
    watcher_param_t* sbs_param = (watcher_param_t*)arg;
    evt_msgid = sbs_param->evt_msgid;
    gl_ble_cbs* ble_msg_cb = sbs_param->cbs;
    
    silabs_msg_queue_t queue_data;

    while(1)
    {
        // set cancellation point 
        pthread_testcancel();

        // recv evt from msg queue
        if(-1 == msgrcv(evt_msgid, (void*)&queue_data, sizeof(struct gecko_cmd_packet), 0, 0))
        {
            log_err("msgrcv error!!!\n");
            continue;
        }

        struct gecko_cmd_packet *p = &(queue_data.evt);

        switch(BGLIB_MSG_ID(p->header))
        {
            case gecko_evt_system_boot_id:
            {
                gl_ble_module_data_t data;
                data.system_boot_data.major = p->data.evt_system_boot.major;
                data.system_boot_data.minor = p->data.evt_system_boot.minor;
                data.system_boot_data.patch = p->data.evt_system_boot.patch;
                data.system_boot_data.build = p->data.evt_system_boot.build;
                data.system_boot_data.bootloader = p->data.evt_system_boot.bootloader;
                data.system_boot_data.hw = p->data.evt_system_boot.hw;
                hex2str((uint8*)&p->data.evt_system_boot.hash,sizeof(uint32),data.system_boot_data.ble_hash);

                if(ble_msg_cb->ble_module_event)
                {
                    ble_msg_cb->ble_module_event(MODULE_BLE_SYSTEM_BOOT_EVT, &data);
                }

                break;
            }
            case gecko_evt_le_connection_closed_id:
            {
                gl_ble_gap_data_t data;
                data.disconnect_data.reason = p->data.evt_le_connection_closed.reason;
                char tmp_address[MAC_STR_LEN] = {0};
                uint16_t ret = ble_dev_mgr_get_address(p->data.evt_le_connection_closed.connection, tmp_address);
                if(ret != 0)
                {
                    log_err("get dev mac from dev-list failed!\n");
                    return NULL;
                }
                str2addr(tmp_address, data.disconnect_data.address);
                
                // delete from dev-list
                ble_dev_mgr_del(p->data.evt_le_connection_closed.connection);

                if(ble_msg_cb->ble_gap_event)
                {
                    ble_msg_cb->ble_gap_event(GAP_BLE_DISCONNECT_EVT, &data);
                }

                break;
            }
            case gecko_evt_gatt_characteristic_value_id:
            {
                gl_ble_gatt_data_t data;
                data.remote_characteristic_value.offset = p->data.evt_gatt_characteristic_value.offset;
                data.remote_characteristic_value.att_opcode = p->data.evt_gatt_characteristic_value.att_opcode;
                data.remote_characteristic_value.characteristic = p->data.evt_gatt_characteristic_value.characteristic;
                hex2str(p->data.evt_gatt_characteristic_value.value.data,p->data.evt_gatt_characteristic_value.value.len,data.remote_characteristic_value.value);
                
                char tmp_address[MAC_STR_LEN] = {0};
                uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_characteristic_value.connection, tmp_address);
                if(ret != 0)
                {
                    log_err("get dev mac from dev-list failed!\n");
                    return NULL;
                }
                str2addr(tmp_address, data.remote_characteristic_value.address);

                if(ble_msg_cb->ble_gatt_event)
                {
                    ble_msg_cb->ble_gatt_event(GATT_REMOTE_CHARACTERISTIC_VALUE_EVT, &data);
                }

                break;
            }
            case gecko_evt_gatt_server_attribute_value_id:
            {
                gl_ble_gatt_data_t data;
                data.local_gatt_attribute.offset = p->data.evt_gatt_server_attribute_value.offset;
                data.local_gatt_attribute.attribute = p->data.evt_gatt_server_attribute_value.attribute;
                data.local_gatt_attribute.att_opcode = p->data.evt_gatt_server_attribute_value.att_opcode;
                hex2str(p->data.evt_gatt_server_attribute_value.value.data,p->data.evt_gatt_server_attribute_value.value.len,data.local_gatt_attribute.value);
                
                char tmp_address[MAC_STR_LEN] = {0};
                uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_server_attribute_value.connection, tmp_address);
                if(ret != 0)
                {
                    log_err("get dev mac from dev-list failed!\n");
                    return NULL;
                }
                str2addr(tmp_address, data.local_gatt_attribute.address);

                if(ble_msg_cb->ble_gatt_event)
                {
                    ble_msg_cb->ble_gatt_event(GATT_LOCAL_GATT_ATT_EVT, &data);
                }

                break;
            }
            case gecko_evt_gatt_server_characteristic_status_id:
            {
                gl_ble_gatt_data_t data;
                data.local_characteristic_status.status_flags = p->data.evt_gatt_server_characteristic_status.status_flags;
                data.local_characteristic_status.characteristic = p->data.evt_gatt_server_characteristic_status.characteristic;
                data.local_characteristic_status.client_config_flags = p->data.evt_gatt_server_characteristic_status.client_config_flags;

                char tmp_address[MAC_STR_LEN] = {0};
                uint16_t ret = ble_dev_mgr_get_address(p->data.evt_gatt_server_characteristic_status.connection, tmp_address);
                if(ret != 0)
                {
                    log_err("get dev mac from dev-list failed!\n");
                    return NULL;
                }
                str2addr(tmp_address, data.local_characteristic_status.address);

                if(ble_msg_cb->ble_gatt_event)
                {
                    ble_msg_cb->ble_gatt_event(GATT_LOCAL_CHARACTERISTIC_STATUS_EVT, &data);
                }

                break;
            }
            case gecko_evt_le_gap_scan_response_id:
            {
                gl_ble_gap_data_t data;
                data.scan_rst.rssi = p->data.evt_le_gap_scan_response.rssi;
                data.scan_rst.bonding = p->data.evt_le_gap_scan_response.bonding;
                data.scan_rst.packet_type = p->data.evt_le_gap_scan_response.packet_type;
                data.scan_rst.ble_addr_type = p->data.evt_le_gap_scan_response.address_type;
                hex2str(p->data.evt_le_gap_scan_response.data.data, p->data.evt_le_gap_scan_response.data.len,data.scan_rst.ble_adv);
                memcpy(data.scan_rst.address, p->data.evt_le_gap_scan_response.address.addr, 6);

                if(ble_msg_cb->ble_gap_event)
                {
                    ble_msg_cb->ble_gap_event(GAP_BLE_SCAN_RESULT_EVT, &data);
                }
                break;
            }
            case gecko_evt_le_connection_parameters_id:
            {
                gl_ble_gap_data_t data;
                data.update_conn_data.txsize = p->data.evt_le_connection_parameters.txsize;
                data.update_conn_data.latency = p->data.evt_le_connection_parameters.latency;
                data.update_conn_data.timeout = p->data.evt_le_connection_parameters.timeout;
                data.update_conn_data.interval = p->data.evt_le_connection_parameters.interval;
                data.update_conn_data.security_mode = p->data.evt_le_connection_parameters.security_mode;

                char tmp_address[MAC_STR_LEN] = {0};
                uint16_t ret = ble_dev_mgr_get_address(p->data.evt_le_connection_parameters.connection, tmp_address);
                if(ret != 0)
                {
                    log_err("get dev mac from dev-list failed!\n");
                    return NULL;
                }
                str2addr(tmp_address, data.update_conn_data.address);

                if(ble_msg_cb->ble_gap_event)
                {
                    ble_msg_cb->ble_gap_event(GAP_BLE_UPDATE_CONN_EVT, &data);
                }

                break;
            }
            case gecko_evt_le_connection_opened_id:
            {
                char addr[MAC_STR_LEN] = {0};
                addr2str(p->data.evt_le_connection_opened.address.addr,addr);
                ble_dev_mgr_add(addr, p->data.evt_le_connection_opened.connection);

                gl_ble_gap_data_t data;
                data.connect_open_data.bonding = p->data.evt_le_connection_opened.bonding;
                data.connect_open_data.conn_role = p->data.evt_le_connection_opened.master;
                data.connect_open_data.advertiser = p->data.evt_le_connection_opened.advertiser;
                data.connect_open_data.ble_addr_type = p->data.evt_le_connection_opened.address_type;
                memcpy(data.connect_open_data.address, p->data.evt_le_connection_opened.address.addr, 6);

                if(ble_msg_cb->ble_gap_event)
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
