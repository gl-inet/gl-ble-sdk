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

#ifndef GL_HW_CFG_H
#define GL_HW_CFG_H

#include "gl_hal.h"

hw_cfg_t GL_BLE_HW_LIST[] = 
{
    {                                         
        "x750",         // model name                                     
        "/dev/ttyS0",   // serial port name                              
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        1,              // RTS IO  
        0xff,           // DFU IO                                     
    },

    {                                        
        "s1300",        // model name                                    
        "/dev/ttyMSM1", // serial port name                         
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        48,             // RTS IO   
        0xff,           // DFU IO                                    
    },

    {                                      
        "xe300",        // model name                                  
        "/dev/ttyS0",   // serial port name                            
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        14,             // RTS IO    
        0xff,           // DFU IO                                   
    },

    {                                                                    
        "x300b",        // model name                                  
        "/dev/ttyS0",   // serial port name                            
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        16,             // RTS IO   
        0xff,           // DFU IO                                       
    },

    {                                                                    
        "ap1300",       // model name                                  
        "/dev/ttyUSB0", // serial port name                            
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        62,             // RTS IO    
        0xff,           // DFU IO                                      
    },

    {                                                                    
        "b2200",        // model name                                  
        "/dev/ttyMSM1", // serial port name                            
        115200,         // baud rate (bits/second)                     
        0,              // true enables RTS/CTS flow control           
        1,              // rst trigger: 1 means high, 0 means low      
        42,             // RTS IO   
        0xff,           // DFU IO                                      
    },

    {                                         
        "s200",                 // model name                                     
        "/dev/ttyCH343USB1",    // serial port name                              
        115200,                 // baud rate (bits/second)                     
        0,                      // true enables RTS/CTS flow control           
        1,                      // rst trigger: 1 means high, 0 means low      
        1,                      // RTS IO  
        11,                     // DFU IO                                    
    },

    {                                         
        "x3000",                // model name                                     
        "/dev/ttyCH343USB0",    // serial port name                              
        115200,                 // baud rate (bits/second)                     
        0,                      // true enables RTS/CTS flow control           
        1,                      // rst trigger: 1 means high, 0 means low      
        26,                     // RTS IO
        27,                     // DFU IO                                         
    },
};	

#endif