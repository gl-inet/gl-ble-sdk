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

#ifndef _SILABS_MSG_H_
#define _SILABS_MSG_H_

#include "sli_bt_api.h"

#ifndef SL_BT_API_QUEUE_LEN
#define SL_BT_API_QUEUE_LEN 30
#endif

#define BGLIB_DEFINE()                                     \
  struct sl_bt_packet _sl_bt_cmd_msg;                              \
  struct sl_bt_packet _sl_bt_rsp_msg;                              \
  struct sl_bt_packet *sl_bt_cmd_msg = &_sl_bt_cmd_msg;            \
  struct sl_bt_packet *sl_bt_rsp_msg = &_sl_bt_rsp_msg;            \
  struct sl_bt_packet sl_bt_queue_buffer[SL_BT_API_QUEUE_LEN];     \
  int sl_bt_queue_w = 0;                                   \
  int sl_bt_queue_r = 0;

extern struct sl_bt_packet sl_bt_queue_buffer[SL_BT_API_QUEUE_LEN];
extern int sl_bt_queue_w;
extern int sl_bt_queue_r;

#define SPE_EVT_MAX 50
extern struct sl_bt_packet special_evt[SPE_EVT_MAX];
extern int special_evt_num;

int wait_rsp_evt(uint32_t evt_id, uint32_t timeout);

typedef struct
{
  int evt_msgid;
} driver_param_t;

void *silabs_driver(void *arg);

void sl_bt_host_handle_command();
void sl_bt_host_handle_command_noresponse();

#endif