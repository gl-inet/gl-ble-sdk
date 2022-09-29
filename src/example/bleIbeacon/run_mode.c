#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <json-c/json.h>

#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"
#include "ibeacon.h"
#include "ubus.h"
#include "run_mode.h"
#include "util.h"

static int scan_type = 0;
static int scan_respone = 0;
static int tx_power = -1;

int mode_check(int argc)
{
    if(argc == 1)
	{
		printf("\nbackground mode\n");
		return BACKGROUNG_MODE;
	}
    else
    {
        printf("\nforeground mode\n");
        return FOREGROUND_MODE;
    }
    return -1;
}

int foreground_param_check(int argc, char *argv[])
{
	if(!strcmp(argv[1], IBEACON_RECIEVER))
	{
		if (argc != 3) 
		{
			printf("param err!\n");
			return GL_ERR_PARAM;
		}
        // passive/active scan check
        if(!strcmp(argv[2], IBEACON_PASSIVE_SCAN))
        {
            scan_type = 0;
            printf("passive scan\n");
        }	
        else if(!strcmp(argv[2], IBEACON_ACTIVE_SCAN))
        {
            scan_type = 1;
            printf("active scan\n");
        }
        else
        {
            printf("param err!\n");
            return GL_ERR_PARAM;
        }
    }
	else if(!strcmp(argv[1], IBEACON_SENDER))
	{
		if ((argc != 7) && (argc != 8))
		{
			printf("param err!\n");
			return GL_ERR_PARAM;
		}
		//ibeacon payload: uuid、major、minor、Measured Power
		if ((strlen(argv[3]) != 32) || (strlen(argv[4]) != 4) || 
				(strlen(argv[5]) != 4) || (strlen(argv[6]) != 2))
		{
			printf("ibeacon payload param err!\n");
			return GL_ERR_PARAM;
		}
		//scan_respone 
		if(argc == 7)
		{
			if(!strcmp(argv[2], IBEACON_SCAN_RSP_CLOSE))
			{
				scan_respone = 0;
			}
			else if(!strcmp(argv[2], IBEACON_SCAN_RSP_OPEN))
			{
				scan_respone = 1;
			}
			else
			{
				printf("scan respon param err!\n");
				return GL_ERR_PARAM;
			}
		}
		//tx power
		if(argc == 8)
		{	
			tx_power = atoi(argv[7]);
		}
	}
	else
	{
		printf("param err!\n");
		return GL_ERR_PARAM;
	}
	return GL_SUCCESS;
}

int foreground(char *argv[])
{   
    if(!strcmp(argv[1], IBEACON_RECIEVER))
	{
        return foreground_ibeacon_reciever();
    }
    else if(!strcmp(argv[1], IBEACON_SENDER))
	{
        return foreground_ibeacon_sender(argv);
    }
}

int foreground_ibeacon_reciever(void)
{
    GL_RET ret = -1;
    json_object *o = NULL;
    const char *temp = NULL;

    // start scan
    ret = gl_ble_discovery(PHYS, INTERVAL, WINDOW, scan_type, MODE);
    if (ret != GL_SUCCESS)
    {
        printf("Start ble discovery error!! Err code: %d\n", ret);
        return GL_UNKNOW_ERR;
    }

    while(1)
    {
        o = get_ibeacon_vendor();
        if(o == NULL)
        {
            continue;
        }
        temp = json_object_to_json_string(o);

        printf("%s\n", temp);
    }
    return GL_SUCCESS;
}

int foreground_ibeacon_sender(char *argv[])
{
    GL_RET ret = -1;
    int current_p = -1;
    char ibeacon_packet_snd[IBEACON_PACKET_SIZE*2 + 1] = {0};
    sprintf(ibeacon_packet_snd, "%s%s%s%s%s", IBEACON_SEND_HEADER, argv[3], argv[4], argv[5], argv[6]);
    printf("data: %s\n", ibeacon_packet_snd);
	//tx power
	if(tx_power != -1)
	{
		ret = gl_ble_set_power(tx_power, &current_p);
		json_object* o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "code", json_object_new_int(ret));
		if(ret == GL_SUCCESS)
		{
			json_object_object_add(o, "current_power", json_object_new_int(current_p));
		}
		const char *temp = json_object_to_json_string(o);
		printf("%s\n",temp);
		
		json_object_put(o);

		tx_power = -1;
	} 
    
    ret = gl_ble_adv_data(0, ibeacon_packet_snd);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_adv_data failed: %d\n", ret);
        return GL_UNKNOW_ERR;
    }
	if(scan_respone == 1)//exist scan 
	{
		ret = gl_ble_adv_data(1, ibeacon_packet_snd);
		if (GL_SUCCESS != ret)
		{
			printf("gl_ble_adv_data failed: %d\n", ret);
			return GL_UNKNOW_ERR;
		}
	}
    // start advertising
    ret = gl_ble_adv(PHYS, INTERVAL_MIN, INTERVAL_MAX, DISCOVER, ADV_CONN);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_adv failed: %d\n", ret);
        return GL_UNKNOW_ERR;
    }

    //get ble mac
    uint8_t mac_buf[6];
    ret = gl_ble_get_mac(mac_buf);
    if (GL_SUCCESS != ret)
    {
        printf("gl_ble_get_mac failed: %d\n", ret);
        return GL_UNKNOW_ERR;
    }
    printf("Broadcasting ibeacon data, the mac of the current device is > " MACSTR "\n", MAC2STR(mac_buf));
    
    while(1)
    {
        sleep(1000);
    }

    return GL_SUCCESS;
}

int background(void)
{
    return ubus_start();
}