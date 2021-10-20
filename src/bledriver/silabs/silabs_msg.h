#ifndef _SILABS_MSG_H_
#define _SILABS_MSG_H_

#include "host_gecko.h"

#ifndef BGLIB_QUEUE_LEN
#define BGLIB_QUEUE_LEN 30
#endif


#define BGLIB_DEFINE()                                      \
  struct gecko_cmd_packet _gecko_cmd_msg;                   \
  struct gecko_cmd_packet _gecko_rsp_msg;                   \
  struct gecko_cmd_packet *gecko_cmd_msg = &_gecko_cmd_msg; \
  struct gecko_cmd_packet *gecko_rsp_msg = &_gecko_rsp_msg; \
  int32_t (*bglib_output)(uint32_t len1, uint8_t* data1);      \
  int32_t (*bglib_input)(uint32_t len1, uint8_t* data1);    \
  int32_t (*bglib_peek)(void);                              \
  struct gecko_cmd_packet gecko_queue[BGLIB_QUEUE_LEN];     \
  int    gecko_queue_w = 0;                                 \
  int    gecko_queue_r = 0;                                 


// #define BGLIB_MSG_GROUP()											
// 	uint32_t msg_group_bit;
// 	struct gecko_cmd_packet save_evt[50];

extern struct gecko_cmd_packet gecko_queue[BGLIB_QUEUE_LEN];
extern int    gecko_queue_w;
extern int    gecko_queue_r;

#define SPE_EVT_MAX				50
extern struct gecko_cmd_packet special_evt[SPE_EVT_MAX];
extern int special_evt_num;




#define BGLIB_INITIALIZE(OFUNC, IFUNC) bglib_output = OFUNC; bglib_input = IFUNC; bglib_peek = NULL;

int wait_rsp_evt(uint32_t evt_id, uint32_t timeout);


typedef struct{
  int evt_msgid;
}driver_param_t;

void* silabs_driver(void* arg);


#endif