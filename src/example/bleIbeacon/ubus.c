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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <json-c/json.h>
#include <pthread.h>
#include "ibeacon.h"
#include "ubus.h"
#include "util.h"

#define UBUS_EVENT "ibeacon_data"

extern ibeacon_ringArray_t *ibeacon_array;
extern uint8_t adv_handle;

struct ubus_context *_ctx_invoke = NULL;
struct ubus_context *_ctx_event = NULL;
static const char *_sock_path_invoke;
static const char *_sock_path_event;
pthread_t ubus_tid;

static struct blob_buf b;
struct ubus_request_data *reqbuf;
static json_object* output = NULL;
static uint8_t scan_type = 0;
static int scan_respone = 0;

static void *ibeacon_data_upload_start(void *arg);


/*IBEACON_RECIEVE_MODE*/
enum
{
	IBEACON_RECIEVE_SCAN_TYPE,
    IBEACON_RECIEVE_STOP,
    IBEACON_RECIEVE_MAX,
};

static const struct blobmsg_policy recieve_policy[IBEACON_RECIEVE_MAX] = {
	[IBEACON_RECIEVE_SCAN_TYPE] = {.name = "scan_type", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_RECIEVE_STOP] = {.name = "stop_scan", .type = BLOBMSG_TYPE_STRING},
};

/* A callback function for ubus methods handling */
static int recieve(struct ubus_context *_ctx_invoke, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    printf("\rrecieve mode start!\n");
    gl_ble_stop_discovery();
	gl_ble_stop_adv(adv_handle);

    if(ibeacon_array)
    {
        free(ibeacon_array);
    }
    ibeacon_array = calloc(1, sizeof(ibeacon_ringArray_t));
    

    GL_RET ret = -1;

    /* for parsed attr */
	struct blob_attr *tb[IBEACON_RECIEVE_MAX];
	blobmsg_parse(recieve_policy, IBEACON_RECIEVE_MAX, tb, blob_data(msg), blob_len(msg));

    if(blobmsg_get_string(tb[IBEACON_RECIEVE_STOP]))
    {
        if(!strcmp(blobmsg_get_string(tb[IBEACON_RECIEVE_STOP]), "true"))
        {
            goto stop;
        }
        // else
        // {
        //     printf("param error\n");
        //     goto exit;
        // }
    }

    if(blobmsg_get_string(tb[IBEACON_RECIEVE_SCAN_TYPE]))
    {
        //param check
        if(!strcmp(blobmsg_get_string(tb[IBEACON_RECIEVE_SCAN_TYPE]), IBEACON_PASSIVE_SCAN))
        {
            printf("passive scan\n");
            scan_type = 0;
        }	
        else if(!strcmp(blobmsg_get_string(tb[IBEACON_RECIEVE_SCAN_TYPE]), IBEACON_ACTIVE_SCAN))
        {
            printf("active scan\n");
            scan_type = 1;
        }
        else
        {
            printf("scan type param error\n");
            goto exit;
        }
    }
    else
    {
        printf("recieve policy not match\n");
        goto exit;
    }

    ret = gl_ble_start_discovery(PHYS, INTERVAL, WINDOW, scan_type, MODE);
    if (ret != GL_SUCCESS)
    {
        printf("Start ble discovery error!! Err code: %d\n", ret);
        goto exit;
    }

stop:
    blob_buf_init(&b, 0);
    output = json_object_new_object();
    json_object_object_add(output, "code", json_object_new_int(GL_SUCCESS));
    blobmsg_add_object(&b, output);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(_ctx_invoke, req, b.head);

	return GL_SUCCESS;

exit:
    blob_buf_init(&b, 0);
    output = json_object_new_object();
    json_object_object_add(output, "code", json_object_new_int(GL_UNKNOW_ERR));
    blobmsg_add_object(&b, output);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(_ctx_invoke, req, b.head);

    return GL_UNKNOW_ERR;
}

/*IBEACON_SEND_MODE*/
enum
{
	IBEACON_SEND_RSP_TYPE,
    IBEACON_SEND_UUID,
    IBEACON_SEND_MAJOR,
    IBEACON_SEND_MINOR,
    IBEACON_SEND_MEASURED_POWER,
    IBEACON_SEND_TX_POWER,
    IBEACON_SEND_STOP,
    IBEACON_SEND_MAX,
};

static const struct blobmsg_policy send_policy[IBEACON_SEND_MAX] = {
	[IBEACON_SEND_RSP_TYPE] = {.name = "rsp_type", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_UUID] = {.name = "uuid", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MAJOR] = {.name = "major", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MINOR] = {.name = "minor", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MEASURED_POWER] = {.name = "measured_power", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_STOP] = {.name = "stop_send", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_TX_POWER] = {.name = "tx_power", .type = BLOBMSG_TYPE_STRING},
};

