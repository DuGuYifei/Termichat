#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "chat_server.h"
#include "database_mock.h"
#include "message.h"
#include "param.h"



// 0: success
// 1: close the connection
int send_to_all(room *room, message *msg, int client_fd)
{
	client_node *cur = room->client_list;
	while (cur != NULL)
	{
		if (write(cur->client_fd, msg, MAX_BUFFER_SIZE) < 0)
		{
			printf("write error - REMOVE HIM\n");
			client_node* temp = cur;
			cur = cur->next;
			room->client_list = client_list_remove(room->client_list, temp->client_fd);
		}else{
			cur = cur->next;
		}
	}
	return 0;
}

void *handle_client(void *c_fd)
{
	int client_fd = *(int *)c_fd;

	message *msg = (message *)malloc(sizeof(message));

	if (read(client_fd, msg, MAX_BUFFER_SIZE) < 0)
	{
		printf("read error\n");
		free(msg);
		close(client_fd);
		pthread_exit(NULL);
	}

	// find the room from database
	pthread_mutex_lock(&room_list_mutex);
	room *room = find_room_token(msg->token);

	if(client_list_contain(room->client_list, client_fd) == 0){
		// first time need register client_fd to room
		room->client_list = client_list_add(room->client_list, client_fd);
	}

	if(strcmp(msg->message, "!q") == 0){
		free(msg);
		client_list_remove(room->client_list, client_fd);
		close(client_fd);
		pthread_mutex_unlock(&room_list_mutex);
		pthread_exit(NULL);
	}

	send_to_all(room, msg, client_fd);
	pthread_mutex_unlock(&room_list_mutex);

	printf("%s\n", msg->message);
	
	free(msg);
	pthread_exit(NULL);
}

// epoll wait thread
void* epollThread(void* arg) {
    struct epoll_event events[MAX_BUFFER_SIZE];
	int epoll_fd = *(int *)arg;
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_BUFFER_SIZE, -1);
        if (num_events == -1) {
            // have sth wrong
            continue;
        }

        // handle event
        for (int i = 0; i < num_events; i++) {
            int client_fd = events[i].data.fd;
			pthread_t tid;
			pthread_create(&tid, NULL, handle_client, (void *)&client_fd);
        }
		memset(&events, 0, MAX_BUFFER_SIZE * sizeof(struct epoll_event));
    }

    return NULL;
}

void chat_listen()
{
	// create socket bind to ip and port
	// epoll listen
	// get socket of client who send message
	// get message from client
	// send to all clients in this room

	// create server socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("socket error\n");
		exit(EXIT_FAILURE);
	}

	// bind
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT_CHAT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)))
	{
		printf("[CHAT SERVER]bind error\n");
		exit(EXIT_FAILURE);
	}

	// listen
	if (listen(server_fd, 20) < 0)
	{
		printf("listen error\n");
		exit(EXIT_FAILURE);
	}

	// accept
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	// create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        // handle error
        return;
    }

	// create epoll wait thread
    pthread_t epoll_thread;
    pthread_create(&epoll_thread, NULL, epollThread, (void *)&epoll_fd);

	while (1)
	{
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd < 0)
		{
			printf("accept error\n");
		}
		struct epoll_event event;
        event.events = EPOLLIN; // focus on read event
        event.data.fd = client_fd;

		// add new client socket into wait list
		epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
	}

	pthread_join(epoll_thread, NULL);

	close(server_fd);
}