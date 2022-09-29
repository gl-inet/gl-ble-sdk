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
