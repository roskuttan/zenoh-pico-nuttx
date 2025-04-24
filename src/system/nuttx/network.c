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

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <stdlib.h>

#include "zenoh-pico/system/link/serial.h"
#include "zenoh-pico/system/platform.h"
#include "zenoh-pico/utils/logging.h"
#include "zenoh-pico/utils/pointers.h"

#if Z_FEATURE_LINK_SERIAL == 1

z_result_t _z_open_serial_from_pins(_z_sys_net_socket_t *sock, uint32_t txpin, uint32_t rxpin, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(txpin);
    (void)(rxpin);
    (void)(baudrate);


    ret = _Z_ERR_GENERIC;

    return ret;
}
//open the serial device
z_result_t _z_open_serial_from_dev(_z_sys_net_socket_t *sock, char *dev, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;

    int fd = open(dev , O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd<0){
        return _Z_ERR_GENERIC;
    }

    struct termios config;
    if(tcgetattr(fd, &config) < 0){
        close(fd);
        ret = _Z_ERR_GENERIC;
        return ret;
    }

    //set baudrate
    speed_t z_baudrate_t;
    switch (baudrate){
        case 9600:
            z_baudrate_t = B9600;
            break;
        case 19200:
            z_baudrate_t = B19200;
            break;
        case 38400:
            z_baudrate_t = B38400;
            break;
        case 57600:
            z_baudrate_t = B57600;
            break;
        case 115200:
            z_baudrate_t = B115200;
            break;
        case 230400: 
            z_baudrate_t = B230400; 
            break;
        default:
            z_baudrate_t = B115200;
            break;
    }
    if (cfsetspeed(&config, z_baudrate_t) < 0){
        close(fd);
        ret = _Z_ERR_GENERIC;
        return ret;
    }
    //clear existing settings for consistency
    config.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS);
    config.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    //set required settings
    config.c_cflag |= CS8;

    if(tcsetattr(fd, TCSANOW, &config) < 0){
        close(fd);
        ret = _Z_ERR_GENERIC;
        return ret;
    }
    sock -> fd = fd;
    return _z_connect_serial(*sock);
}
//dummy function to avoid compilation error
z_result_t _z_listen_serial_from_pins(_z_sys_net_socket_t *sock, uint32_t txpin, uint32_t rxpin, uint32_t baudrate) {
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(txpin);
    (void)(rxpin);
    (void)(baudrate);


    ret = _Z_ERR_GENERIC;

    return ret;
}

z_result_t _z_listen_serial_from_dev(_z_sys_net_socket_t *sock, char *dev, uint32_t baudrate) {
    printf("[INFO]>> serial listen stub called on %s", dev);
    z_result_t ret = _Z_RES_OK;
    (void)(sock);
    (void)(dev);
    (void)(baudrate);

    // 
    ret = _Z_ERR_GENERIC;

    return ret;
}
//read the serial device
size_t _z_read_serial_internal(const _z_sys_net_socket_t sock, uint8_t *header, uint8_t *ptr, size_t len){
    uint8_t *raw_buf = z_malloc(_Z_SERIAL_MAX_COBS_BUF_SIZE);
    if(raw_buf == NULL){
        return SIZE_MAX;
    }
    size_t rb = 0;
    for (size_t i = 0; i < _Z_SERIAL_MAX_COBS_BUF_SIZE; i++){
        ssize_t res;
        while((res = read(sock.fd, &raw_buf[i],1))!=1){
            if(res<0){
                if(errno == EAGAIN){
                    continue;
                } else {
                    z_free(raw_buf);
                    return SIZE_MAX;
                }
            }
            break;
        }
        rb++;
        if(raw_buf[i] == (uint8_t)0x00){
            break;
        }
    }
    uint8_t *tmp_buf = z_malloc(_Z_SERIAL_MFS_SIZE);
    if(tmp_buf == NULL){
        z_free(raw_buf);
        return SIZE_MAX;
    }
    size_t ret = _z_serial_msg_deserialize(raw_buf, rb, ptr, len, header, tmp_buf, _Z_SERIAL_MFS_SIZE);
    z_free(raw_buf);
    z_free(tmp_buf);
    if(ret == SIZE_MAX){
        return SIZE_MAX;
    }
    return ret;
}
//write to the serial device
size_t _z_send_serial_internal(const _z_sys_net_socket_t sock, uint8_t header, const uint8_t *ptr, size_t len){
    uint8_t *tmp_buf = z_malloc(_Z_SERIAL_MFS_SIZE);
    if(tmp_buf == NULL){
        return SIZE_MAX;
    }
    uint8_t *raw_buf = z_malloc(_Z_SERIAL_MAX_COBS_BUF_SIZE);
    if(raw_buf == NULL){
        z_free(tmp_buf);
        return SIZE_MAX;
    }
    size_t frame_len = _z_serial_msg_serialize(raw_buf, _Z_SERIAL_MAX_COBS_BUF_SIZE, ptr, len, header, tmp_buf, _Z_SERIAL_MFS_SIZE);
    if(frame_len == SIZE_MAX){
        z_free(tmp_buf);
        z_free(raw_buf);
        return SIZE_MAX;
    }
    size_t bytes_written = 0;
    while (bytes_written < frame_len){
        ssize_t sent = write(sock.fd, raw_buf + bytes_written, frame_len - bytes_written);
        if(sent < 0){
            if(errno == EAGAIN){
                usleep(100);
                continue;
            } else {
                z_free(tmp_buf);
                z_free(raw_buf);
                return SIZE_MAX;
            }
        }
        bytes_written += sent;
    }
    z_free(tmp_buf);
    z_free(raw_buf);
    return len;
}
//close the serial device
void _z_close_serial(_z_sys_net_socket_t *sock) {
    if(sock && sock->fd >= 0){
        close(sock->fd);
        sock->fd = -1;
    }
}
z_result_t _z_socket_set_non_blocking(const _z_sys_net_socket_t *sock) {
    (void)(sock);
    return _Z_ERR_GENERIC;
}

void _z_socket_close(_z_sys_net_socket_t *sock) {
    (void)(sock);
}

z_result_t _z_socket_accept(const _z_sys_net_socket_t *sock_in, _z_sys_net_socket_t *sock_out) {
    (void)(sock_in);
    (void)(sock_out);
    return _Z_ERR_GENERIC;
}

z_result_t _z_socket_wait_event(void *peers, _z_mutex_rec_t *mutex) {
    (void)(peers);
    (void)(mutex);
    return _Z_ERR_GENERIC;
}
#endif // Z_FEATURE_LINK_SERIAL == 1

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