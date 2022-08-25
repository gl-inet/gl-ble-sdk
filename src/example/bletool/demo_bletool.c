/*****************************************************************************
 * @file  demo_bletool.c
 * @brief CLI interface of BLE functions
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

#include <stdio.h>
#include <getopt.h>
#include <json-c/json.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "gl_bleapi.h"
#include "gl_errno.h"
#include "gl_type.h"

#define PARA_MISSING 	"Parameter missing\n"
#define PARA_ERROR 		"Parameter error\n"

// static bool module_work = false;
static bool test_mode = false;
static void sigal_hander(int sig);

static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data);
static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data);
static int ble_gatt_cb(gl_ble_gatt_event_t event, gl_ble_gatt_data_t *data);
static int str2addr(char* str, BLE_MAC address);
static int addr2str(BLE_MAC adr, char* str);
static int hex2str(uint8_t* head, int len, char* value);


int readLineInit(void);

static bool hold_loop = true;
static bool start_discovery = false;


/* System functions */
GL_RET cmd_enable(int argc, char **argv)
{
	int enable = 0;
	if (argc != 2) 
	{
		enable = 1;
	}else {
		enable = atoi(argv[1]);
	}

	GL_RET ret  = gl_ble_enable(enable);

	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));
	const char *temp=json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);
	
	return GL_SUCCESS;
}

GL_RET cmd_reset(int argc, char **argv)
{
	int enable = 0;
	if (argc != 2) 
	{
		enable = 1;
	}else {
		enable = atoi(argv[1]);
	}

	GL_RET ret  = gl_ble_enable(enable);

	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));
	const char *temp=json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);
	
	return GL_SUCCESS;
}

GL_RET cmd_local_address(int argc, char **argv)
{
	BLE_MAC address;
	char str_addr[20] = {0};
	GL_RET ret = gl_ble_get_mac(address);

	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));
	if(ret == GL_SUCCESS)
	{
		addr2str(address, str_addr);
		json_object_object_add(o, "mac", json_object_new_string(str_addr));
	}
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_set_power(int argc, char **argv)
{
	int power = 0;
	int current_p = 0;
	if (argc < 2) {
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}
	else {
		power = atoi(argv[1]);
	}

	GL_RET ret = gl_ble_set_power(power, &current_p);

	// json format
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

	return GL_SUCCESS;
}

/*BLE slave functions */
GL_RET cmd_adv(int argc, char **argv)
{
	int phys = 1, interval_min = 160, interval_max = 160, discover = 2, adv_conn = 2;

	if((argc != 1) && (argc != 6))
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	if(argc == 6)
	{
		phys = atoi(argv[1]);
		interval_min = atoi(argv[2]);
		interval_max = atoi(argv[3]);
		discover = atoi(argv[4]);
		adv_conn = atoi(argv[5]);
	}

	if (interval_max < interval_min)
	{
		interval_max = interval_min;
	}

	GL_RET ret = gl_ble_adv(phys, interval_min, interval_max, discover, adv_conn);

	// json format	
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_adv_data(int argc, char **argv)
{
	int flag = -1;
	char *value = NULL;

	if(argc != 3)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	flag = atoi(argv[1]);
	value = argv[2];

	if (flag < 0 || !value) {
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}

	GL_RET ret = gl_ble_adv_data(flag, value);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);	
	
	
	json_object_put(o);
	return GL_SUCCESS;
}

