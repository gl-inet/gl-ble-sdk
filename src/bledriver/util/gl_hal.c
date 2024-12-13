/*****************************************************************************
 * @file  hal.c
 * @brief Hardware interface adaptation
 *******************************************************************************
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
#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include <unistd.h>   
#include <fcntl.h>   
#include "gl_uart.h"
#include "gl_log.h"
#include "gl_hal.h"
#include "gl_errno.h"

#define UCI_PACKAGE "ble"

unsigned char ENDIAN;

char rston[64] = {0};
char rstoff[64] = {0};

hw_cfg_t* ble_hw_cfg = NULL;
static bool is_ble_rst_exist = false;

static int check_endian(void);
static int serial_init(void);
static GL_RET get_model_hw_cfg(void);
static GL_RET normal_check_rst_io(void);
static GL_RET sdk_check_ver(void);

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

	if(access("/sys/class/gpio/ble_rst", F_OK) == 0)
	{
		is_ble_rst_exist = true;
		log_debug("Ble rst io exist.\n");
		return GL_SUCCESS;
	}

	char io[32] = {0};
	sprintf(io, "/sys/class/gpio/gpio%d", ble_hw_cfg->rst_gpio);
	log_debug("%s\n", io);

	char create_io[64] = {0};   
	sprintf(create_io, "((echo %d > /sys/class/gpio/export) 2>/dev/null)", ble_hw_cfg->rst_gpio);
	log_debug("%s\n", create_io);

	char create_io_direction[80] = {0};   
	sprintf(create_io_direction, "((echo out > /sys/class/gpio/gpio%d/direction) 2>/dev/null)", ble_hw_cfg->rst_gpio);
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

/* Check openwrt version
	SDK in official openwrt will have a special io base num.
	If "/sys/class/gpio/gpiochip412" exist, all io shoule add 412.
	If "/sys/class/gpio/gpiochip455" exist, all io shoule add 455.
	If "/sys/class/gpio/gpiochip512" exist, all io shoule add 512.
*/
#define SPECIAL_CHIP_IO412 		"/sys/class/gpio/gpiochip412"
#define SPECIAL_CHIP_IO455 		"/sys/class/gpio/gpiochip455"
#define SPECIAL_CHIP_IO512 		"/sys/class/gpio/gpiochip512"
static GL_RET sdk_check_ver(void)
{
	if(!ble_hw_cfg)
	{
		log_err("HW cfg lost!\n");
		return GL_UNKNOW_ERR;
	}

	if((access(SPECIAL_CHIP_IO412, F_OK)) != -1)
	{
		log_debug("SDK gpiochip412 exist.\n");
		ble_hw_cfg->rst_gpio += 412;
	}
	else if((access(SPECIAL_CHIP_IO455, F_OK)) != -1)
	{
		log_debug("SDK gpiochip455 exist.\n");
		ble_hw_cfg->rst_gpio += 455;
	}
	else if((access(SPECIAL_CHIP_IO512, F_OK)) != -1)
	{
		log_debug("SDK gpiochip512 exist.\n");
		ble_hw_cfg->rst_gpio += 512;
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

	if(is_ble_rst_exist)
	{
		if(ble_hw_cfg->rst_trigger == 1)
		{
			strcpy(rston, "((echo 1 > /sys/class/gpio/ble_rst/value) 2>/dev/null)");
			strcpy(rstoff, "((echo 0 > /sys/class/gpio/ble_rst/value) 2>/dev/null)");

		}else if(ble_hw_cfg->rst_trigger == 0){
			strcpy(rston, "((echo 0 > /sys/class/gpio/ble_rst/value) 2>/dev/null)");
			strcpy(rstoff, "((echo 1 > /sys/class/gpio/ble_rst/value) 2>/dev/null)");

		}else{
			log_err("hw rst trigger cfg error!\n");
			return GL_UNKNOW_ERR;
		}
	}
	else
	{
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
	}

    return uartOpen((int8_t*)ble_hw_cfg->port, ble_hw_cfg->baudRate, ble_hw_cfg->flowcontrol, 100);
}

static GL_RET get_model_hw_cfg(void)
{
	char uart_baudrate[16] = {0};
	char flowcontrol[8] = {0};
	char rst_trigger[8] = {0};
	char gpio_rst[8] = {0};
	char gpio_dfu[8] = {0};
	char uart_tty[32] = {0};

    struct uci_context* ctx = guci2_init();
	if(!ctx)
	{
		log_err("open uci handle error\n");
		return GL_UNKNOW_ERR;
	}

    if(guci2_get(ctx,"uart_baudrate",uart_baudrate) < 0)
    {
		log_err("Get hw uart_baudrate error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get uart_baudrate: %s\n", uart_baudrate);
	}

	if(guci2_get(ctx,"uart_flowcontrol",flowcontrol) < 0)
    {
		log_err("Get hw flowcontrol error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get flowcontrol: %s\n", flowcontrol);
	}

	if(guci2_get(ctx,"rst_trigger",rst_trigger) < 0)
    {
		log_err("Get hw rst_trigger error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get rst_trigger: %s\n", rst_trigger);
	}

	if(guci2_get(ctx,"gpio_rst",gpio_rst) < 0)
    {
		log_err("Get hw gpio_rst error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get gpio_rst: %s\n", gpio_rst);
	}

	if(guci2_get(ctx,"gpio_dfu",gpio_dfu) < 0)
    {
		log_err("Get hw gpio_dfu error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get gpio_dfu: %s\n", gpio_dfu);
	}

	if(guci2_get(ctx,"uart_tty",uart_tty) < 0)
    {
		log_err("Get hw uart_tty error!\n");
		return GL_UNKNOW_ERR;
	}else{
		log_debug("Get uart_tty: %s\n", uart_tty);
	}

	guci2_free(ctx);

	ble_hw_cfg = malloc(sizeof(hw_cfg_t));
	ble_hw_cfg->baudRate = atoi(uart_baudrate);
	ble_hw_cfg->flowcontrol = atoi(flowcontrol);
	ble_hw_cfg->rst_trigger = atoi(rst_trigger);
	ble_hw_cfg->rst_gpio = atoi(gpio_rst);
	ble_hw_cfg->dfu_gpio = atoi(gpio_dfu);
	memcpy(ble_hw_cfg->port, uart_tty, sizeof(uart_tty));
	sdk_check_ver();

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

int guci2_get(struct uci_context* ctx, const char* section_or_key, char out_value[])
{
	struct uci_package *pkg;
    struct uci_section *sec;
    const char *value = NULL;
	int ret = UCI_OK;

    if (uci_load(ctx, UCI_PACKAGE, &pkg)) {
        return -1;
    }

    sec = uci_lookup_section(ctx, pkg, UCI_PACKAGE);
    if (!sec) {
		uci_unload(ctx, pkg);
        return -1;
    }

    value = uci_lookup_option_string(ctx, sec, section_or_key);
    if (value) {
		strcpy(out_value, value);
    } else {
        fprintf(stderr, "Failed to get option value\n");
    }

	uci_unload(ctx, pkg);
	return ret;
}