/* A callback function for ubus methods handling */
static int send(struct ubus_context *_ctx_invoke, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    // pthread_mutex_lock(&mutex);
    printf("\nsend mode start!\n");
	gl_ble_stop_discovery();
    gl_ble_stop_adv(adv_handle);

    if(ibeacon_array)
    {
        free(ibeacon_array);
    }
    ibeacon_array = calloc(1, sizeof(ibeacon_ringArray_t));

    GL_RET ret = -1;
    int tx_power = -1;
    int current_p = -1;
    char ibeacon_packet_snd[IBEACON_PACKET_SIZE*2 + 1] = {0};
    
	/* for parsed attr */
	struct blob_attr *tb[IBEACON_SEND_MAX];
    blobmsg_parse(send_policy, IBEACON_SEND_MAX, tb, blob_data(msg), blob_len(msg)); 

    if(blobmsg_get_string(tb[IBEACON_SEND_STOP]))
    {
        if(!strcmp(blobmsg_get_string(tb[IBEACON_SEND_STOP]), "true"))
        {
            goto stop;
        }
        // else
        // {
        //     printf("param error\n");
        //     goto exit;
        // }
    }
    
    if(blobmsg_get_string(tb[IBEACON_SEND_RSP_TYPE])
        && blobmsg_get_string(tb[IBEACON_SEND_UUID]) && blobmsg_get_string(tb[IBEACON_SEND_MAJOR]) 
        && blobmsg_get_string(tb[IBEACON_SEND_MINOR]) && blobmsg_get_string(tb[IBEACON_SEND_MEASURED_POWER]))
    {      
			if(!strcmp(blobmsg_get_string(tb[IBEACON_SEND_RSP_TYPE]), IBEACON_SCAN_RSP_CLOSE))
			{
                printf("scan_respone close\n");
				scan_respone = 0;
			}
			else if(!strcmp(blobmsg_get_string(tb[IBEACON_SEND_RSP_TYPE]), IBEACON_SCAN_RSP_OPEN))
			{
                printf("scan_respone open\n");
				scan_respone = 1;
			}
			else
			{
				printf("scan respon param err!\n");
				goto exit;
			}
		
            if(strlen(blobmsg_get_string(tb[IBEACON_SEND_UUID])) != 32 || strlen(blobmsg_get_string(tb[IBEACON_SEND_MAJOR])) != 4
                || strlen(blobmsg_get_string(tb[IBEACON_SEND_MINOR])) != 4 || strlen(blobmsg_get_string(tb[IBEACON_SEND_MEASURED_POWER])) != 2)
            {
                printf("ibeacon payload param err!\n");
			    goto exit;
            }

            sprintf(ibeacon_packet_snd, "%s%s%s%s%s", IBEACON_SEND_HEADER, 
                    blobmsg_get_string(tb[IBEACON_SEND_UUID]), 
                    blobmsg_get_string(tb[IBEACON_SEND_MAJOR]), 
                    blobmsg_get_string(tb[IBEACON_SEND_MINOR]),
                    blobmsg_get_string(tb[IBEACON_SEND_MEASURED_POWER]));
    }
    else
    {
        printf("send policy not match\n");
        goto exit;
    }

    if(blobmsg_get_string(tb[IBEACON_SEND_TX_POWER]))
    {
        tx_power = atoi(blobmsg_get_string(tb[IBEACON_SEND_TX_POWER]));

        ret = gl_ble_set_power(tx_power, &current_p);
		output = json_object_new_object();
		if(ret == GL_SUCCESS)
		{
			json_object_object_add(output, "current_power", json_object_new_int(current_p));
		}
		const char *temp = json_object_to_json_string(output);
		printf("%s\n",temp);
    }
    else
    {
        printf("tx power null\n");
    }

    // init adv param
    uint32_t interval_min = 32;
    uint32_t interval_max = 64;
    uint8_t discover = 3;
    uint8_t connect = 3;

    // create adv handle
	ret = gl_ble_create_adv_handle(&adv_handle);
	if (GL_SUCCESS != ret)
	{
		printf("gl_ble_create_adv_handle failed: %d\n", ret);
		exit(-1);
	}
    
    ret = gl_ble_set_legacy_adv_data(adv_handle, 0, ibeacon_packet_snd);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_set_legacy_adv_data failed: %d\n", ret);
        goto exit;
    }
    if (scan_respone == 1)
    {

        ret = gl_ble_set_legacy_adv_data(adv_handle, 1, ibeacon_packet_snd);
        if (GL_SUCCESS != ret)
        {
            printf("gl_ble_set_legacy_adv_data failed: %d\n", ret);
            goto exit;
        }
    }

    // start advertising
    ret = gl_ble_start_legacy_adv(adv_handle, interval_min, interval_max, discover, connect);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_start_legacy_adv failed: %d\n", ret);
        goto exit;
    }

    //get ble mac
    uint8_t mac_buf[6];
    ret = gl_ble_get_mac(mac_buf);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_get_mac failed: %d\n", ret);
        goto exit;

    }
    printf("Broadcasting ibeacon data, the mac of the current device is > " MACSTR "\n", MAC2STR(mac_buf));

