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

#ifndef ZENOH_PICO_SYSTEM_NUTTX_H
#define ZENOH_PICO_SYSTEM_NUTTX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "zenoh-pico/config.h"

/*------------------ Thread Types ------------------*/
#if Z_FEATURE_MULTI_THREAD == 1
typedef pthread_t       _z_task_t;
typedef pthread_attr_t  z_task_attr_t;
typedef pthread_mutex_t _z_mutex_t;
typedef pthread_cond_t  _z_condvar_t;
typedef pthread_mutex_t _z_mutex_rec_t;
#else
typedef void* _z_task_t;
typedef void* z_task_attr_t;
typedef void* _z_mutex_t;
typedef void* _z_condvar_t;
typedef void* _z_mutex_rec_t;
#endif

/*------------------ Clock and Time Types ------------------*/
typedef struct timespec z_clock_t;
typedef struct timeval  z_time_t;

/*------------------ Serial Types ------------------*/
typedef struct {
    int fd;
} _z_sys_net_socket_t;

typedef struct {
    const char *dev_name;
} _z_sys_net_endpoint_serial_t;
typedef _z_sys_net_endpoint_serial_t _z_sys_net_endpoint_t;
typedef _z_sys_net_socket_t _z_sys_net_socket_serial_t;

/*------------------ System Error Reporting ------------------*/
/**
 * Reports a system error.
 *
 * Implement this function to handle or log error codes as needed.
 *
 * @param errcode The error code returned from a system call.
 */
void _z_report_system_error(int errcode);

#ifdef __cplusplus
}
#endif

#endif
