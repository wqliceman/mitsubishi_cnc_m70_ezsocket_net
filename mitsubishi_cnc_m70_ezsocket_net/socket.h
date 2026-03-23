/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2022-2026 wqliceman
 * GitHub: iceman
 * Email: wqliceman@gmail.com
 *
 * This file is part of the Mitsubishi CNC M70 EZSocket communication library.
 * See the LICENSE file in the project root for full license information.
 */

#ifndef __SOCKET_H_
#define __SOCKET_H_

#include "utill.h"

int socket_send_data(int fd, void* ptr, int nbytes);
int socket_recv_data(int fd, void* ptr, int nbytes);
int socket_recv_data_one_loop(int fd, void* ptr, int nbytes);
int socket_open_tcp_client_socket(char* ip, short port);
void socket_close_tcp_socket(int sockFd);

#endif //__SOCKET_H_
