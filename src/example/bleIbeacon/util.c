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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

int ENDIAN;

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

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