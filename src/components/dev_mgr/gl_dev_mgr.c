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

#include <pthread.h>
#include <string.h>
#include "gl_dev_mgr.h"

#include <json-c/json.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "gl_errno.h"
#include "gl_log.h"

ble_dev_mgr_ctx_t g_ble_dev_mgr = {0};

uint32_t HAL_TimeStamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec;
}

static void dev_list_MutexLock(void)
{
    log_debug("dev_list_MutexLock\n");

    if(!g_ble_dev_mgr.dev_list_mutex)
    {
        log_err("dev_list_mutex NULL!\n");
        return ;
    }

    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)g_ble_dev_mgr.dev_list_mutex))) {
        log_err("lock mutex failed: - '%s' (%d)\n", strerror(err_num), err_num);
    }
}

// static int dev_list_MutexTryLock(void)
// {
//     log_debug("dev_list_MutexTryLock\n");

//     if(!g_ble_dev_mgr.dev_list_mutex)
//     {
//         log_err("dev_list_mutex NULL!\n");
//         return -1;
//     }
    
//     return pthread_mutex_trylock((pthread_mutex_t *)g_ble_dev_mgr.dev_list_mutex);
// }

static void dev_list_MutexUnlock(void)
{
    log_debug("dev_list_MutexUnlock\n");
    
    if(!g_ble_dev_mgr.dev_list_mutex)
    {
        log_err("dev_list_mutex NULL!\n");
        return ;
    }

    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)g_ble_dev_mgr.dev_list_mutex))) {
        log_err("unlock mutex failed - '%s' (%d)\n", strerror(err_num), err_num);
    }
}


/*************************************************************************************************************/

ble_dev_mgr_ctx_t *_ble_dev_mgr_get_ctx(void) 
{
    return &g_ble_dev_mgr; 
}


int ble_dev_mgr_init(void) {

    // get list handle
    ble_dev_mgr_ctx_t *mgr_ctx = _ble_dev_mgr_get_ctx();
    memset(mgr_ctx, 0, sizeof(ble_dev_mgr_ctx_t));
    
    // init mutex
    int err_num;
    mgr_ctx->dev_list_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (NULL == mgr_ctx->dev_list_mutex) {
        log_err("create ble dev list mutex failed!\n");
        return -1;
    }

    if (0 != (err_num = pthread_mutex_init(mgr_ctx->dev_list_mutex, NULL))) {
        printf("init mutex failed\n");
        free(mgr_ctx->dev_list_mutex);
        return -1;
    }

    // init Device List
    INIT_LIST_HEAD(&mgr_ctx->dev_list);

    return GL_SUCCESS;
}

int ble_dev_mgr_destroy(void) 
{

    // get list handle
    ble_dev_mgr_ctx_t *mgr_ctx = _ble_dev_mgr_get_ctx();
    
    int err_num;

    // delete all node
    ble_dev_mgr_del_all();

    // free mutex
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mgr_ctx->dev_list_mutex))) {
        log_err("destroy mutex failed\n");
    }
    free(mgr_ctx->dev_list_mutex);

    return GL_SUCCESS;
}

void ble_dev_mgr_print(void) {
    // get lock 
    dev_list_MutexLock();

    ble_dev_mgr_ctx_t *mgr_ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *node = NULL, *next_node = NULL;

    log_debug("\nConnected devices: \n");

    list_for_each_entry_safe(node, next_node, &mgr_ctx->dev_list, linked_list) {
        if (node != NULL) {
            log_debug("dev_addr = %s, connection = %d \n",
                   node->ble_dev_desc.dev_addr, node->ble_dev_desc.connection);
        } else
            log_err("No device connection\n");
    }

    // release lock
    dev_list_MutexUnlock();
}

static int search_ble_dev_by_addr(char *dev_addr, ble_dev_mgr_node_t **node) {
    ble_dev_mgr_ctx_t *ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, linked_list) {
        if (!strcmp(search_node->ble_dev_desc.dev_addr, dev_addr)) {
            if (node) {
                *node = search_node;
            }
            return GL_SUCCESS;
        }
    }
    log_err("The device is not in the list");
    return GL_ERR_MSG;
}

static int search_ble_dev_by_connection(uint16_t connection,
                                 ble_dev_mgr_node_t **node) {
    ble_dev_mgr_ctx_t *ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *search_node = NULL;

    list_for_each_entry(search_node, &ctx->dev_list, linked_list) {
        if (search_node->ble_dev_desc.connection == connection) {
            if (node) *node = search_node;
            return GL_SUCCESS;
        }
    }
    log_err("The device is not in the list");
    return GL_ERR_MSG;
}



