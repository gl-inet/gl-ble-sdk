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
#include <stdbool.h>
#include <json-c/json.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <libubox/blobmsg_json.h>

#include "ibeacon.h"
#include "util.h"

extern ibeacon_ringArray_t *ibeacon_array;

ble_ibeacon_head_t ibeacon_common_head = {
    .flags = {0x02, 0x01, 0x06},
    .length = 0x1A,
    .type = 0xFF,
    .company_id = 0x4C00,
    .beacon_type = 0x0215
};

bool is_ibeacon_packet(uint8_t *adv_data, uint8_t adv_data_len)
{
    bool result = false;

    if ((adv_data != NULL) && (adv_data_len == 0x1E)){
        if (!memcmp(adv_data, (uint8_t*)&ibeacon_common_head, IBEACON_HEADER_SIZE)){
            result = true;
        }
    }

    return result;
}

void ibeacon_data_collect(gl_ble_gap_data_t *data)
{
	if(!is_ibeacon_packet(data->legacy_scan_rst.ble_adv, data->legacy_scan_rst.ble_adv_len))
	{
		return;
	}

	memcpy(&ibeacon_array->data[ibeacon_array->w_count], data, sizeof(gl_ble_gap_data_t));
	ibeacon_array->w_count = (ibeacon_array->w_count + 1) % MAX_DATA_NUM;

}

char *get_ibeacon_vendor_field(char *data, uint8_t field, char *buf)
{	
	switch (field)
	{
		case PROXIMITY_UUID:

			memcpy(buf, &data[IBEACON_HEADER_SIZE*2], PROXIMITY_UUID_SIZE*2);
			break;
		
		case MAJOR:

			memcpy(buf, &data[IBEACON_HEADER_SIZE*2 + PROXIMITY_UUID_SIZE*2], MAJOR_SIZE*2);
			break;

		case MINOR:

			memcpy(buf, &data[IBEACON_HEADER_SIZE*2 + PROXIMITY_UUID_SIZE*2 + MAJOR_SIZE*2], MINOR_SIZE*2);
			break;

		case MEASURED_POWER:

			memcpy(buf, &data[IBEACON_HEADER_SIZE*2 + PROXIMITY_UUID_SIZE*2 + MAJOR_SIZE*2 + MINOR_SIZE*2], MEASURED_POWER_SIZE*2);
			break;

		default:
			break;
	}

	return buf;
}

json_object *get_ibeacon_vendor(void)
{
	char address[BLE_MAC_LEN] = {0};
	char ble_adv[MAX_LEGACY_ADV_DATA_LEN * 2 + 1] = {0};
	char proximity_uuid[PROXIMITY_UUID_SIZE*2 + 1] = {0};
	char major[MAJOR_SIZE*2 + 1] = {0};
	char minor[MINOR_SIZE*2 + 1] = {0};
	char measured_power[MEASURED_POWER_SIZE*2 + 1] = {0};
	uint8_t power = 0;

	if(ibeacon_array->r_count != ibeacon_array->w_count)
	{
		addr2str(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.address, address);
		hex2str(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.ble_adv, ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.ble_adv_len, ble_adv);

		get_ibeacon_vendor_field(ble_adv, PROXIMITY_UUID, proximity_uuid);
		get_ibeacon_vendor_field(ble_adv, MAJOR, major);
		get_ibeacon_vendor_field(ble_adv, MINOR, minor);
	    get_ibeacon_vendor_field(ble_adv, MEASURED_POWER, measured_power);

		str2array(&power, measured_power, strlen(measured_power) / 2);
		
		// json format
		json_object *o = NULL;
		o = json_object_new_object();
		json_object_object_add(o, "type", json_object_new_string("Beacon"));
		json_object_object_add(o, "mac", json_object_new_string(address));
		json_object_object_add(o, "address_type", json_object_new_int(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.ble_addr_type));
		json_object_object_add(o, "rssi", json_object_new_int(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.rssi));
		json_object_object_add(o, "event_flags", json_object_new_int(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.event_flags));
		json_object_object_add(o, "bonding", json_object_new_int(ibeacon_array->data[ibeacon_array->r_count].legacy_scan_rst.bonding));

		json_object_object_add(o, "proximity_uuid", json_object_new_string(proximity_uuid));
		json_object_object_add(o, "major", json_object_new_string(major));
		json_object_object_add(o, "minor", json_object_new_string(minor));
		json_object_object_add(o, "measured_power", json_object_new_int((int8_t)power));

		ibeacon_array->r_count = (ibeacon_array->r_count + 1) % MAX_DATA_NUM;

		return o;
	}
	else
	{
		usleep(500000);
	}
	return NULL;
}