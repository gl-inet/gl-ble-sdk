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
#include "gl_uart.h"
#include "gl_thread.h"
#include "timestamp.h"
#include "gl_log.h"
#include "gl_dev_mgr.h"
#include "silabs_bleapi.h"
#include "gl_type.h"
#include "gl_hal.h"
#include "silabs_evt.h"
#include "sli_bt_api.h"

BGLIB_DEFINE();
bool appBooted = false; // App booted flag

bool wait_reset_flag = false;

struct sl_bt_packet *evt = NULL;

// save resp data evt
struct sl_bt_packet special_evt[SPE_EVT_MAX];
int special_evt_num = 0;

struct sl_bt_packet *gecko_get_event(int block);
struct sl_bt_packet *gecko_wait_event(void);
struct sl_bt_packet *gecko_wait_message(void); // wait for event from system

void silabs_event_handler(struct sl_bt_packet *p);
static void reverse_rev_payload(struct sl_bt_packet *pck);

static int evt_msqid;

void *silabs_driver(void *arg)
{
    driver_param_t driver_param = *((driver_param_t *)arg);
    evt_msqid = driver_param.evt_msgid;

    while (1)
    {
        // Check for stack event.
        evt = gecko_wait_event();

        // Run application and event handler.
        silabs_event_handler(evt);

        // set cancellation point
        pthread_testcancel();
    }

    return NULL;
}

/*
 * wait for module events
 */
struct sl_bt_packet *gecko_wait_event(void)
{
    // block mode
    return gecko_get_event(1);
}

/*
 *
 */
struct sl_bt_packet *gecko_get_event(int block)
{
    struct sl_bt_packet *p;

    while (1)
    {
        if (sl_bt_queue_w != sl_bt_queue_r)
        {
            p = &sl_bt_queue_buffer[sl_bt_queue_r];
            sl_bt_queue_r = (sl_bt_queue_r + 1) % SL_BT_API_QUEUE_LEN;
            return p;
        }

        // reset
        if (wait_reset_flag)
        {
            system(rstoff);

            // clean dev list
            ble_dev_mgr_del_all();

            // clean uart cache
            usleep(100 * 1000);
            uartCacheClean();

            appBooted = false;
            wait_reset_flag = false;
        }

        p = gecko_wait_message();
        if (p)
        {
            return p;
        }
    }
}

struct sl_bt_packet *gecko_wait_message(void) // wait for event from system
{
    uint32_t msg_length;
    uint32_t header = 0;
    uint8_t *payload;
    struct sl_bt_packet *pck, *retVal = NULL;
    int ret;

    int dataToRead = SL_BT_MSG_HEADER_LEN;
    uint8_t *header_p = (uint8_t *)&header;

    if (!appBooted)
    {
        while (1)
        {
            ret = uartRxNonBlocking(1, header_p);
            if (ret == 1)
            {
                if (*header_p == 0xa0)
                {
                    break;
                }
            }

            if (wait_reset_flag)
            {
                return 0;
            }
        }

        dataToRead--;
        header_p++;
    }

    while (dataToRead)
    {
        ret = uartRxNonBlocking(dataToRead, header_p);
        if (ret != -1)
        {
            dataToRead -= ret;
            header_p += ret;
        }
        else
        {
            return 0;
        }

        if (wait_reset_flag)
        {
            return 0;
        }
    }

    if (ENDIAN)
    {
        reverse_endian((uint8_t *)&header, SL_BT_MSG_HEADER_LEN);
    }

    if (ret < 0 || (header & 0x78) != sl_bgapi_dev_type_bt)
    {
        return 0;
    }

    msg_length = SL_BT_MSG_LEN(header);

    if (msg_length > SL_BGAPI_MAX_PAYLOAD_SIZE)
    {
        return 0;
    }

    if ((header & 0xf8) == (sl_bgapi_dev_type_bt | sl_bgapi_msg_type_evt))
    {
        // received event
        if ((sl_bt_queue_w + 1) % SL_BT_API_QUEUE_LEN == sl_bt_queue_r)
        {
            // drop packet
            if (msg_length)
            {
                uint8_t tmp_payload[SL_BGAPI_MAX_PAYLOAD_SIZE];
                uartRx(msg_length, tmp_payload);
            }
            return 0; // NO ROOM IN QUEUE
        }
        pck = &sl_bt_queue_buffer[sl_bt_queue_w];
        sl_bt_queue_w = (sl_bt_queue_w + 1) % SL_BT_API_QUEUE_LEN;
    }
    else if ((header & 0xf8) == sl_bgapi_dev_type_bt)
    {
        // response
        retVal = pck = sl_bt_rsp_msg;
    }
    else
    {
        // fail
        return 0;
    }
    pck->header = header;
    payload = (uint8_t *)&pck->data.payload;

