/*****************************************************************************
 * @file  hal.c
 * @brief Hardware interface adaptation
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
#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include <unistd.h>   
#include <fcntl.h>   
#include "gl_uart.h"
#include "gl_log.h"
#include "gl_hal.h"
#include "gl_hw_cfg.h"
#include "gl_errno.h"


#define MODEL_FILE_PATH		"/proc/gl-hw-info/model"

unsigned char ENDIAN;

char rston[64] = {0};
char rstoff[64] = {0};

char model[48] = {0};
hw_cfg_t* ble_hw_cfg = NULL;

static int check_endian(void);
static int serial_init(void);
static GL_RET get_model_hw_cfg(void);
static GL_RET normal_check_rst_io(void);
static GL_RET qsdk_check_ver(void);

struct uci_context* guci2_init(void);
int guci2_free(struct uci_context* ctx);
int guci2_get(struct uci_context* ctx, const char* section_or_key, char value[]);


static int check_endian(void)
{
  int x = 1;
  if(*((char *)&x) == 1) 
  {
	ENDIAN = 0;   
	log_debug("little endian\n");
  }else{
	ENDIAN = 1;   
	log_debug("big endian\n");
  }

  return 0;
}

static GL_RET normal_check_rst_io(void)
{
	if(!ble_hw_cfg)
	{
		log_err("HW cfg lost!\n");
		return GL_UNKNOW_ERR;
	}

	char io[32] = {0};
	sprintf(io, "/sys/class/gpio/gpio%d", ble_hw_cfg->rst_gpio);
	log_debug("%s\n", io);

	char create_io[64] = {0};   
	sprintf(create_io, "echo %d > /sys/class/gpio/export", ble_hw_cfg->rst_gpio);
	log_debug("%s\n", create_io);

	char create_io_direction[80] = {0};   
	sprintf(create_io_direction, "echo out > /sys/class/gpio/gpio%d/direction", ble_hw_cfg->rst_gpio);
	log_debug("%s\n", create_io_direction);


	// create IO
	int i = 0;
	while((access(io, F_OK)) != 0)
	{
		log_debug("Ble rst io: %s not exist. Now trying create ... Time: %d\n", io, i+1);   
		system(create_io);

		i++;
		usleep(300000);

		if(i > 5)
		{   
			log_err("Creating ble RST IO failed!\n");
			return GL_UNKNOW_ERR; 
		}
	}

	// set IO direction
	system(create_io_direction);

	log_debug("Ble rst io: %s exist.\n", io);

	return GL_SUCCESS;
}

/* Check special openwrt version
	QSDK in open source openwrt will have a special io base num.
	If "/sys/class/gpio/gpiochip412" exist, all io shoule add 412.
*/
#define SPECIAL_CHIP_IO 		"/sys/class/gpio/gpiochip412"
static GL_RET qsdk_check_ver(void)
{
	if(!ble_hw_cfg)
	{
		log_err("HW cfg lost!\n");
		return GL_UNKNOW_ERR;
	}

	if((access(SPECIAL_CHIP_IO, F_OK)) != -1)
	{
		log_debug("QSDK gpiochip412 exist.\n");
		ble_hw_cfg->rst_gpio += 412;
	}

	return GL_SUCCESS;
}

static int serial_init(void)
{
	if(!ble_hw_cfg)
	{
		log_err("HW cfg lost!\n");
		return GL_UNKNOW_ERR;
	}

	if(ble_hw_cfg->rst_trigger == 1)
	{
		sprintf(rston, "echo 1 > /sys/class/gpio/gpio%d/value", ble_hw_cfg->rst_gpio);
		sprintf(rstoff, "echo 0 > /sys/class/gpio/gpio%d/value", ble_hw_cfg->rst_gpio);

	}else if(ble_hw_cfg->rst_trigger == 0){
		sprintf(rston, "echo 0 > /sys/class/gpio/gpio%d/value", ble_hw_cfg->rst_gpio);
		sprintf(rstoff, "echo 1 > /sys/class/gpio/gpio%d/value", ble_hw_cfg->rst_gpio);

	}else{
		log_err("hw rst trigger cfg error!\n");
		return GL_UNKNOW_ERR;
	}

    return uartOpen((int8_t*)ble_hw_cfg->port, ble_hw_cfg->baudRate, ble_hw_cfg->flowcontrol, 100);
}

