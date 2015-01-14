//
// common.c
//
// Created by Jaewon Seo.
//

#include "common.h"

void * send_thread_main(void * arg)
{	
    thread_arg_t thread_arg = *((thread_arg_t *) arg);
    char * user_name = thread_arg.user_name;
	int socket = thread_arg.socket;

	char message_to_send[MESSAGE_SIZE];
	char message_with_name[USER_NAME_SIZE + MESSAGE_SIZE];

	while (1)
	{
		fgets(message_to_send, MESSAGE_SIZE, stdin);
		if (!strcasecmp(message_to_send, "q\n")) {
			return 0;
		}
		sprintf(message_with_name, "[%s] %s", user_name, message_to_send);
		write(socket, message_with_name, strlen(message_with_name));
	}

	return 0;
}

void * receive_thread_main(void * arg)
{
    thread_arg_t thread_arg = *((thread_arg_t *) arg);
	int socket = thread_arg.socket;

	char received_message[USER_NAME_SIZE + MESSAGE_SIZE];
	int received_message_length;

	while (1)
	{
		received_message_length = read(socket, received_message, USER_NAME_SIZE + MESSAGE_SIZE - 1);
		if (received_message_length == -1) {
			return (void *) -1;
		}
		received_message[received_message_length] = 0;
		fputs(received_message, stdout);
	}

	return 0;
}