/*****************************************************************************
 * @file 
 * @brief 
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

#ifndef GL_HAL_H
#define GL_HAL_H


extern char rston[];
extern char rstoff[];

typedef struct {
    char model[20];
    char port[32];
    uint32_t baudRate;
    uint32_t flowcontrol;
    uint8_t rst_trigger;
    uint16_t rst_gpio;
}hw_cfg_t;

/*Initilize the hal*/
int hal_init(void);

int hal_destroy(void);


#endif