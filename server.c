//
// server.c
// chatting_server
//
// Created by Jaewon Seo,
//

#include "common.h"

#define PORT 3001
#define MAX_DATA 100
#define USERNAME "server"

pthread_t epoll_thread_id;

typedef struct epoll_arg_t {
    int fd;
    struct epoll_event * evts;
    struct epoll_event evt;
    int server_fd;
} epoll_arg_t;

void * epoll_thread_main(void * arg)
{
    int socket_count = 0;
    int reserved_socket_conut = 6;

    int client_socket;
    int server_as_client_socket;

    char buffer[MESSAGE_SIZE];
    int received_message_length;

    int event_cnt;
    epoll_arg_t epoll_arg = *((epoll_arg_t *) arg);

    while (1)
    {
        event_cnt = epoll_wait(epoll_arg.fd, epoll_arg.evts, EPOLL_SIZE, -1);
        if (event_cnt == -1)
        {
            perror("epoll_wait");
            break;
        }

        int event_no;
        for (event_no = 0; event_no < event_cnt; event_no++)
        {
            int client_socket = epoll_arg.evts[event_no].data.fd;

            if (client_socket == epoll_arg.server_fd) {
                struct sockaddr_in client_address;
                socklen_t address_size = sizeof(client_address);

                server_as_client_socket = accept(client_socket, (struct sockaddr*) &client_address, &address_size);

                if (server_as_client_socket < 0)
                    break;

                fcntl(server_as_client_socket, F_SETFL, fcntl(server_as_client_socket, F_GETFL, 0) | O_NONBLOCK);

                epoll_arg.evt.events = EPOLLIN | EPOLLET;
                epoll_arg.evt.data.fd = server_as_client_socket;
                epoll_ctl(epoll_arg.fd, EPOLL_CTL_ADD, server_as_client_socket, &epoll_arg.evt);

                socket_count++;
                continue;
            }

            while (1)
            {
                received_message_length = read(client_socket, buffer, MESSAGE_SIZE);
                buffer[received_message_length] = 0;

                if (received_message_length == 0) {
                    epoll_ctl(epoll_arg.fd, EPOLL_CTL_DEL, client_socket, NULL);
                    close(client_socket);
                    printf("Connection closed: %d \n", client_socket);
                    socket_count--;
                    break;
                }

                if (received_message_length < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                    break;
                }

                int target_socket;
                for (target_socket = reserved_socket_conut; (target_socket - reserved_socket_conut) < socket_count; target_socket++) {
                    if (client_socket == target_socket) {
                        continue;
                    }
                    write(target_socket, buffer, MESSAGE_SIZE);
                }
            }
        }
    }

    return 0;
}

int server_as_client()
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
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

    pthread_join(epoll_thread_id, NULL);
    pthread_join(send_thread_id, NULL);
    pthread_join(receive_thread_id, NULL);

error:
    close(client_sock);
leave:
    return ret;
}

int main()
{
    int ret = -1, rsize;
    int server_sock;
    int accepted_sock;

    struct sockaddr_in server_addr;
    socklen_t server_addr_size = sizeof(server_addr);

    char read_buf[MAX_DATA];

    if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        goto leave;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if ((ret = bind(server_sock, (struct sockaddr *) &server_addr, server_addr_size))) {
        perror("bind");
        goto error;
    }

    if ((ret = listen(server_sock, 1))) {
        perror("listen");
        goto error;
    }

    epoll_arg_t epoll_arg;
    epoll_arg.server_fd = server_sock;
    epoll_arg.fd = epoll_create(EPOLL_SIZE);
    epoll_arg.evts = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);

    int sock_option_value = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &sock_option_value, sizeof(sock_option_value));

    fcntl(server_sock, F_SETFL, fcntl(server_sock, F_GETFL, 0) | O_NONBLOCK);

    epoll_arg.evt.events = EPOLLIN;
    epoll_arg.evt.data.fd = server_sock;
    epoll_ctl (epoll_arg.fd, EPOLL_CTL_ADD, server_sock, &epoll_arg.evt);

    pthread_create (&epoll_thread_id, NULL, epoll_thread_main, (void *) &epoll_arg);

    server_as_client();

error:
    close(server_sock);
leave:
    return ret;
}
