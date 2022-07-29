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

#ifndef GL_HW_CFG_H
#define GL_HW_CFG_H

#include "gl_hal.h"

/*
    hw_cfg_t DEFAULT_BLE_HW_CFG = {                                          
        "default",      // model name                                  
        "/dev/ttyS0", // serial port name                              
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        1,              // RTS IO                                      
    };
*/


hw_cfg_t X750_BLE_HW_CFG = {                                         
    "x750",      // model name                                     
    "/dev/ttyS0", // serial port name                              
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    1,              // RTS IO                                      
};

hw_cfg_t S1300_BLE_HW_CFG = {                                        
    "s1300",      // model name                                    
    "/dev/ttyMSM1", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    48,              // RTS IO                                      
};

hw_cfg_t XE300_BLE_HW_CFG = {                                      
    "xe300",      // model name                                  
    "/dev/ttyS0", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    14,              // RTS IO                                      
};

hw_cfg_t MT300N_V2_BLE_HW_CFG = {                                                                    
    "mt300n-v2",      // model name                                  
    "/dev/ttyS1", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    5,              // RTS IO                                      
};

hw_cfg_t E750_BLE_HW_CFG = {                                        
    "e750",      // model name                                    
    "/dev/ttyUSB0", // serial port name                           
    115200,         // baud rate (bits/second)                    
    0,              // true enables RTS/CTS flow control          
    1,              // rst trigger: 1 means high, 0 means low     
    17,              // RTS IO                                    
};

hw_cfg_t X300B_BLE_HW_CFG = {                                                                    
    "x300b",      // model name                                  
    "/dev/ttyS0", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    16,              // RTS IO                                      
};

hw_cfg_t AP1300_BLE_HW_CFG = {                                                                    
    "ap1300",      // model name                                  
    "/dev/ttyUSB0", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    62,              // RTS IO                                      
};

hw_cfg_t B2200_BLE_HW_CFG = {                                                                    
    "b2200",      // model name                                  
    "/dev/ttyMSM1", // serial port name                            
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    42,              // RTS IO                                      
};

hw_cfg_t S200_BLE_HW_CFG = {                                         
    "s200",         // model name                                     
    "/dev/ttyS0",   // serial port name                              
    115200,         // baud rate (bits/second)                     
    0,              // true enables RTS/CTS flow control           
    1,              // rst trigger: 1 means high, 0 means low      
    1,              // RTS IO                                      
};

#endif