#ifndef _UTIL_H
#define _UTIL_H

#include "gl_type.h"

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

int addr2str(BLE_MAC adr, char* str);

int str2array(uint8_t* dst, char* src, int len);

int hex2str(uint8_t* head, int len, char* value);


#endif 