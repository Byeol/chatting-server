//
// client.c
// chatting_client
//
// Created by Jaewon Seo.
//

#include "common.h"

#define IP "127.0.0.1"
#define PORT 3001
#define USERNAME "client"

int main(int argc, char *argv[])
{
    int ret = 0;
    int client_sock, wsize;
    struct sockaddr_in server_addr;
    pthread_t send_thread_id, receive_thread_id;

    if ((client_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto leave;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port = htons(PORT);

    if ((ret = connect(client_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)))) {
        perror("connect");
        goto error;
    }

    printf("connected!\n");

    thread_arg_t thread_arg;
    thread_arg.socket = client_sock;
    thread_arg.user_name = USERNAME;

    pthread_create(&send_thread_id, NULL, send_thread_main, (void *) &thread_arg);
    pthread_create(&receive_thread_id, NULL, receive_thread_main, (void *) &thread_arg);

    pthread_join(send_thread_id, NULL);
    pthread_join(receive_thread_id, NULL);

error:
    close(client_sock);
leave:
    return ret;
}