int ble_dev_mgr_add(char *dev_addr, uint16_t connection) 
{
    // get lock 
    dev_list_MutexLock();
    
    ble_dev_mgr_ctx_t *mgr_ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *node = NULL;
	ble_dev_mgr_node_t *search_node = NULL;

    list_for_each_entry(search_node, &mgr_ctx->dev_list, linked_list) {
        log_debug("list node: %s ---- Join node: %s\n", search_node->ble_dev_desc.dev_addr, dev_addr);
        if (!strcmp(search_node->ble_dev_desc.dev_addr, dev_addr)) {
			node = search_node;
            break;
        }
    }

	if(node == NULL)
	{
		node = malloc(sizeof(ble_dev_mgr_node_t));
		memset(node, 0, sizeof(ble_dev_mgr_node_t));

		memcpy(node->ble_dev_desc.dev_addr, dev_addr, MAC_STR_LEN);
		node->ble_dev_desc.connection = connection;

		INIT_LIST_HEAD(&node->linked_list);

		// int ret_dev_list = list_empty(&mgr_ctx->dev_list);

		list_add_tail(&node->linked_list, &mgr_ctx->dev_list);
		log_debug("Device Join: dev_addr=%s, connection=%d.\n",
			node->ble_dev_desc.dev_addr, node->ble_dev_desc.connection);
	}else{
		node->ble_dev_desc.connection = connection;
		log_debug("Device update: dev_addr=%s, connection=%d.\n",
			node->ble_dev_desc.dev_addr, node->ble_dev_desc.connection);
	}

    // release lock
    dev_list_MutexUnlock();

    return GL_SUCCESS;
}

int ble_dev_mgr_del(uint16_t connection) {

    // get lock 
    dev_list_MutexLock();

    ble_dev_mgr_node_t *node = NULL;

    if (connection == 0) {
        log_err("Connection is null");
        dev_list_MutexUnlock();
        return GL_ERR_PARAM;
    }
    if (search_ble_dev_by_connection(connection, &node) != 0) {
        log_err("The device is not in the list");
        dev_list_MutexUnlock();
        return GL_ERR_MSG;
    }

    list_del(&node->linked_list);

    log_debug("Device Leave: dev_addr=%s, connection=%d\n",
             node->ble_dev_desc.dev_addr, node->ble_dev_desc.connection);
    free(node);

    // release lock
    dev_list_MutexUnlock();

    return GL_SUCCESS;
}

uint16_t ble_dev_mgr_get_address(uint16_t connection, char *mac) {

    ble_dev_mgr_node_t *node = NULL;
    if (connection == 0) {
        log_err("Connection is null");
        return GL_ERR_PARAM;
    }

    // get lock 
    dev_list_MutexLock();

    if (search_ble_dev_by_connection(connection, &node) != 0) {
        log_err("The device is not in the list");
        dev_list_MutexUnlock();
        return GL_ERR_MSG;
    }

	// *mac = node->ble_dev_desc.dev_addr;
    memcpy(mac, node->ble_dev_desc.dev_addr, MAC_STR_LEN);
    
    // release lock
    dev_list_MutexUnlock();
    
    return GL_SUCCESS;
}

uint16_t ble_dev_mgr_get_connection(char *dev_addr, int* connection) {

    ble_dev_mgr_node_t *node = NULL;
    if (dev_addr == NULL) {
        log_err("Address is null");
        return GL_ERR_PARAM;
    }

    // get lock 
    dev_list_MutexLock();

    if (search_ble_dev_by_addr(dev_addr, &node) != 0) {
        log_err("The device is not in the list");
        dev_list_MutexUnlock();
        return GL_ERR_MSG;
    }

	*connection = node->ble_dev_desc.connection;

    // release lock
    dev_list_MutexUnlock();

    return GL_SUCCESS;
}

int ble_dev_mgr_get_list_size(void) {
    // get lock 
    dev_list_MutexLock();

    int index = 0;
    ble_dev_mgr_ctx_t *ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *node = NULL;

    list_for_each_entry(node, &ctx->dev_list, linked_list) { index++; }

    // release lock
    dev_list_MutexUnlock();

    return index;
}

int ble_dev_mgr_update(uint16_t connection) {
    // get lock 
    dev_list_MutexLock();
    
    ble_dev_mgr_node_t *node = NULL;

    if (search_ble_dev_by_connection(connection, &node) != 0) {
        dev_list_MutexUnlock();
        return -1;
    }
    node->ble_dev_desc.connection = connection;

    // release lock
    dev_list_MutexUnlock();

    return GL_SUCCESS;
}

int ble_dev_mgr_del_all(void)
{
    // get lock 
    dev_list_MutexLock();

    ble_dev_mgr_ctx_t *mgr_ctx = _ble_dev_mgr_get_ctx();
    ble_dev_mgr_node_t *node = NULL;
    ble_dev_mgr_node_t *next_node = NULL;
    
    list_for_each_entry_safe(node, next_node, &mgr_ctx->dev_list, linked_list) {
        log_debug("Del node: %s, connection=%d\n", node->ble_dev_desc.dev_addr, node->ble_dev_desc.connection);
        list_del(&node->linked_list);
        free(node);
    }

    // release lock
    dev_list_MutexUnlock();

    return GL_SUCCESS;
}