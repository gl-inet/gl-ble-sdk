#ifndef _IBEACON_H
#define _IBEACON_H

#include <json-c/json.h>
#include "gl_errno.h"
#include "gl_type.h"
#include "gl_bleapi.h"
/**
 * @brief Ibeacon type
 * 
 */
#define IBEACON_RECIEVER            "recieve"
#define IBEACON_SENDER              "send"
#define IBEACON_ACTIVE_SCAN         "active_scan"
#define IBEACON_PASSIVE_SCAN        "passive_scan"
#define IBEACON_SCAN_RSP_OPEN       "rsp_open"
#define IBEACON_SCAN_RSP_CLOSE      "rsp_close"

#define IBEACON_SEND_HEADER         "0201061aff004c1502"
#define IBEACON_RECIEVE_HEADER      "0201061aff4c000215"

#define IBEACON_PACKET_SIZE         30
#define IBEACON_HEADER_SIZE         9
#define PROXIMITY_UUID_SIZE	    	16
#define MAJOR_SIZE				    2
#define MINOR_SIZE				    2
#define MEASURED_POWER_SIZE		    1

#define MAX_DATA_NUM                1024

/**
 * @brief adv/scan parm
 * 
 */
/* common */
#define PHYS                1
/* adv */
#define INTERVAL_MIN        32
#define INTERVAL_MAX        64
#define DISCOVER            4
#define ADV_CONN            3
/* scan */
#define INTERVAL            80
#define WINDOW              48
#define SCAN_TYPE           0
#define MODE                1

/**
 * @brief ibeacon payload.
 */
typedef struct {
    uint8_t flags[3];
    uint8_t length;
    uint8_t type;
    uint16_t company_id;
    uint16_t beacon_type;
}__attribute__((packed)) ble_ibeacon_head_t;

typedef struct {
    uint8_t proximity_uuid[16];
    uint8_t major[2];
    uint8_t minor[2];
    int8_t measured_power;
}__attribute__((packed)) ble_ibeacon_vendor_t;


typedef struct {
    ble_ibeacon_head_t ibeacon_head;
    ble_ibeacon_vendor_t ibeacon_vendor;
}__attribute__((packed)) ble_ibeacon_t;

typedef struct
{
    uint16_t w_count;
    uint16_t r_count;
    gl_ble_gap_data_t data[MAX_DATA_NUM];
}ibeacon_ringArray_t;

typedef enum
{
    PROXIMITY_UUID,
    MAJOR,
    MINOR,
    MEASURED_POWER
}ibeacon_vendor_field_e;

extern ble_ibeacon_head_t ibeacon_common_head;

bool is_ibeacon_packet(uint8_t *adv_data, uint8_t adv_data_len);

void ibeacon_data_collect(gl_ble_gap_data_t *data);

char *get_ibeacon_vendor_field(char *data, uint8_t field, char *buf);

json_object *get_ibeacon_vendor(void);

#endif