GL_RET cmd_adv_stop(int argc, char **argv)
{
	GL_RET ret = gl_ble_stop_adv();

	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_send_notify(int argc, char **argv)
{
	int char_handle = -1;
	char *value = NULL;
	char *address = NULL;	

	if(argc != 4)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	address = argv[1];
	char_handle = atoi(argv[2]);
	value = argv[3];

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	uint8_t addr_len = strlen(address);
	if (addr_len < BLE_MAC_LEN - 1 || char_handle < 0 || !value)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret = gl_ble_send_notify(address_u8, char_handle, value);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp=json_object_to_json_string(o);
	printf("%s\n",temp);	

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_discovery(int argc, char **argv)
{
	int phys = 1, interval = 16, window = 16, type = 0, mode = 1;

	if((argc != 1) && (argc != 6))
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	if(argc == 6)
	{
		phys = atoi(argv[1]);
		interval = atoi(argv[2]);
		window = atoi(argv[3]);
		type = atoi(argv[4]);
		mode = atoi(argv[5]);
	}

	GL_RET ret = gl_ble_discovery(phys, interval, window, type, mode);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	start_discovery = true;

	return GL_SUCCESS;
}

GL_RET cmd_stop(int argc, char **argv)
{
	GL_RET ret = gl_ble_stop_discovery();

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_connect(int argc, char **argv)
{
	int phy = 1, address_type = -1;
	char *address = NULL;

	if(argc != 4)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	phy = atoi(argv[1]);
	address_type = atoi(argv[2]);
	address = argv[3];

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	if (address_type < 0 || !address) {
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret =  gl_ble_connect(address_u8, address_type, phy);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));
	const char *temp=json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_disconnect(int argc, char **argv)
{
	char *address = NULL;

	if(argc != 2)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	address = argv[1];

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	uint8_t addr_len = strlen(address);
	if (addr_len < BLE_MAC_LEN - 1)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret = gl_ble_disconnect(address_u8);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_get_rssi(int argc, char **argv)
{
	char *address = NULL;

	if(argc != 2)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	address = argv[1];

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	uint8_t addr_len = strlen(address);
	if (addr_len != BLE_MAC_LEN - 1)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}
	
	BLE_MAC address_u8;
	str2addr(address, address_u8);
	int rssi = 0;

	GL_RET ret = gl_ble_get_rssi(address_u8, &rssi);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));

	if ( ret == GL_SUCCESS ) {
		json_object_object_add(o, "rssi", json_object_new_int(rssi));
	}
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_get_service(int argc, char **argv)
{
	char *address = NULL;

	if (argc != 2)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}
	address = argv[1];


	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	uint8_t addr_len = strlen(address);
	if (addr_len < BLE_MAC_LEN - 1)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	gl_ble_service_list_t service_list;
	memset(&service_list, 0, sizeof(gl_ble_service_list_t));

	int ret = gl_ble_get_service(&service_list, address_u8);

	// json format
	json_object *o = NULL, *l = NULL, *obj = NULL, *array = NULL;
	
	array = json_object_new_array();
	obj = json_object_new_object(); 
	json_object_object_add(obj, "code", json_object_new_int(ret));
    json_object_object_add(obj, "service_list", array);
	int len = service_list.list_len;
	int i = 0;

	if ( !ret ) {
		while ( i < len ) {	
			o = json_object_new_object();
            l = json_object_object_get(obj, "service_list");
			json_object_object_add(o, "service_handle", json_object_new_int(service_list.list[i].handle));
			json_object_object_add(o, "service_uuid", json_object_new_string(service_list.list[i].uuid));
            json_object_array_add(l,o);
			i++;
		}
	}
	const char *temp = json_object_to_json_string(obj);
	printf("%s\n",temp);

	
	json_object_put(obj); 

	return GL_SUCCESS;
}

GL_RET cmd_get_char(int argc, char **argv)
{
	int service_handle = -1;
	char *address = NULL;	
	uint8_t addr_len;

	if (argc != 3)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}
	address = argv[1];
	service_handle = atoi(argv[2]);

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	addr_len = strlen(address);
	if (addr_len < BLE_MAC_LEN - 1 || service_handle < 0)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	gl_ble_char_list_t char_list;
	memset(&char_list, 0, sizeof(gl_ble_char_list_t));

	GL_RET ret = gl_ble_get_char(&char_list, address_u8, service_handle);

	// json format
	json_object *o = NULL, *l = NULL, *obj = NULL, *array = NULL;
	
	array = json_object_new_array();
	obj = json_object_new_object(); 
	json_object_object_add(obj, "code", json_object_new_int(ret));
    json_object_object_add(obj, "characteristic_list", array);
	int len = char_list.list_len;
	int i = 0;

	if ( ret == GL_SUCCESS ) {
		while ( i < len ) {	
			o = json_object_new_object();
            l = json_object_object_get(obj, "characteristic_list");
			json_object_object_add(o, "characteristic_handle", json_object_new_int(char_list.list[i].handle));
			json_object_object_add(o, "properties", json_object_new_int(char_list.list[i].properties));
			json_object_object_add(o, "characteristic_uuid", json_object_new_string(char_list.list[i].uuid));
            json_object_array_add(l,o);
			i++;
		}
	}
	const char *temp = json_object_to_json_string(obj);
	printf("%s\n",temp);

	
	json_object_put(obj);

	return GL_SUCCESS;
}
GL_RET cmd_set_notify(int argc, char **argv)
{
	int char_handle = -1, flag = -1;
	char *address = NULL;	
	uint8_t addr_len;

	if (argc != 4)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM_MISSING;
	}
	address = argv[1];
	char_handle = atoi(argv[2]);
	flag = atoi(argv[3]);

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	addr_len = strlen(address);
	if (addr_len < BLE_MAC_LEN - 1 || char_handle < 0 || flag < 0)
	{
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret = gl_ble_set_notify(address_u8, char_handle, flag);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_read_value(int argc, char **argv)
{
	int char_handle = -1;
	char *address = NULL;
	uint8_t addr_len;

	if(argc != 3)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	address = argv[1];
	char_handle = atoi(argv[2]);

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	addr_len = strlen(address);

	if (addr_len < BLE_MAC_LEN - 1 || char_handle < 0) {
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret = gl_ble_read_char(address_u8, char_handle);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o,"code",json_object_new_int(ret));

	const char *temp = json_object_to_json_string(o);
	printf("%s\n",temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

GL_RET cmd_write_value(int argc, char **argv)
{
	int char_handle = -1, res = 0;
	char *value = NULL;
	char *address = NULL;	
	uint8_t addr_len;

	if(argc != 5)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}
	address = argv[1];
	char_handle = atoi(argv[2]);
	res = atoi(argv[3]);
	value = argv[4];

	if(address == NULL)
	{
		printf(PARA_MISSING);
		return GL_ERR_PARAM;
	}

	addr_len = strlen(address);

	if (addr_len < BLE_MAC_LEN - 1  || char_handle < 0 || !value) {
		printf(PARA_ERROR);
		return GL_ERR_PARAM;
	}

	BLE_MAC address_u8;
	str2addr(address, address_u8);

	GL_RET ret = gl_ble_write_char(address_u8, char_handle, value, res);

	// json format
	json_object* o = NULL;
	o = json_object_new_object();
	json_object_object_add(o, "code", json_object_new_int(ret));
	const char *temp = json_object_to_json_string(o);
	printf("%s\n", temp);

	
	json_object_put(o);

	return GL_SUCCESS;
}

static int ble_gatt_cb(gl_ble_gatt_event_t event, gl_ble_gatt_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	switch (event)
	{
		case GATT_REMOTE_CHARACTERISTIC_VALUE_EVT:
		{
			addr2str(data->remote_characteristic_value.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("remote_characteristic_value"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "characteristic", json_object_new_int(data->remote_characteristic_value.characteristic));
			json_object_object_add(o, "att_opcode", json_object_new_int(data->remote_characteristic_value.att_opcode));
			json_object_object_add(o, "offset", json_object_new_int(data->remote_characteristic_value.offset));
			json_object_object_add(o, "value", json_object_new_string(data->remote_characteristic_value.value));
			const char *temp=json_object_to_json_string(o);
			printf("GATT_CB_MSG >> %s\n",temp);

			json_object_put(o);
			break;
		}
		case GATT_LOCAL_GATT_ATT_EVT:
		{
			addr2str(data->local_gatt_attribute.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("local_gatt_attribute"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "attribute", json_object_new_int(data->local_gatt_attribute.attribute));
			json_object_object_add(o, "att_opcode", json_object_new_int(data->local_gatt_attribute.att_opcode));
			json_object_object_add(o, "offset", json_object_new_int(data->local_gatt_attribute.offset));
			json_object_object_add(o, "value", json_object_new_string(data->local_gatt_attribute.value));
			const char *temp = json_object_to_json_string(o);
			printf("GATT_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}
		case GATT_LOCAL_CHARACTERISTIC_STATUS_EVT:
		{
			addr2str(data->local_characteristic_status.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("remote_set"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "characteristic", json_object_new_int(data->local_characteristic_status.characteristic));
			json_object_object_add(o, "status_flags", json_object_new_int(data->local_characteristic_status.status_flags));
			json_object_object_add(o, "client_config_flags", json_object_new_int(data->local_characteristic_status.client_config_flags));
			const char *temp = json_object_to_json_string(o);
			printf("GATT_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}

		default:
			break;
	}

	return 0;
}

static int ble_module_cb(gl_ble_module_event_t event, gl_ble_module_data_t *data)
{
	switch (event)
	{
		case MODULE_BLE_SYSTEM_BOOT_EVT:
		{
			// module_work = true;
			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("module_start"));
			json_object_object_add(o, "major", json_object_new_int(data->system_boot_data.major));
			json_object_object_add(o, "minor", json_object_new_int(data->system_boot_data.minor));
			json_object_object_add(o, "patch", json_object_new_int(data->system_boot_data.patch));
			json_object_object_add(o, "build", json_object_new_int(data->system_boot_data.build));
			json_object_object_add(o, "bootloader", json_object_new_int(data->system_boot_data.bootloader));
			json_object_object_add(o, "hw", json_object_new_int(data->system_boot_data.hw));
			json_object_object_add(o, "ble_hash", json_object_new_string(data->system_boot_data.ble_hash));
			const char *temp = json_object_to_json_string(o);
			printf("MODULE_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}
		default:
			break;
	}

	return 0;
}

static int ble_gap_cb(gl_ble_gap_event_t event, gl_ble_gap_data_t *data)
{
	char address[BLE_MAC_LEN] = {0};
	char ble_adv[MAX_ADV_DATA_LEN] = {0};
	switch (event)
	{
		case GAP_BLE_SCAN_RESULT_EVT:
		{
			addr2str(data->scan_rst.address, address);
			hex2str(data->scan_rst.ble_adv, data->scan_rst.ble_adv_len, ble_adv);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("scan_result"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "address_type", json_object_new_int(data->scan_rst.ble_addr_type));
			json_object_object_add(o, "rssi", json_object_new_int(data->scan_rst.rssi));
			json_object_object_add(o, "packet_type", json_object_new_int(data->scan_rst.packet_type));
			json_object_object_add(o, "bonding", json_object_new_int(data->scan_rst.bonding));
			json_object_object_add(o, "data", json_object_new_string(ble_adv));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);

			json_object_put(o);
			break;
		}

		case GAP_BLE_UPDATE_CONN_EVT:
		{
			addr2str(data->update_conn_data.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("connect_update"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "interval", json_object_new_int(data->update_conn_data.interval));
			json_object_object_add(o, "latency", json_object_new_int(data->update_conn_data.latency));
			json_object_object_add(o, "timeout", json_object_new_int(data->update_conn_data.timeout));
			json_object_object_add(o, "security_mode", json_object_new_int(data->update_conn_data.security_mode));
			json_object_object_add(o, "txsize", json_object_new_int(data->update_conn_data.txsize));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}

		case GAP_BLE_CONNECT_EVT:
		{
			addr2str(data->connect_open_data.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("connect_open"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "address_type", json_object_new_int(data->connect_open_data.ble_addr_type));
			json_object_object_add(o, "connect_role", json_object_new_int(data->connect_open_data.conn_role));
			json_object_object_add(o, "bonding", json_object_new_int(data->connect_open_data.bonding));
			json_object_object_add(o, "advertiser", json_object_new_int(data->connect_open_data.advertiser));
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}

		case GAP_BLE_DISCONNECT_EVT:
		{
			addr2str(data->disconnect_data.address, address);

			// json format
			json_object* o = NULL;
			o = json_object_new_object();
			json_object_object_add(o, "type", json_object_new_string("connect_close"));
			json_object_object_add(o, "mac", json_object_new_string(address));
			json_object_object_add(o, "reason", json_object_new_int(data->disconnect_data.reason));	
			const char *temp = json_object_to_json_string(o);
			printf("GAP_CB_MSG >> %s\n",temp);
			
			json_object_put(o);
			break;
		}
		default:
			break;
	}

	return 0;
}

GL_RET cmd_quite(int argc, char **argv)
{
	hold_loop = false;

	return GL_SUCCESS;
}

GL_RET cmd_stop_current_discovery(int argc, char **argv)
{
	if(!start_discovery)
	{
		printf("Command not support. Please input help to get more.\n");
		return GL_SUCCESS;
	}


	GL_RET ret = gl_ble_stop_discovery();
	if(ret == GL_SUCCESS)
	{
		start_discovery = false;
	}else{
		printf("gl_ble_stop_discovery error return %d\n", ret);
		exit(0);
	}

	return ret;
}

static int i = 0;
static int los_num = 0;
static int err_num = 0;
GL_RET cmd_test(int argc, char **argv)
{
	printf(" Start test!!!\n");
	int test_num = 1000;
	if(argc == 2)
	{
		test_num = atoi(argv[1]);
	}

	BLE_MAC ole_address;
	BLE_MAC new_address;
	char str_addr[20] = {0};

	i = 0;
	los_num = 0;
	err_num = 0;
	test_mode = true;
	while(i < test_num)
	{
		printf("TEST NUM: %d/%d  ", i+1, test_num);
		memset(new_address, 0, DEVICE_MAC_LEN);
		GL_RET ret = gl_ble_get_mac(new_address);
		if(ret != GL_SUCCESS)
		{
			los_num++;
			printf("mac lost!\n");
		}else{
			addr2str(new_address, str_addr);
			printf("%s  ", str_addr);
			if(i == 0)
			{
				memcpy(&ole_address, &new_address, DEVICE_MAC_LEN);
				printf("\n");
			}else{
				if(0 != memcmp(&ole_address, &new_address, DEVICE_MAC_LEN))
				{
					err_num++;
					printf("failed!\n");
				}else{
					printf("passed!\n");
				}
			}
		}
		i++;
	}

	printf("Test end! Test time: %d  Error time: %d  Lost time: %d\n", i, err_num, los_num);
	return GL_SUCCESS;
}

GL_RET cmd_help(int argc, char **argv);

typedef struct {
	char *name;
	int (*func)(int argc, char **argv);
	char *doc;
} command_t;

command_t command_list[] = {
	/* System functions */
	{"quit", cmd_quite, "Quit bletool"},
	{"help", cmd_help, "Help"},
	{"enable", cmd_enable, "Enable or disable the module"},
	{"set_power", cmd_set_power, "Set the tx power level"},
	{"local_address", cmd_local_address, "Get local Bluetooth module public address"},
	/*BLE slave functions */
	{"adv_data", cmd_adv_data, "Set adv data"},
	{"adv", cmd_adv, "Set and Start advertising"},
	{"adv_stop", cmd_adv_stop, "Stop advertising"},
	{"send_notify", cmd_send_notify, "Send notification to remote device"},
	/*BLE master functions */
	{"discovery", cmd_discovery, "Start discovery"},
	{"stop_discovery", cmd_stop, "End current GAP procedure"},
	{"connect", cmd_connect, "Open connection"},
	{"disconnect", cmd_disconnect, "Close connection"},
	{"get_rssi", cmd_get_rssi, "Get rssi of an established connection"},
	{"get_service", cmd_get_service, "Get supported services list"},
	{"get_char", cmd_get_char, "Get supported characteristics in specified service"},
	{"set_notify", cmd_set_notify, "Enable or disable the notifications and indications"},
	{"read_value", cmd_read_value, "Read specified characteristic value"},
	{"write_value", cmd_write_value, "Write characteristic value"},
	{"uart_test", cmd_test, "Test the stability of serial communication"},
	/* Hidden method */
	{"q", cmd_stop_current_discovery, ""},
	{NULL, NULL, 0}
};

GL_RET cmd_help(int argc, char **argv)
{
	int i = 0;
	while (1) {
		if ((command_list[i].name) && (0 != strcmp(command_list[i].name, "q")))
		{
			printf("%-25s      %s\n", command_list[i].name, command_list[i].doc);

		}else {
			break;
		}
		i++;
	}
	return GL_SUCCESS;
}


/*****************************************************************************************************************************************/
static char *getCmdByIndex(unsigned int CmdIndex);
static char *cmdGenerator(const char *pszText, int State);
static char **cmdcompletion (const char *pszText, int Start, int End);
static void interactive_input(char* str);
static int match_cmd(int argc, char **argv);

#define CMD_MAP_NUM     ((sizeof(command_list)/sizeof(command_t)) - 1)

static char cmdprompt[] = "bletool >> ";

int main(int argc, char *argv[])
{
	gl_ble_init();

	gl_ble_cbs ble_cb;
	memset(&ble_cb, 0, sizeof(gl_ble_cbs));

	ble_cb.ble_gap_event = ble_gap_cb;
	ble_cb.ble_gatt_event = ble_gatt_cb;
	ble_cb.ble_module_event = ble_module_cb;
	gl_ble_subscribe(&ble_cb);

	signal(SIGTERM, sigal_hander);
	signal(SIGINT, sigal_hander);
	signal(SIGQUIT, sigal_hander);
	

	readLineInit();
	char* inputstr = NULL;

	while(hold_loop)
	{
		inputstr = readline(cmdprompt);
		if((inputstr == NULL)||(*inputstr == '\0'))
		{
			free(inputstr);
			inputstr = NULL;
			continue;
		}

		add_history(inputstr);

		interactive_input(inputstr);
		
		free(inputstr);
		inputstr = NULL;
	}

	gl_ble_destroy();

	return 0;
}

static char *getCmdByIndex(unsigned int CmdIndex)
{
	if(CmdIndex >= CMD_MAP_NUM)
		return NULL;
	return command_list[CmdIndex].name;
}

static char *cmdGenerator(const char *pszText, int State)
{
	static int ListIdx = 0, TextLen = 0;
	if(!State)
	{
		ListIdx = 0;
		TextLen = strlen(pszText);
	}

	const char *pszName = NULL;
	while((pszName = getCmdByIndex(ListIdx)))
	{
		ListIdx++;
		if(!strncmp (pszName, pszText, TextLen))
		{
			return strdup(pszName);
		}
	}
	return NULL;
}

static char **cmdcompletion (const char *pszText, int Start, int End)
{
	char **pMatches = NULL;
	if(0 == Start)
		pMatches = rl_completion_matches(pszText, cmdGenerator);
	return pMatches;
}

int readLineInit(void)
{
	rl_attempted_completion_function = cmdcompletion;

	return 0;
}

static int match_cmd(int argc, char **argv)
{
	int i;
	if(0 == argc)
	{
		printf("Command missing.\n");
		return -1;
	}

	for(i = 0; command_list[i].name;i++)
	{
		if((strlen(command_list[i].name) == strlen(argv[0])) && (0 == strcmp(command_list[i].name, argv[0])))
		{
			return command_list[i].func(argc, argv);
		}
	}

	printf("Command not support. Please input help to get more.\n");
	return -1;

}

static void interactive_input(char* str)
{
	char ptr[512+128] = {0};
	strcpy(ptr,str);
	char* parameter[20];
	int num = 0;
	char* token;
	token = strtok(ptr," ");
	while(token)
	{
		parameter[num] = (char*)calloc(512,sizeof(char));
		strncpy(parameter[num++],token,(strlen(token)>512?512:strlen(token)));
		token = strtok(NULL," ");
	}

	match_cmd(num,parameter);

	while(num)
	{
		num--;
		free(parameter[num]);
	}
	return ;
}

static void sigal_hander(int sig)
{
	if(test_mode)
	{
		printf("Test end! Test time: %d  Error time: %d  Lost time: %d\n", i, err_num, los_num);
	}
	gl_ble_unsubscribe();
	gl_ble_destroy();

    exit(0);
}

static int str2addr(char* str, BLE_MAC address) 
{
	int mac[6] = {0};
    sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x", &mac[5], &mac[4], &mac[3],
           &mac[2], &mac[1], &mac[0]);

	uint8_t i = 0;
	for(;i < 6; i++)
	{
		address[i] = mac[i];
	}
    return 0;
}

static int addr2str(BLE_MAC adr, char* str) 
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
            adr[3], adr[2], adr[1], adr[0]);
    return 0;
}

static int hex2str(uint8_t* head, int len, char* value)
{
    int i = 0;

    // FIXME: (Sometime kernel don't mask all uart print) When wifi network up/down, it will recv a big message
    if(len >= 256/2)    
    {    
        strcpy(value,"00");
        // printf("recv a err msg! err len = %d\n",len);
        return -1;
    }
    
    while (i < len) {
        sprintf(value + i * 2, "%02x", head[i]);
        i++;
    }
    return 0;
}