static GL_RET get_model_hw_cfg(void)
{
    struct uci_context* ctx = guci2_init();
	if(!ctx)
	{
		log_err("open uci handle error\n");
		return GL_UNKNOW_ERR;
	}

    if(guci2_get(ctx,"glconfig.general.model",model) < 0)
    {
		guci2_free(ctx);

		// sdk4.0 chang model file
		FILE *model_file = NULL;
		model_file = fopen(MODEL_FILE_PATH, "r");
		if(model_file == NULL)
		{
			log_info("open MODEL_FILE_PATH error. Maybe it's a official openwrt firmware?\n");
			ble_hw_cfg = &B2200_BLE_HW_CFG;
			qsdk_check_ver();
			normal_check_rst_io();
			return 1;
		}
		size_t model_name_len = 0;
		// // get model name size
		// if (fseek(model_file, 0L, SEEK_END)) 
		// {
		// 	log_err("fseek MODEL_FILE_PATH error\n");
		// 	return 1;
		// }
		// model_name_len = ftell(model_file);
		// if (fseek(model_file, 0L, SEEK_SET)) 
		// {
		// 	log_err("fseek MODEL_FILE_PATH error\n");
		// 	return 1;
		// }

		// if (fread(model, 1, model_name_len, model_file) != model_name_len) 
		// {
		// 	printf("File read failure\n");
		// 	return 1;
		// }
		model_name_len = fread(model, 1, 24, model_file);
		if(model_name_len <= 0)
		{
			log_err("File read failure\n");
			fclose(model_file);
			return 1;
		}else{
			printf("File read[%d]: %s\n", model_name_len, model);
		}
    }else{
		guci2_free(ctx);
	}


	log_debug("Get model: %s\n", model);

	if(0 == strcmp(model, "s1300"))
	{
		ble_hw_cfg = &S1300_BLE_HW_CFG;
		qsdk_check_ver();

	}else if(0 == strcmp(model, "x750")){
		ble_hw_cfg = &X750_BLE_HW_CFG;
		// mark kernel log
		system("echo 1 4 1 7 > /proc/sys/kernel/printk");

	}else if(0 == strcmp(model, "xe300")){
		ble_hw_cfg = &XE300_BLE_HW_CFG;
		// mark kernel log
		system("echo 1 4 1 7 > /proc/sys/kernel/printk");

	}else if(0 == strcmp(model, "mt300n-v2")){
		ble_hw_cfg = &MT300N_V2_BLE_HW_CFG; 

	}else if(0 == strcmp(model, "x300b")){
		ble_hw_cfg = &X300B_BLE_HW_CFG;
		// mark kernel log
		system("echo 1 4 1 7 > /proc/sys/kernel/printk");

	}else if(0 == strcmp(model, "ap1300")){
		ble_hw_cfg = &AP1300_BLE_HW_CFG;
		qsdk_check_ver();

	}else if(0 == strcmp(model, "b2200")){
		ble_hw_cfg = &B2200_BLE_HW_CFG;
		qsdk_check_ver();

	}else if(0 == strcmp(model, "e750")){
		ble_hw_cfg = &E750_BLE_HW_CFG;

	}else if(0 == strncmp(model, "s200", strlen("s200"))){
		ble_hw_cfg = &S200_BLE_HW_CFG;
		// mark kernel log
		system("echo 1 4 1 7 > /proc/sys/kernel/printk");
	}else{
		log_err("Unknow model!\n");
		return GL_UNKNOW_ERR;
	}

	normal_check_rst_io();

	return GL_SUCCESS;
}

int hal_init(void)
{
    check_endian();

	get_model_hw_cfg();

	// Manual clean uart cache: fix tcflush() not work on some model
	// uartCacheClean();
	

    int serialFd = serial_init();
    if( serialFd < 0 )
    {
        fprintf(stderr,"Hal initilized failed.\n");
        return GL_UNKNOW_ERR;
    }

    return serialFd;
}

int hal_destroy(void)
{
	return uartClose();
}


struct uci_context* guci2_init(void)
{
	
	struct uci_context* ctx = uci_alloc_context();
		
	return ctx;
}

int guci2_free(struct uci_context* ctx)
{
	uci_free_context(ctx);
	return 0;
}

static const char *delimiter = " ";

static void uci_show_value(struct uci_option *o, char value[]){
	struct uci_element *e;
	bool sep = false;
//	char *space;

	switch(o->type) {
	case UCI_TYPE_STRING:
		sprintf(value,"%s", o->v.string);
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			sprintf(value,"%s", (sep ? delimiter : ""));
			//space = strpbrk(e->name, " \t\r\n");
			//if (!space )
				sprintf(value,"%s", e->name);
			//sep = true;
		}
		break;
	default:
		strcpy(value,"");
		break;
	}
}

int guci2_get(struct uci_context* ctx, const char* section_or_key, char value[])
{
	struct uci_ptr ptr;
	struct uci_element *e;
	int ret = UCI_OK;
	char *str=(char*)malloc(strlen(section_or_key)+1); //must not use a const value
	strcpy(str,section_or_key);
	if (uci_lookup_ptr(ctx, &ptr, str, true) != UCI_OK) {
		ret=-1;
		strcpy(value,"");
		goto out;
	}
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE)) {
		ctx->err = UCI_ERR_NOTFOUND;
		ret=-1;
		strcpy(value,"");
		goto out;
	}
	e = ptr.last;
	switch(e->type) {
	case UCI_TYPE_SECTION:
		sprintf(value,"%s", ptr.s->type);
		break;
	case UCI_TYPE_OPTION:
		uci_show_value(ptr.o, value);
		break;
	default:
		strcpy(value,"");
		ret=-1;
		goto out;
		break;
	}
out:
	free(str);
	return ret;
}