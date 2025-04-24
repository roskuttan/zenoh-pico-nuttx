//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache Software License 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>
//

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"

//---------------------------------------------------------------------
// Result Codes
//---------------------------------------------------------------------
#ifndef _Z_RES_OK
#define _Z_RES_OK 0
#endif

#ifndef _Z_ERR_SYSTEM_GENERIC
#define _Z_ERR_SYSTEM_GENERIC -1
#endif

#ifndef Z_ETIMEDOUT
#define Z_ETIMEDOUT -2
#endif

//---------------------------------------------------------------------
// Type Definitions
//---------------------------------------------------------------------

typedef struct timespec z_clock_t;
typedef struct timeval  z_time_t;

typedef pthread_t       _z_task_t;
typedef pthread_attr_t  z_task_attr_t;
typedef pthread_mutex_t _z_mutex_t;
typedef pthread_cond_t  _z_condvar_t;

//---------------------------------------------------------------------
// Random Number Functions
//---------------------------------------------------------------------

uint8_t z_random_u8(void) {
    return (uint8_t)rand();
}

uint16_t z_random_u16(void) {
    return (uint16_t)rand();
}

uint32_t z_random_u32(void) {
    return (uint32_t)rand();
}

uint64_t z_random_u64(void) {
    uint64_t ret = ((uint64_t)rand() << 32) | (uint64_t)rand();
    return ret;
}

void z_random_fill(void *buf, size_t len) {
    uint8_t *p = (uint8_t *)buf;
    for (size_t i = 0; i < len; i++)
        p[i] = (uint8_t)rand();
}

//---------------------------------------------------------------------
// Memory Management Functions
//---------------------------------------------------------------------

void *z_malloc(size_t size) {
    return malloc(size);
}

void *z_realloc(void *ptr, size_t size) {
    return realloc(ptr, size);
}

void z_free(void *ptr) {
    free(ptr);
}

//---------------------------------------------------------------------
// Thread (Task) Functions
//---------------------------------------------------------------------

