//
// Copyright (c) 2025 ZettaScale Technology
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
//   Angelo Rison K, <roskuttan@gmail.com>  - Added NuttX RTOS Support
//

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform/nuttx.h"
#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/utils/logging.h"
#include "zenoh-pico/utils/pointers.h"

#if Z_FEATURE_LINK_SERIAL == 1

// typedef struct {
//     const char *dev_name;  // Serial device name (e.g., "/dev/ttyS0")
// } _z_sys_net_endpoint_serial_t;

// typedef struct {
//     int _fd;  // File descriptor for the open serial port
// } _z_sys_net_socket_serial_t;

z_result_t _z_create_endpoint_tcp(_z_sys_net_endpoint_t *ep, const char *s_address, const char *s_port) {
    (void)s_port;  // Unused for serial transport
    if (!ep || !s_address) {
        return _Z_ERR_SYSTEM_GENERIC;
    }
    ((_z_sys_net_endpoint_serial_t *)ep)->dev_name = s_address;
    return _Z_RES_OK;
}

void _z_free_endpoint_tcp(_z_sys_net_endpoint_t *ep) {
    (void)ep;
}

z_result_t _z_open_tcp(_z_sys_net_socket_t *sock, const _z_sys_net_endpoint_t ep, uint32_t tout) {
    (void)tout;
    if (!sock) {
        return _Z_ERR_SYSTEM_GENERIC;
    }

    const char *dev_name = ((_z_sys_net_endpoint_serial_t *)&ep)->dev_name;
    int fd = open(dev_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        z_log("Failed to open serial device %s, errno: %d", dev_name, errno);
        return _Z_ERR_SYSTEM_GENERIC;
    }

    // Configure the serial port: 115200 baud, 8N1, raw mode
    struct termios tio;
    if (tcgetattr(fd, &tio) < 0) {
        close(fd);
        return _Z_ERR_SYSTEM_GENERIC;
    }
    cfsetispeed(&tio, B115200);
    cfsetospeed(&tio, B115200);
    cfmakeraw(&tio);
    if (tcsetattr(fd, TCSANOW, &tio) < 0) {
        close(fd);
        return _Z_ERR_SYSTEM_GENERIC;
    }
    ((_z_sys_net_socket_serial_t *)sock)->_fd = fd;
    return _Z_RES_OK;
}

void _z_close_tcp(_z_sys_net_socket_t *sock) {
    if (!sock) {
        return;
    }
    int fd = ((_z_sys_net_socket_serial_t *)sock)->_fd;
    if (fd >= 0) {
        close(fd);
        ((_z_sys_net_socket_serial_t *)sock)->_fd = -1;
    }
}

size_t _z_read_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    ssize_t n = read(((_z_sys_net_socket_serial_t *)&sock)->_fd, ptr, len);
    return (n < 0) ? SIZE_MAX : (size_t)n;
}

size_t _z_read_exact_tcp(const _z_sys_net_socket_t sock, uint8_t *ptr, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t n = read(((_z_sys_net_socket_serial_t *)&sock)->_fd, ptr + total, len - total);
        if (n < 0) {
            if (errno == EAGAIN) {
                break;
            }
            return SIZE_MAX;
        }
        if (n == 0)
            break;
        total += n;
    }
    return total;
}

size_t _z_send_tcp(const _z_sys_net_socket_t sock, const uint8_t *ptr, size_t len) {
    ssize_t n = write(((_z_sys_net_socket_serial_t *)&sock)->_fd, ptr, len);
    return (n < 0) ? 0 : (size_t)n;
}

#endif  // Z_FEATURE_LINK_SERIAL == 1

#if Z_FEATURE_LINK_TCP == 1
#error "TCP not supported yet on NuttX port of Zenoh-Pico"
#endif

#if Z_FEATURE_LINK_UDP_UNICAST == 1 || Z_FEATURE_LINK_UDP_MULTICAST == 1
#error "UDP not supported yet on NuttX port of Zenoh-Pico"
#endif

#if Z_FEATURE_LINK_BLUETOOTH == 1
#error "Bluetooth not supported yet on NuttX port of Zenoh-Pico"
#endif

#if Z_FEATURE_RAWETH_TRANSPORT == 1
#error "Raw ethernet transport not supported yet on NuttX port of Zenoh-Pico"
#endif
