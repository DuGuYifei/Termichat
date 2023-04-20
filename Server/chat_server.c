#include <sys/socket.h>
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
			room->client_list = client_list_remove(room->client_list, cur->client_fd);
			return 1;
		}

		cur = cur->next;
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
		close(client_fd);
		pthread_exit(NULL);
	}

	// find the room from database
	pthread_mutex_lock(&room_list_mutex);
	room *room = find_room_token(msg->token);

	// first time need register client_fd to room
	room->client_list = client_list_add(room->client_list, client_fd);
	send_to_all(room, msg, client_fd);
	pthread_mutex_unlock(&room_list_mutex);

	int check = 0;
	while (1)
	{
		// read
		if (read(client_fd, msg, MAX_BUFFER_SIZE) < 0)
		{
			printf("read error\n");
			close(client_fd);
			break;
		}

		printf("%s\n", msg->message);

		// send to all clients in this room
		pthread_mutex_lock(&room_list_mutex);
		check = send_to_all(room, msg, client_fd);
		pthread_mutex_unlock(&room_list_mutex);
		if(check){
			break;
		}
	}
	close(client_fd);
	free(msg);
	pthread_exit(NULL);
}

void chat_listen()
{
	// create socket bind to ip and port
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
	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(client_addr);

	while (1)
	{
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd < 0)
		{
			printf("accept error\n");
		}

		// create thread to handle client
		pthread_t tid;
		pthread_create(&tid, NULL, handle_client, (void *)&client_fd);
	}

	close(server_fd);
}