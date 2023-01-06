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
#ifndef _RUN_MODE_H
#define _RUN_MODE_H

typedef enum{
    BACKGROUNG_MODE,
    FOREGROUND_MODE
}mode_e;

int mode_check(int argc);

int foreground_param_check(int argc, char *argv[]);

int foreground(char *argv[]);

int foreground_ibeacon_reciever(void);

int foreground_ibeacon_sender(char *argv[]);

int background(void);

#endif
