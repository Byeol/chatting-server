//
// common.h
//
// Created by Jaewon Seo.
//

#ifndef CHATTING_COMMON_H
#define CHATTING_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#define MESSAGE_SIZE 1024
#define USER_NAME_SIZE 20
#define EPOLL_SIZE 50

typedef struct {
	char * user_name;
	int socket;
} thread_arg_t;

void * send_thread_main(void * arg);
void * receive_thread_main(void * arg);

#endif