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
#ifndef _UTIL_H
#define _UTIL_H

#include "gl_type.h"

#define MAC2STR(a) (a)[5], (a)[4], (a)[3], (a)[2], (a)[1], (a)[0]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"

int addr2str(BLE_MAC adr, char* str);

int str2array(uint8_t* dst, char* src, int len);

int hex2str(uint8_t* head, int len, char* value);


#endif 