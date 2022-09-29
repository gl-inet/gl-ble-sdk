#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

int ENDIAN;

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02X:%02X:%02X:%02X:%02X:%02X"

int addr2str(BLE_MAC adr, char* str) 
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
            adr[3], adr[2], adr[1], adr[0]);
    return 0;
}

int str2array(uint8_t* dst, char* src, int len) 
{
    int i = 0;
    int tmp;
    while (i < len) {
        sscanf(src + i * 2, "%02x", &tmp);
        dst[i] = tmp;
        // printf("str2array dst[%d] = 0x%02x\n", i, dst[i]);
        i++;
    }
    return 0;
}

int hex2str(uint8_t* head, int len, char* value) {
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