z_result_t _z_task_init(_z_task_t *task, z_task_attr_t *attr, void *(*fun)(void *), void *arg) {
    int ret;
    if (attr == NULL) {
        pthread_attr_t local_attr;
        ret = pthread_attr_init(&local_attr);
        if (ret != 0)
            return _Z_ERR_SYSTEM_GENERIC;
        // Set a default stack size (e.g., 4096 bytes; adjust as needed)
        ret = pthread_attr_setstacksize(&local_attr, 4096);
        if (ret != 0) {
            pthread_attr_destroy(&local_attr);
            return _Z_ERR_SYSTEM_GENERIC;
        }
        ret = pthread_create(task, &local_attr, fun, arg);
        pthread_attr_destroy(&local_attr);
    } else {
        ret = pthread_create(task, attr, fun, arg);
    }
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_task_join(_z_task_t *task) {
    int ret = pthread_join(*task, NULL);
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_task_detach(_z_task_t *task) {
    int ret = pthread_detach(*task);
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_task_cancel(_z_task_t *task) {
    int ret = pthread_cancel(*task);
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

void _z_task_exit(void) {
    pthread_exit(NULL);
}

void _z_task_free(_z_task_t **task) {
    *task = NULL;
}

//---------------------------------------------------------------------
// Mutex Functions
//---------------------------------------------------------------------

z_result_t _z_mutex_init(_z_mutex_t *m) {
    return (pthread_mutex_init(m, NULL) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_drop(_z_mutex_t *m) {
    if (m == NULL)
        return _Z_RES_OK;
    return (pthread_mutex_destroy(m) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_lock(_z_mutex_t *m) {
    return (pthread_mutex_lock(m) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_try_lock(_z_mutex_t *m) {
    return (pthread_mutex_trylock(m) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_unlock(_z_mutex_t *m) {
    return (pthread_mutex_unlock(m) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_rec_init(_z_mutex_rec_t *m) {
  pthread_mutexattr_t attr;
  if (pthread_mutexattr_init(&attr) != 0) return _Z_ERR_SYSTEM_GENERIC;
  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
      pthread_mutexattr_destroy(&attr);
      return _Z_ERR_SYSTEM_GENERIC;
  }
  int ret = pthread_mutex_init(&m->mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_rec_drop(_z_mutex_rec_t *m) {
  return (pthread_mutex_destroy(&m->mutex) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_rec_lock(_z_mutex_rec_t *m) {
  return (pthread_mutex_lock(&m->mutex) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_rec_try_lock(_z_mutex_rec_t *m) {
  return (pthread_mutex_trylock(&m->mutex) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_mutex_rec_unlock(_z_mutex_rec_t *m) {
  return (pthread_mutex_unlock(&m->mutex) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

//---------------------------------------------------------------------
// Condition Variable Functions
//---------------------------------------------------------------------

z_result_t _z_condvar_init(_z_condvar_t *cv) {
    pthread_condattr_t attr;
    int ret = pthread_condattr_init(&attr);
    if (ret != 0)
        return _Z_ERR_SYSTEM_GENERIC;
#ifdef CLOCK_MONOTONIC
    ret = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    if (ret != 0) {
        pthread_condattr_destroy(&attr);
        return _Z_ERR_SYSTEM_GENERIC;
    }
#endif
    ret = pthread_cond_init(cv, &attr);
    pthread_condattr_destroy(&attr);
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_condvar_drop(_z_condvar_t *cv) {
    return (pthread_cond_destroy(cv) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_condvar_signal(_z_condvar_t *cv) {
    return (pthread_cond_signal(cv) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_condvar_signal_all(_z_condvar_t *cv) {
    return (pthread_cond_broadcast(cv) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_condvar_wait(_z_condvar_t *cv, _z_mutex_t *m) {
    return (pthread_cond_wait(cv, m) == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

z_result_t _z_condvar_wait_until(_z_condvar_t *cv, _z_mutex_t *m, const z_clock_t *abstime) {
    int ret = pthread_cond_timedwait(cv, m, abstime);
    if (ret == ETIMEDOUT)
        return Z_ETIMEDOUT;
    return (ret == 0) ? _Z_RES_OK : _Z_ERR_SYSTEM_GENERIC;
}

//---------------------------------------------------------------------
// Sleep Functions
//---------------------------------------------------------------------

z_result_t z_sleep_us(size_t time_us) {
    while (usleep(time_us) != 0 && errno == EINTR);
    return _Z_RES_OK;
}

z_result_t z_sleep_ms(size_t time_ms) {
    return z_sleep_us(time_ms * 1000);
}

z_result_t z_sleep_s(size_t time_s) {
    sleep(time_s);
    return _Z_RES_OK;
}

//---------------------------------------------------------------------
// Clock Functions
//---------------------------------------------------------------------

z_clock_t z_clock_now(void) {
    z_clock_t now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now;
}

unsigned long z_clock_elapsed_us(z_clock_t *instant) {
    z_clock_t now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    unsigned long elapsed = (now.tv_sec - instant->tv_sec) * 1000000UL +
                            (now.tv_nsec - instant->tv_nsec) / 1000UL;
    return elapsed;
}

unsigned long z_clock_elapsed_ms(z_clock_t *instant) {
    z_clock_t now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    unsigned long elapsed = (now.tv_sec - instant->tv_sec) * 1000UL +
                            (now.tv_nsec - instant->tv_nsec) / 1000000UL;
    return elapsed;
}

unsigned long z_clock_elapsed_s(z_clock_t *instant) {
    z_clock_t now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec - instant->tv_sec;
}

void z_clock_advance_us(z_clock_t *clock, unsigned long duration_us) {
    clock->tv_sec += duration_us / 1000000UL;
    clock->tv_nsec += (duration_us % 1000000UL) * 1000UL;
    if (clock->tv_nsec >= 1000000000UL) {
        clock->tv_sec += 1;
        clock->tv_nsec -= 1000000000UL;
    }
}

void z_clock_advance_ms(z_clock_t *clock, unsigned long duration_ms) {
    z_clock_advance_us(clock, duration_ms * 1000UL);
}

void z_clock_advance_s(z_clock_t *clock, unsigned long duration_s) {
    clock->tv_sec += duration_s;
}

//---------------------------------------------------------------------
// Time Functions
//---------------------------------------------------------------------

z_time_t z_time_now(void) {
    z_time_t now;
    gettimeofday(&now, NULL);
    return now;
}

const char *z_time_now_as_str(char *const buf, unsigned long buflen) {
    z_time_t tv = z_time_now();
    struct tm ts;
    localtime_r(&tv.tv_sec, &ts);
    strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", &ts);
    return buf;
}

unsigned long z_time_elapsed_us(z_time_t *time) {
    z_time_t now;
    gettimeofday(&now, NULL);
    unsigned long elapsed = (now.tv_sec - time->tv_sec) * 1000000UL +
                            (now.tv_usec - time->tv_usec);
    return elapsed;
}

unsigned long z_time_elapsed_ms(z_time_t *time) {
    return z_time_elapsed_us(time) / 1000;
}

unsigned long z_time_elapsed_s(z_time_t *time) {
    z_time_t now;
    gettimeofday(&now, NULL);
    return now.tv_sec - time->tv_sec;
}