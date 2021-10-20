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
#ifndef _GL_THREAD_H_
#define _GL_THREAD_H_

typedef struct {
    void* g_dispatch_thread;
    void* mutex;
} thread_ctx_t;


thread_ctx_t* _thread_get_ctx(void);

void *HAL_MutexCreate(void);

void HAL_MutexDestroy(void *mutex);

void _thread_ctx_mutex_lock(void);

int _thread_ctx_mutex_try_lock(void);

void _thread_ctx_mutex_unlock(void);




typedef enum {
    os_thread_priority_idle = -3,        /* priority: idle (lowest) */
    os_thread_priority_low = -2,         /* priority: low */
    os_thread_priority_belowNormal = -1, /* priority: below normal */
    os_thread_priority_normal = 0,       /* priority: normal (default) */
    os_thread_priority_aboveNormal = 1,  /* priority: above normal */
    os_thread_priority_high = 2,         /* priority: high */
    os_thread_priority_realtime = 3,     /* priority: realtime (highest) */
    os_thread_priority_error = 0x84,     /* system cannot determine priority or thread has illegal priority */
} hal_os_thread_priority_t;

typedef struct _hal_os_thread {
    hal_os_thread_priority_t priority;     /*initial thread priority */
    void                    *stack_addr;   /* thread stack address malloced by caller, use system stack by . */
    int                   stack_size;   /* stack size requirements in bytes; 0 is default stack size */
    int                      detach_state; /* 0: not detached state; otherwise: detached state. */
    char                    *name;         /* thread name. */
} hal_os_thread_param_t;

int HAL_ThreadCreate(void **thread_handle,void *(*work_routine)(void *), \
						void *arg,hal_os_thread_param_t *hal_os_thread_param, \
            			int *stack_used);

void HAL_ThreadDetach(void *thread_handle);

void HAL_ThreadDelete(void *thread_handle);





#endif