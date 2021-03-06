/*****************************************************************************
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gl_common.h"



int addr2str(BLE_MAC adr, char* str) {
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", adr[5], adr[4],
            adr[3], adr[2], adr[1], adr[0]);
    return 0;
}

int str2addr(char* str, BLE_MAC address) {
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

int str2array(uint8_t* dst, char* src, int len) {
    int i = 0;
    int tmp;
    while (i < len) {
        sscanf(src + i * 2, "%02x", &tmp);
        dst[i] = tmp;
        i++;
    }
    return 0;
}
int hex2str(uint8_t* head, int len, char* value) {
    int i = 0;

    // fix bug(?): (kernel don't mask all uart print) When wifi network up/down, it will recv a big message
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

void reverse_endian(uint8_t* header, uint8_t length) {
    uint8_t* tmp = (uint8_t*)malloc(length);
    memcpy(tmp, header, length);
    int i = length - 1;
    int j = 0;
    for (; i >= 0; i--, j++) {
        *(header + j) = *(tmp + i);
    }
    free(tmp);
    return;
}
