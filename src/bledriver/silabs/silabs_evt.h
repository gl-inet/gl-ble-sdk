#ifndef _SILABS_EVT_H_
#define _SILABS_EVT_H_



typedef struct{
    long msgtype;   
    struct gecko_cmd_packet evt;
}silabs_msg_queue_t;

typedef struct{
    int evt_msgid;
    gl_ble_cbs* cbs;
}watcher_param_t;



void* silabs_watcher(void* arg);




#endif