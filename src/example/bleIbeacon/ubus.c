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
#include "ibeacon.h"
#include "ubus.h"
#include "util.h"

extern ibeacon_ringArray_t *ibeacon_array;

struct ubus_context *ctx = NULL;
static const char* sock_path = NULL;

static struct blob_buf b;
struct ubus_request_data *reqbuf;
static json_object* output = NULL;
static int scan_type = 0;
static int scan_respone = 0;

/*IBEACON_RECIEVE_MODE*/
enum
{
	IBEACON_RECIEVE_SCAN_TYPE,
    IBEACON_RECIEVE_MAX,
};

static const struct blobmsg_policy recieve_policy[IBEACON_RECIEVE_MAX] = {
	[IBEACON_RECIEVE_SCAN_TYPE] = {.name = "scan_type", .type = BLOBMSG_TYPE_STRING},
};

/* A callback function for ubus methods handling */
static int recieve(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    printf("\rrecieve mode start!\n");
    gl_ble_stop_discovery();
	gl_ble_stop_adv();

    if(ibeacon_array)
    {
        free(ibeacon_array);
    }
    ibeacon_array = calloc(1, sizeof(ibeacon_ringArray_t));
    

    GL_RET ret = -1;

    /* for parsed attr */
	struct blob_attr *tb[IBEACON_RECIEVE_MAX];
	blobmsg_parse(recieve_policy, IBEACON_RECIEVE_MAX, tb, blob_data(msg), blob_len(msg));

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

    

    ret = gl_ble_discovery(PHYS, INTERVAL, WINDOW, scan_type, MODE);
    if (ret != GL_SUCCESS)
    {
        printf("Start ble discovery error!! Err code: %d\n", ret);
        goto exit;
    }

    json_object *o = get_ibeacon_vendor();
    json_object_object_add(o, "code", json_object_new_int(GL_SUCCESS));
    blob_buf_init(&b, 0);
    blobmsg_add_object(&b, o);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(ctx, req, b.head);

    const char *temp = json_object_to_json_string(o);
    printf("%s\n", temp);

	return GL_SUCCESS;

exit:
    blob_buf_init(&b, 0);
    output = json_object_new_object();
    json_object_object_add(output, "code", json_object_new_int(GL_UNKNOW_ERR));
    blobmsg_add_object(&b, output);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(ctx, req, b.head);

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
    IBEACON_SEND_MAX,
};

static const struct blobmsg_policy send_policy[IBEACON_SEND_MAX] = {
	[IBEACON_SEND_RSP_TYPE] = {.name = "rsp_type", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_UUID] = {.name = "uuid", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MAJOR] = {.name = "major", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MINOR] = {.name = "minor", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_MEASURED_POWER] = {.name = "measured_power", .type = BLOBMSG_TYPE_STRING},
    [IBEACON_SEND_TX_POWER] = {.name = "tx_power", .type = BLOBMSG_TYPE_STRING},
};



/* A callback function for ubus methods handling */
static int send(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    // pthread_mutex_lock(&mutex);
    printf("\nsend mode start!\n");
	gl_ble_stop_discovery();
    gl_ble_stop_adv();

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
	//param check
    
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
    
    
    ret = gl_ble_adv_data(0, ibeacon_packet_snd);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_adv_data failed: %d\n", ret);
        goto exit;
    }
	if(scan_respone == 1)
	{
		ret = gl_ble_adv_data(1, ibeacon_packet_snd);
		if (GL_SUCCESS != ret)
		{
			printf("gl_ble_adv_data failed: %d\n", ret);
			goto exit;
		}
	}

    // start advertising
    ret = gl_ble_adv(PHYS, INTERVAL_MIN, INTERVAL_MAX, DISCOVER, ADV_CONN);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_adv failed: %d\n", ret);
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
	ubus_send_reply(ctx, req, b.head);
	return GL_SUCCESS;

exit:
    blob_buf_init(&b, 0);
    output = json_object_new_object();
    json_object_object_add(output, "code", json_object_new_int(GL_UNKNOW_ERR));
    blobmsg_add_object(&b, output);
    json_object_put(output);
    output = NULL;
    ubus_send_reply(ctx, req, b.head);
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

static void ubus_reconn_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout reconn_timer = {
		.cb = ubus_reconn_timer,
	};
	if(ubus_reconnect(ctx, sock_path) != 0){
		uloop_timeout_set(&reconn_timer, 1000);

	}else{
		ubus_add_uloop(ctx);
	}
}

static void ubus_connection_lost(struct ubus_context *ctx)
{
	printf("ubus_connection_lost\n");
	ubus_reconn_timer(NULL);
}



int ubus_start(void)
{
	uloop_init();

	ctx = ubus_connect(sock_path);
    if (!ctx) {
		fprintf(stderr,"ubus connect failed\n");
		exit(-1);
	}
	ctx->connection_lost = ubus_connection_lost;

    ubus_add_uloop(ctx);
    
	if (ubus_add_object(ctx, &ibeacon_obj) != 0)
	{
		fprintf(stderr,"ubus add obj failed\n");
		ubus_free(ctx);
	    uloop_done();
		return GL_UNKNOW_ERR;
	}

	uloop_run();
    return GL_SUCCESS;
}