stop:
	/* send a reply msg to the caller for information */
    if(output == NULL)
    {
        output = json_object_new_object();
    }
    blob_buf_init(&b, 0);
	json_object_object_add(output, "code", json_object_new_int(GL_SUCCESS));
    blobmsg_add_object(&b, output);
	json_object_put(output);
    output = NULL;
	ubus_send_reply(_ctx_invoke, req, b.head);
	return GL_SUCCESS;

exit:
    blob_buf_init(&b, 0);
    output = json_object_new_object();
    json_object_object_add(output, "code", json_object_new_int(GL_UNKNOW_ERR));
    blobmsg_add_object(&b, output);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(_ctx_invoke, req, b.head);
    return GL_UNKNOW_ERR;
}

/* ubus methods */
static struct ubus_method ibeacon_methods[] = 
{
	UBUS_METHOD("recieve", recieve, recieve_policy),
	UBUS_METHOD("send", send, send_policy),
	
};

/* ubus object type */
static struct ubus_object_type ibeacon_obj_type = UBUS_OBJECT_TYPE("ibeacon", ibeacon_methods);

/* ubus object assignment */
struct ubus_object ibeacon_obj = 
{
	.name = "ibeacon",
	.type = &ibeacon_obj_type,
	.methods = ibeacon_methods,
	.n_methods = ARRAY_SIZE(ibeacon_methods),
};

//ubus event reconnect
static void event_reconn_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry =
	{
		.cb = event_reconn_timer,
	};
	int t = 2;

	if (ubus_reconnect(_ctx_event, _sock_path_event) != 0) {
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

}
 
static void event_connection_lost(struct ubus_context *ctx)
{
	event_reconn_timer(NULL);
}

//ubus invoke reconnect
static void invoke_reconn_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry =
	{
		.cb = invoke_reconn_timer,
	};
	int t = 2;
	
	if (ubus_reconnect(_ctx_invoke, _sock_path_invoke) != 0) {
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

}
 
static void invoke_connection_lost(struct ubus_context *ctx)
{
	invoke_reconn_timer(NULL);
}

//--------------------------ubus invoke init---------------------------------
GL_RET ubus_invoke_init(void)
{
	char *path_invoke = NULL;
	uloop_init();

	_sock_path_invoke = path_invoke;

	_ctx_invoke = ubus_connect(path_invoke);

	if (!_ctx_invoke)
	{
		printf("ubus connect failed\n");
		return GL_UNKNOW_ERR;
	}
	
	_ctx_invoke->connection_lost = invoke_connection_lost;
 
	ubus_add_uloop(_ctx_invoke);
 
	/* 向ubusd注册对象和方法，供其他ubus客户端调用。 */
	if (ubus_add_object(_ctx_invoke, &ibeacon_obj) != 0)
	{
		printf("ubus add obj failed\n");
		return GL_UNKNOW_ERR;
	}

	uloop_run();
	return GL_SUCCESS;
}



//---------------------------ubus event init---------------------------------
GL_RET ubus_event_init(void)
{
	char *path_event = NULL;
	_sock_path_event = path_event;
	_ctx_event = ubus_connect(path_event);
	if (!_ctx_event)
	{
		printf("ubus connect failed\n");
		return GL_UNKNOW_ERR;
	}

	_ctx_event->connection_lost = event_connection_lost;

	int error = pthread_create(&ubus_tid, NULL, ibeacon_data_upload_start, _ctx_event);
    if (error) 
	{
		printf("pthread_create_adv_data_upload_start_failed: %s\n", strerror(error));
		return GL_UNKNOW_ERR;
    }
	return GL_SUCCESS;
	
}

int ubus_start(void)
{
    GL_RET ret;
    ret = ubus_event_init();
    if(GL_SUCCESS != ret)
    {
        return ret;
    }
	ret = ubus_invoke_init();
    return ret;
}

static void *ibeacon_data_upload_start(void *arg)
{	
    struct ubus_context *ctx = (struct ubus_context *)arg;
    static struct blob_buf _b;
	json_object *o = NULL;

    while(1)
    {
        o = get_ibeacon_vendor();
        if(o == NULL)
        {
            continue;
        }
        blob_buf_init(&_b, 0);
        blobmsg_add_object(&_b, o);

        json_object_put(o);
        o = NULL;

        ubus_send_event(ctx, UBUS_EVENT, _b.head);
    }
    return;
}