    // Read the payload data if required and store it after the header.
    if (msg_length > 0)
    {

        ret = uartRx(msg_length, payload);
        if (ret < 0)
        {
            // log_err("recv fail\n");
            return 0;
        }
    }
    // log_hexdump((uint8_t *)&header, 4);
    // log_hexdump(payload, msg_length);
    if (ENDIAN)
    {
        reverse_rev_payload(pck);
    }

    // Using retVal avoid double handling of event msg types in outer function
    return retVal;
}

int rx_peek_timeout(int ms)
{
    int timeout = ms * 10;
    while (timeout)
    {
        timeout--;
        if (SL_BT_MSG_ID(sl_bt_cmd_msg->header) == SL_BT_MSG_ID(sl_bt_rsp_msg->header))
        {
            return 0;
        }
        usleep(100);
    }

    return -1;
}

void sl_bt_host_handle_command()
{
    uint32_t send_msg_length = SL_BT_MSG_HEADER_LEN + SL_BT_MSG_LEN(sl_bt_cmd_msg->header);
    if (ENDIAN)
    {
        reverse_endian((uint8_t *)&sl_bt_cmd_msg->header, SL_BT_MSG_HEADER_LEN);
    }
    sl_bt_rsp_msg->header = 0;
    // log_hexdump((uint8_t *)sl_bt_cmd_msg, send_msg_length);
    uartTx(send_msg_length, (uint8_t *)sl_bt_cmd_msg); // send cmd msg

    rx_peek_timeout(300); // wait for response
}

void sl_bt_host_handle_command_noresponse()
{
    uint32_t send_msg_length = SL_BT_MSG_HEADER_LEN + SL_BT_MSG_LEN(sl_bt_cmd_msg->header);
    if (ENDIAN)
    {
        reverse_endian((uint8_t *)&sl_bt_cmd_msg->header, SL_BT_MSG_HEADER_LEN);
    }
    // log_hexdump((uint8_t *)&sl_bt_cmd_msg, send_msg_length);
    uartTx(send_msg_length, (uint8_t *)sl_bt_cmd_msg); // send cmd msg
}

/*
 *	module events report
 */
// static silabs_msg_queue_t msg_data;
static silabs_msg_queue_t msg_data;
static int msg_evt_len;
void silabs_event_handler(struct sl_bt_packet *p)
{
    // printf("Event handler: 0x%04x\n", SL_BT_MSG_ID(evt->header));

    // Do not handle any events until system is booted up properly.
    if ((SL_BT_MSG_ID(evt->header) != sl_bt_evt_system_boot_id) && !appBooted)
    {
        log_debug("Wait for system boot ... \n");
        // usleep(50000);
        return;
    }

    switch (SL_BT_MSG_ID(p->header))
    {
    case sl_bt_evt_system_boot_id:
    {
        appBooted = true;
    }
    case sl_bt_evt_connection_closed_id:
    case sl_bt_evt_gatt_characteristic_value_id:
    case sl_bt_evt_gatt_server_attribute_value_id:
    case sl_bt_evt_gatt_server_characteristic_status_id:
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
    case sl_bt_evt_scanner_extended_advertisement_report_id:
    case sl_bt_evt_sync_data_id:
    case sl_bt_evt_sync_closed_id:
    case sl_bt_evt_connection_parameters_id:
    case sl_bt_evt_connection_opened_id:
    {
        msg_data.msgtype = 1;
        msg_evt_len = SL_BT_MSG_HEADER_LEN + SL_BT_MSG_LEN(p->header);
        memcpy(&(msg_data.evt), p, msg_evt_len);

        // send evt msg to msg queue
        if (-1 == msgsnd(evt_msqid, (void *)&msg_data, msg_evt_len, IPC_NOWAIT))
        {
            perror("msgsnd");
            log_debug("silabs evt msgsnd error!  errno: %d\n", errno);
        }
        break;
    }

    case sl_bt_evt_gatt_service_id:
    case sl_bt_evt_gatt_characteristic_id:
    {
        special_evt[special_evt_num].header = p->header;
        memcpy(&special_evt[special_evt_num].data.payload, p->data.payload, SL_BGAPI_MAX_PAYLOAD_SIZE);
        special_evt_num++;
        break;
    }
    default:
        break;
    }

    return;
}

