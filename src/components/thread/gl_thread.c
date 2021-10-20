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
#include <stdarg.h>
#include <memory.h>
#include <stdlib.h>

#include <pthread.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <semaphore.h>
#include <errno.h>
#include <assert.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#include "gl_thread.h"
#include "gl_log.h"

static thread_ctx_t g_thread_ctx = { 0 };

static void HAL_MutexLock(void *mutex);
static void HAL_MutexUnlock(void *mutex);
static int HAL_MutexTryLock(void *mutex);


thread_ctx_t* _thread_get_ctx(void)
{
    return &g_thread_ctx;
}

void _thread_ctx_mutex_lock(void)
{
    // log_debug("_thread_ctx_mutex_lock\n");
    thread_ctx_t* ctx = _thread_get_ctx();
    if (ctx->mutex) {
        HAL_MutexLock(ctx->mutex);
    }else{
		log_err("mutex lock NULL");
	}
}

int _thread_ctx_mutex_try_lock(void)
{
    // log_debug("_thread_ctx_mutex_try_lock\n");

    thread_ctx_t* ctx = _thread_get_ctx();
    if (ctx->mutex) {
        return HAL_MutexTryLock(ctx->mutex);
    }else{
		log_err("mutex try lock NULL");
	}

	return -1;
}

void _thread_ctx_mutex_unlock(void)
{
    // log_debug("_thread_ctx_mutex_unlock\n");

    thread_ctx_t* ctx = _thread_get_ctx();
    if (ctx->mutex) {
        HAL_MutexUnlock(ctx->mutex);
    }else{
		log_err("mutex unlock NULL");
	}
}

void *HAL_MutexCreate(void)
{
    int err_num;
    pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (NULL == mutex) {
        return NULL;
    }

    if (0 != (err_num = pthread_mutex_init(mutex, NULL))) {
        log_err("create mutex failed\n");
        free(mutex);
        return NULL;
    }

    return mutex;
}

void HAL_MutexDestroy(void *mutex)
{
    int err_num;

    if (!mutex) {
        printf("mutex want to destroy is NULL!\n");
        return;
    }
    if (0 != (err_num = pthread_mutex_destroy((pthread_mutex_t *)mutex))) {
        log_err("destroy mutex failed\n");
    }

    free(mutex);
}

static void HAL_MutexLock(void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_lock((pthread_mutex_t *)mutex))) {
        log_err("lock mutex failed: - '%s' (%d)\n", strerror(err_num), err_num);
    }
}

static int HAL_MutexTryLock(void *mutex)
{
    return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}

static void HAL_MutexUnlock(void *mutex)
{
    int err_num;
    if (0 != (err_num = pthread_mutex_unlock((pthread_mutex_t *)mutex))) {
        log_err("unlock mutex failed - '%s' (%d)\n", strerror(err_num), err_num);
    }
}

int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used)
{
    int ret = -1;

    if (stack_used) {
        *stack_used = 0;
    }

    ret = pthread_create((pthread_t *)thread_handle, NULL, work_routine, arg);

    return ret;
}

void HAL_ThreadDetach(void *thread_handle)
{
    pthread_detach((pthread_t)thread_handle);
}

void HAL_ThreadDelete(void *thread_handle)
{
    if (NULL == thread_handle) {
        pthread_exit(0);
    } else {
        /*main thread delete child thread*/
        pthread_cancel((pthread_t)thread_handle);
        pthread_join((pthread_t)thread_handle, 0);
    }
}
