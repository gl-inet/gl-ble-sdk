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
#ifndef _GL_DEV_MGR_H_
#define _GL_DEV_MGR_H_

#include <libubox/list.h>
#include <stdint.h>
#include <json-c/json.h>

#define MAC_STR_LEN 18

typedef struct _silabs_dev_desc_t
{
  char dev_addr[MAC_STR_LEN];
  uint16_t connection;
} ble_dev_desc_t;

typedef struct
{
    ble_dev_desc_t ble_dev_desc;
    uint32_t timestamp;
    struct list_head linked_list;
} ble_dev_mgr_node_t;

typedef struct
{
    void* dev_list_mutex;
    struct list_head dev_list;
} ble_dev_mgr_ctx_t;

void ble_dev_mgr_print(void);

int ble_dev_mgr_init(void);

int ble_dev_mgr_del_all(void);

int ble_dev_mgr_add(char *dev_addr, uint16_t connection);

int ble_dev_mgr_del(uint16_t connection);

int ble_dev_mgr_update(uint16_t connection);

uint16_t ble_dev_mgr_get_connection(char *dev_addr, int* connection);

uint16_t ble_dev_mgr_get_address(uint16_t connection, char *mac);

int ble_dev_mgr_get_list_size(void);

int ble_dev_mgr_destroy(void);

#endif // !_GL_DEV_MGR_H_