int wait_rsp_evt(uint32_t evt_id, uint32_t timeout)
{
    uint32_t spend = 0;

    while (timeout > spend)
    {
        if (evt_id == SL_BT_MSG_ID(evt->header))
        {
            return 0;
        }
        usleep(1000);
        spend++;
    }
    return -1;
}

static void reverse_rev_payload(struct sl_bt_packet *pck)
{
    uint32_t p = SL_BT_MSG_ID(pck->header);
    //   log_debug("p: %04x %04x\n", p, SL_BT_MSG_ID(pck->header));

    switch (p)
    {

    case sl_bt_rsp_dfu_flash_set_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_dfu_flash_set_address.result), 2);
        break;
    case sl_bt_rsp_dfu_flash_upload_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_dfu_flash_upload.result), 2);
        break;
    case sl_bt_rsp_dfu_flash_upload_finish_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_dfu_flash_upload_finish.result), 2);
        break;
    case sl_bt_rsp_system_hello_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_hello.result), 2);
        break;
    case sl_bt_rsp_system_start_bluetooth_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_start_bluetooth.result), 2);
        break;
    case sl_bt_rsp_system_stop_bluetooth_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_stop_bluetooth.result), 2);
        break;
    case sl_bt_rsp_system_get_version_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_get_version.result), 2);
        break;
    case sl_bt_rsp_system_halt_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_halt.result), 2);
        break;
    case sl_bt_rsp_system_linklayer_configure_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_linklayer_configure.result), 2);
        break;
    case sl_bt_rsp_system_set_tx_power_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_set_tx_power.result), 2);
        break;
    case sl_bt_rsp_system_get_tx_power_setting_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_get_tx_power_setting.result), 2);
        break;
    case sl_bt_rsp_system_set_identity_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_set_identity_address.result), 2);
        break;
    case sl_bt_rsp_system_get_identity_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_get_identity_address.result), 2);
        break;
    case sl_bt_rsp_system_get_random_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_get_random_data.result), 2);
        break;
    case sl_bt_rsp_system_data_buffer_write_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_data_buffer_write.result), 2);
        break;
    case sl_bt_rsp_system_data_buffer_clear_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_data_buffer_clear.result), 2);
        break;
    case sl_bt_rsp_system_get_counters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_get_counters.result), 2);
        break;
    case sl_bt_rsp_system_set_lazy_soft_timer_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_system_set_lazy_soft_timer.result), 2);
        break;
    case sl_bt_rsp_gap_set_privacy_mode_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gap_set_privacy_mode.result), 2);
        break;
    case sl_bt_rsp_gap_set_data_channel_classification_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gap_set_data_channel_classification.result), 2);
        break;
    case sl_bt_rsp_gap_enable_whitelisting_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gap_enable_whitelisting.result), 2);
        break;
    case sl_bt_rsp_advertiser_create_set_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_create_set.result), 2);
        break;
    case sl_bt_rsp_advertiser_configure_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_configure.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_timing_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_timing.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_channel_map_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_channel_map.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_tx_power_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_tx_power.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_report_scan_request_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_report_scan_request.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_random_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_random_address.result), 2);
        break;
    case sl_bt_rsp_advertiser_clear_random_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_clear_random_address.result), 2);
        break;
    case sl_bt_rsp_advertiser_stop_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_stop.result), 2);
        break;
    case sl_bt_rsp_advertiser_delete_set_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_delete_set.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_phy_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_phy.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_configuration_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_configuration.result), 2);
        break;
    case sl_bt_rsp_advertiser_clear_configuration_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_clear_configuration.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_data.result), 2);
        break;
    case sl_bt_rsp_advertiser_set_long_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_set_long_data.result), 2);
        break;
    case sl_bt_rsp_advertiser_start_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_start.result), 2);
        break;
    case sl_bt_rsp_advertiser_start_periodic_advertising_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_start_periodic_advertising.result), 2);
        break;
    case sl_bt_rsp_advertiser_stop_periodic_advertising_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_stop_periodic_advertising.result), 2);
        break;
    case sl_bt_rsp_legacy_advertiser_set_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_legacy_advertiser_set_data.result), 2);
        break;
    case sl_bt_rsp_legacy_advertiser_generate_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_legacy_advertiser_generate_data.result), 2);
        break;
    case sl_bt_rsp_legacy_advertiser_start_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_legacy_advertiser_start.result), 2);
        break;
    case sl_bt_rsp_legacy_advertiser_start_directed_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_legacy_advertiser_start_directed.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_set_phy_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_set_phy.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_set_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_set_data.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_set_long_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_set_long_data.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_generate_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_generate_data.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_start_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_start.result), 2);
        break;
    case sl_bt_rsp_extended_advertiser_start_directed_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_extended_advertiser_start_directed.result), 2);
        break;
    case sl_bt_rsp_periodic_advertiser_set_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_periodic_advertiser_set_data.result), 2);
        break;
    case sl_bt_rsp_periodic_advertiser_set_long_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_periodic_advertiser_set_long_data.result), 2);
        break;
    case sl_bt_rsp_periodic_advertiser_start_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_periodic_advertiser_start.result), 2);
        break;
    case sl_bt_rsp_periodic_advertiser_stop_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_periodic_advertiser_stop.result), 2);
        break;
    case sl_bt_rsp_scanner_set_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_scanner_set_parameters.result), 2);
        break;
    case sl_bt_rsp_scanner_stop_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_scanner_stop.result), 2);
        break;
    case sl_bt_rsp_scanner_set_timing_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_scanner_set_timing.result), 2);
        break;
    case sl_bt_rsp_scanner_set_mode_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_scanner_set_mode.result), 2);
        break;
    case sl_bt_rsp_scanner_start_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_scanner_start.result), 2);
        break;
    case sl_bt_rsp_sync_set_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_set_parameters.result), 2);
        break;
    case sl_bt_rsp_sync_open_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_open.result), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_open.sync), 2);
        break;
    case sl_bt_rsp_sync_set_reporting_mode_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_set_reporting_mode.result), 2);
        break;
    case sl_bt_rsp_sync_close_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_close.result), 2);
        break;
    case sl_bt_rsp_past_receiver_set_default_sync_receive_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_past_receiver_set_default_sync_receive_parameters.result), 2);
        break;
    case sl_bt_rsp_past_receiver_set_sync_receive_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_past_receiver_set_sync_receive_parameters.result), 2);
        break;
    case sl_bt_rsp_advertiser_past_transfer_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_advertiser_past_transfer.result), 2);
        break;
    case sl_bt_rsp_sync_past_transfer_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sync_past_transfer.result), 2);
        break;
    case sl_bt_rsp_connection_set_default_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_default_parameters.result), 2);
        break;
    case sl_bt_rsp_connection_set_default_preferred_phy_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_default_preferred_phy.result), 2);
        break;
    case sl_bt_rsp_connection_open_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_open.result), 2);
        break;
    case sl_bt_rsp_connection_set_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_parameters.result), 2);
        break;
    case sl_bt_rsp_connection_set_preferred_phy_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_preferred_phy.result), 2);
        break;
    case sl_bt_rsp_connection_disable_slave_latency_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_disable_slave_latency.result), 2);
        break;
    case sl_bt_rsp_connection_get_rssi_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_get_rssi.result), 2);
        break;
    case sl_bt_rsp_connection_read_channel_map_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_read_channel_map.result), 2);
        break;
    case sl_bt_rsp_connection_set_power_reporting_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_power_reporting.result), 2);
        break;
    case sl_bt_rsp_connection_set_remote_power_reporting_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_set_remote_power_reporting.result), 2);
        break;
    case sl_bt_rsp_connection_get_tx_power_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_get_tx_power.result), 2);
        break;
    case sl_bt_rsp_connection_get_remote_tx_power_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_get_remote_tx_power.result), 2);
        break;
    case sl_bt_rsp_connection_close_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_close.result), 2);
        break;
    case sl_bt_rsp_connection_read_remote_used_features_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_read_remote_used_features.result), 2);
        break;
    case sl_bt_rsp_connection_get_security_status_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_connection_get_security_status.result), 2);
        break;
    case sl_bt_rsp_gatt_set_max_mtu_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_set_max_mtu.result), 2);
        break;
    case sl_bt_rsp_gatt_discover_primary_services_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_discover_primary_services.result), 2);
        break;
    case sl_bt_rsp_gatt_discover_primary_services_by_uuid_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_discover_primary_services_by_uuid.result), 2);
        break;
    case sl_bt_rsp_gatt_find_included_services_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_find_included_services.result), 2);
        break;
    case sl_bt_rsp_gatt_discover_characteristics_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_discover_characteristics.result), 2);
        break;
    case sl_bt_rsp_gatt_discover_characteristics_by_uuid_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_discover_characteristics_by_uuid.result), 2);
        break;
    case sl_bt_rsp_gatt_discover_descriptors_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_discover_descriptors.result), 2);
        break;
    case sl_bt_rsp_gatt_set_characteristic_notification_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_set_characteristic_notification.result), 2);
        break;
    case sl_bt_rsp_gatt_send_characteristic_confirmation_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_send_characteristic_confirmation.result), 2);
        break;
    case sl_bt_rsp_gatt_read_characteristic_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_read_characteristic_value.result), 2);
        break;
    case sl_bt_rsp_gatt_read_characteristic_value_from_offset_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_read_characteristic_value_from_offset.result), 2);
        break;
    case sl_bt_rsp_gatt_read_multiple_characteristic_values_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_read_multiple_characteristic_values.result), 2);
        break;
    case sl_bt_rsp_gatt_read_characteristic_value_by_uuid_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_read_characteristic_value_by_uuid.result), 2);
        break;
    case sl_bt_rsp_gatt_write_characteristic_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_write_characteristic_value.result), 2);
        break;
    case sl_bt_rsp_gatt_write_characteristic_value_without_response_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_write_characteristic_value_without_response.sent_len), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_write_characteristic_value_without_response.result), 2);
        break;
    case sl_bt_rsp_gatt_prepare_characteristic_value_write_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_prepare_characteristic_value_write.result), 2);
        break;
    case sl_bt_rsp_gatt_prepare_characteristic_value_reliable_write_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_prepare_characteristic_value_reliable_write.result), 2);
        break;
    case sl_bt_rsp_gatt_execute_characteristic_value_write_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_execute_characteristic_value_write.result), 2);
        break;
    case sl_bt_rsp_gatt_read_descriptor_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_read_descriptor_value.result), 2);
        break;
    case sl_bt_rsp_gatt_write_descriptor_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_write_descriptor_value.result), 2);
        break;
    case sl_bt_rsp_gattdb_new_session_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_new_session.session), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_new_session.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_service.service), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_remove_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_remove_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_included_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_included_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_remove_included_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_remove_included_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_uuid16_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid16_characteristic.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid16_characteristic.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_uuid128_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid128_characteristic.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid128_characteristic.result), 2);
        break;
    case sl_bt_rsp_gattdb_remove_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_remove_characteristic.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_uuid16_descriptor_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid16_descriptor.descriptor), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid16_descriptor.result), 2);
        break;
    case sl_bt_rsp_gattdb_add_uuid128_descriptor_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid128_descriptor.descriptor), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_add_uuid128_descriptor.result), 2);
        break;
    case sl_bt_rsp_gattdb_remove_descriptor_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_remove_descriptor.result), 2);
        break;
    case sl_bt_rsp_gattdb_start_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_start_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_stop_service_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_stop_service.result), 2);
        break;
    case sl_bt_rsp_gattdb_start_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_start_characteristic.result), 2);
        break;
    case sl_bt_rsp_gattdb_stop_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_stop_characteristic.result), 2);
        break;
    case sl_bt_rsp_gattdb_commit_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_commit.result), 2);
        break;
    case sl_bt_rsp_gattdb_abort_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gattdb_abort.result), 2);
        break;
    case sl_bt_rsp_gatt_server_set_max_mtu_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_set_max_mtu.result), 2);
        break;
    case sl_bt_rsp_gatt_server_get_mtu_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_get_mtu.result), 2);
        break;
    case sl_bt_rsp_gatt_server_find_attribute_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_find_attribute.attribute), 2);
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_find_attribute.result), 2);
        break;
    case sl_bt_rsp_gatt_server_read_attribute_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_read_attribute_value.result), 2);
        break;
    case sl_bt_rsp_gatt_server_read_attribute_type_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_read_attribute_type.result), 2);
        break;
    case sl_bt_rsp_gatt_server_write_attribute_value_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_write_attribute_value.result), 2);
        break;
    case sl_bt_rsp_gatt_server_send_user_read_response_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_send_user_read_response.result), 2);
        break;
    case sl_bt_rsp_gatt_server_send_user_write_response_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_send_user_write_response.result), 2);
        break;
    case sl_bt_rsp_gatt_server_send_notification_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_send_notification.result), 2);
        break;
    case sl_bt_rsp_gatt_server_send_indication_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_send_indication.result), 2);
        break;
    case sl_bt_rsp_gatt_server_notify_all_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_notify_all.result), 2);
        break;
    case sl_bt_rsp_gatt_server_read_client_configuration_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_read_client_configuration.result), 2);
        break;
    case sl_bt_rsp_gatt_server_send_user_prepare_write_response_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_send_user_prepare_write_response.result), 2);
        break;
    case sl_bt_rsp_gatt_server_set_capabilities_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_set_capabilities.result), 2);
        break;
    case sl_bt_rsp_gatt_server_enable_capabilities_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_enable_capabilities.result), 2);
        break;
    case sl_bt_rsp_gatt_server_disable_capabilities_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_disable_capabilities.result), 2);
        break;
    case sl_bt_rsp_gatt_server_get_enabled_capabilities_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_get_enabled_capabilities.result), 2);
        break;
    case sl_bt_rsp_gatt_server_read_client_supported_features_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_gatt_server_read_client_supported_features.result), 2);
        break;
    case sl_bt_rsp_nvm_save_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_nvm_save.result), 2);
        break;
    case sl_bt_rsp_nvm_load_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_nvm_load.result), 2);
        break;
    case sl_bt_rsp_nvm_erase_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_nvm_erase.result), 2);
        break;
    case sl_bt_rsp_nvm_erase_all_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_nvm_erase_all.result), 2);
        break;
    case sl_bt_rsp_test_dtm_tx_v4_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_test_dtm_tx_v4.result), 2);
        break;
    case sl_bt_rsp_test_dtm_tx_cw_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_test_dtm_tx_cw.result), 2);
        break;
    case sl_bt_rsp_test_dtm_rx_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_test_dtm_rx.result), 2);
        break;
    case sl_bt_rsp_test_dtm_end_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_test_dtm_end.result), 2);
        break;
    case sl_bt_rsp_sm_configure_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_configure.result), 2);
        break;
    case sl_bt_rsp_sm_set_minimum_key_size_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_minimum_key_size.result), 2);
        break;
    case sl_bt_rsp_sm_set_debug_mode_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_debug_mode.result), 2);
        break;
    case sl_bt_rsp_sm_add_to_whitelist_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_add_to_whitelist.result), 2);
        break;
    case sl_bt_rsp_sm_store_bonding_configuration_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_store_bonding_configuration.result), 2);
        break;
    case sl_bt_rsp_sm_set_bondable_mode_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_bondable_mode.result), 2);
        break;
    case sl_bt_rsp_sm_set_passkey_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_passkey.result), 2);
        break;
    case sl_bt_rsp_sm_increase_security_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_increase_security.result), 2);
        break;
    case sl_bt_rsp_sm_enter_passkey_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_enter_passkey.result), 2);
        break;
    case sl_bt_rsp_sm_passkey_confirm_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_passkey_confirm.result), 2);
        break;
    case sl_bt_rsp_sm_bonding_confirm_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_bonding_confirm.result), 2);
        break;
    case sl_bt_rsp_sm_delete_bonding_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_delete_bonding.result), 2);
        break;
    case sl_bt_rsp_sm_delete_bondings_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_delete_bondings.result), 2);
        break;
    case sl_bt_rsp_sm_get_bonding_handles_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_get_bonding_handles.result), 2);
        break;
    case sl_bt_rsp_sm_get_bonding_details_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_get_bonding_details.result), 2);
        break;
    case sl_bt_rsp_sm_find_bonding_by_address_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_find_bonding_by_address.result), 2);
        break;
    case sl_bt_rsp_sm_set_bonding_key_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_bonding_key.result), 2);
        break;
    case sl_bt_rsp_sm_set_legacy_oob_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_legacy_oob.result), 2);
        break;
    case sl_bt_rsp_sm_set_oob_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_oob.result), 2);
        break;
    case sl_bt_rsp_sm_set_remote_oob_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_remote_oob.result), 2);
        break;
    case sl_bt_rsp_sm_set_bonding_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_sm_set_bonding_data.result), 2);
        break;
    case sl_bt_rsp_ota_set_device_name_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_ota_set_device_name.result), 2);
        break;
    case sl_bt_rsp_ota_set_advertising_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_ota_set_advertising_data.result), 2);
        break;
    case sl_bt_rsp_ota_set_configuration_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_ota_set_configuration.result), 2);
        break;
    case sl_bt_rsp_ota_set_rf_path_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_ota_set_rf_path.result), 2);
        break;
    case sl_bt_rsp_coex_set_options_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_coex_set_options.result), 2);
        break;
    case sl_bt_rsp_coex_set_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_coex_set_parameters.result), 2);
        break;
    case sl_bt_rsp_coex_set_directional_priority_pulse_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_coex_set_directional_priority_pulse.result), 2);
        break;
    case sl_bt_rsp_coex_get_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_coex_get_parameters.result), 2);
        break;
    case sl_bt_rsp_coex_get_counters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_coex_get_counters.result), 2);
        break;
    case sl_bt_rsp_l2cap_open_le_channel_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_l2cap_open_le_channel.result), 2);
        break;
    case sl_bt_rsp_l2cap_send_le_channel_open_response_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_l2cap_send_le_channel_open_response.result), 2);
        break;
    case sl_bt_rsp_l2cap_channel_send_data_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_l2cap_channel_send_data.result), 2);
        break;
    case sl_bt_rsp_l2cap_channel_send_credit_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_l2cap_channel_send_credit.result), 2);
        break;
    case sl_bt_rsp_l2cap_close_channel_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_l2cap_close_channel.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_set_dtm_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_set_dtm_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_clear_dtm_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_clear_dtm_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_enable_connection_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_enable_connection_cte.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_disable_connection_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_disable_connection_cte.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_enable_connectionless_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_enable_connectionless_cte.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_disable_connectionless_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_disable_connectionless_cte.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_enable_silabs_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_enable_silabs_cte.result), 2);
        break;
    case sl_bt_rsp_cte_transmitter_disable_silabs_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_transmitter_disable_silabs_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_set_dtm_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_set_dtm_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_clear_dtm_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_clear_dtm_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_set_sync_cte_type_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_set_sync_cte_type.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_set_default_sync_receive_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_set_default_sync_receive_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_set_sync_receive_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_set_sync_receive_parameters.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_configure_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_configure.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_enable_connection_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_enable_connection_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_disable_connection_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_disable_connection_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_enable_connectionless_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_enable_connectionless_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_disable_connectionless_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_disable_connectionless_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_enable_silabs_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_enable_silabs_cte.result), 2);
        break;
    case sl_bt_rsp_cte_receiver_disable_silabs_cte_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_cte_receiver_disable_silabs_cte.result), 2);
        break;
    case sl_bt_rsp_user_message_to_target_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_user_message_to_target.result), 2);
        break;
    case sl_bt_rsp_user_manage_event_filter_id:
        reverse_endian((uint8_t *)&(pck->data.rsp_user_manage_event_filter.result), 2);
        break;
    case sl_bt_evt_dfu_boot_id:
        reverse_endian((uint8_t *)&(pck->data.evt_dfu_boot.version), 4);
        break;
    case sl_bt_evt_dfu_boot_failure_id:
        reverse_endian((uint8_t *)&(pck->data.evt_dfu_boot_failure.reason), 2);
        break;
    case sl_bt_evt_system_boot_id:
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.major), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.minor), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.patch), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.build), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.bootloader), 4);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.hw), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_system_boot.hash), 4);
        break;
    case sl_bt_evt_system_error_id:
        reverse_endian((uint8_t *)&(pck->data.evt_system_error.reason), 2);
        break;
    case sl_bt_evt_system_hardware_error_id:
        reverse_endian((uint8_t *)&(pck->data.evt_system_hardware_error.status), 2);
        break;
    case sl_bt_evt_system_external_signal_id:
        reverse_endian((uint8_t *)&(pck->data.evt_system_external_signal.extsignals), 4);
        break;
    case sl_bt_evt_system_soft_timer_id:
        break;
    case sl_bt_evt_advertiser_timeout_id:
        break;
    case sl_bt_evt_advertiser_scan_request_id:
        break;
    case sl_bt_evt_periodic_advertiser_status_id:
        reverse_endian((uint8_t *)&(pck->data.evt_periodic_advertiser_status.status), 4);
        break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
        break;
    case sl_bt_evt_scanner_extended_advertisement_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_scanner_extended_advertisement_report.periodic_interval), 2);
        break;
    case sl_bt_evt_scanner_scan_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_scanner_scan_report.periodic_interval), 2);
        break;
    case sl_bt_evt_sync_opened_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sync_opened.sync), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_opened.adv_interval), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_opened.clock_accuracy), 2);
        break;
    case sl_bt_evt_sync_transfer_received_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sync_transfer_received.status), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_transfer_received.sync), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_transfer_received.service_data), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_transfer_received.adv_interval), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_transfer_received.clock_accuracy), 2);
        break;
    case sl_bt_evt_sync_data_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sync_data.sync), 2);
        break;
    case sl_bt_evt_sync_closed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sync_closed.reason), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_sync_closed.sync), 2);
        break;
    case sl_bt_evt_connection_opened_id:
        break;
    case sl_bt_evt_connection_parameters_id:
        reverse_endian((uint8_t *)&(pck->data.evt_connection_parameters.interval), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_connection_parameters.latency), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_connection_parameters.timeout), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_connection_parameters.txsize), 2);
        break;
    case sl_bt_evt_connection_phy_status_id:
        break;
    case sl_bt_evt_connection_rssi_id:
        break;
    case sl_bt_evt_connection_get_remote_tx_power_completed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_connection_get_remote_tx_power_completed.status), 2);
        break;
    case sl_bt_evt_connection_tx_power_id:
        break;
    case sl_bt_evt_connection_remote_tx_power_id:
        break;
    case sl_bt_evt_connection_closed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_connection_closed.reason), 2);
        break;
    case sl_bt_evt_connection_remote_used_features_id:
        break;
    case sl_bt_evt_gatt_mtu_exchanged_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_mtu_exchanged.mtu), 2);
        break;
    case sl_bt_evt_gatt_service_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_service.service), 4);
        break;
    case sl_bt_evt_gatt_characteristic_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_characteristic.characteristic), 2);
        break;
    case sl_bt_evt_gatt_descriptor_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_descriptor.descriptor), 2);
        break;
    case sl_bt_evt_gatt_characteristic_value_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_characteristic_value.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_characteristic_value.offset), 2);
        break;
    case sl_bt_evt_gatt_descriptor_value_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_descriptor_value.descriptor), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_descriptor_value.offset), 2);
        break;
    case sl_bt_evt_gatt_procedure_completed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_procedure_completed.result), 2);
        break;
    case sl_bt_evt_gatt_server_attribute_value_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_attribute_value.attribute), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_attribute_value.offset), 2);
        break;
    case sl_bt_evt_gatt_server_user_read_request_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_user_read_request.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_user_read_request.offset), 2);
        break;
    case sl_bt_evt_gatt_server_user_write_request_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_user_write_request.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_user_write_request.offset), 2);
        break;
    case sl_bt_evt_gatt_server_characteristic_status_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_characteristic_status.characteristic), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_characteristic_status.client_config_flags), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_characteristic_status.client_config), 2);
        break;
    case sl_bt_evt_gatt_server_execute_write_completed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_gatt_server_execute_write_completed.result), 2);
        break;
    case sl_bt_evt_gatt_server_indication_timeout_id:
        break;
    case sl_bt_evt_test_dtm_completed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_test_dtm_completed.result), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_test_dtm_completed.number_of_packets), 2);
        break;
    case sl_bt_evt_sm_passkey_display_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sm_passkey_display.passkey), 4);
        break;
    case sl_bt_evt_sm_passkey_request_id:
        break;
    case sl_bt_evt_sm_confirm_passkey_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sm_confirm_passkey.passkey), 4);
        break;
    case sl_bt_evt_sm_bonded_id:
        break;
    case sl_bt_evt_sm_bonding_failed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_sm_bonding_failed.reason), 2);
        break;
    case sl_bt_evt_sm_confirm_bonding_id:
        break;
    case sl_bt_evt_l2cap_le_channel_open_request_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_request.spsm), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_request.cid), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_request.max_sdu), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_request.max_pdu), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_request.credit), 2);
        break;
    case sl_bt_evt_l2cap_le_channel_open_response_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_response.cid), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_response.max_sdu), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_response.max_pdu), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_response.credit), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_le_channel_open_response.errorcode), 2);
        break;
    case sl_bt_evt_l2cap_channel_data_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_channel_data.cid), 2);
        break;
    case sl_bt_evt_l2cap_channel_credit_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_channel_credit.cid), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_channel_credit.credit), 2);
        break;
    case sl_bt_evt_l2cap_channel_closed_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_channel_closed.cid), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_channel_closed.reason), 2);
        break;
    case sl_bt_evt_l2cap_command_rejected_id:
        reverse_endian((uint8_t *)&(pck->data.evt_l2cap_command_rejected.reason), 2);
        break;
    case sl_bt_evt_cte_receiver_dtm_iq_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_dtm_iq_report.status), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_dtm_iq_report.event_counter), 2);
        break;
    case sl_bt_evt_cte_receiver_connection_iq_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_connection_iq_report.status), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_connection_iq_report.event_counter), 2);
        break;
    case sl_bt_evt_cte_receiver_connectionless_iq_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_connectionless_iq_report.status), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_connectionless_iq_report.sync), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_connectionless_iq_report.event_counter), 2);
        break;
    case sl_bt_evt_cte_receiver_silabs_iq_report_id:
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_silabs_iq_report.status), 2);
        reverse_endian((uint8_t *)&(pck->data.evt_cte_receiver_silabs_iq_report.packet_counter), 2);
        break;
    case sl_bt_evt_user_message_to_host_id:
        break;
    }
}
