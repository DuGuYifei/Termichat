#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "chat_server.h"
#include "database_mock.h"
#include "message.h"
#include "param.h"

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
		printf("bind error\n");
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

	message *msg = (message *)malloc(sizeof(message));
	while (client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len))
	{
		if (client_fd < 0)
		{
			printf("accept error\n");
		}

		// read
		if (read(client_fd, msg, MAX_BUFFER_SIZE) < 0)
		{
			printf("read error\n");
			continue;
		}

		printf("%s\n", msg->message);

		// find the room from database
		// send to all clients in this room
		pthread_mutex_lock(&room_list_mutex);
		room *room = find_room(msg->roomname);

		client_node *cur = room->client_list;
		while (cur != NULL)
		{
			printf("%i\n", cur->client_fd);
			if (write(cur->client_fd, msg, MAX_BUFFER_SIZE) < 0)
			{
				printf("write error\n");
			}
			printf("write success\n");
			shutdown(cur->client_fd, SHUT_WR);
			cur = cur->next;
		}
		pthread_mutex_unlock(&room_list_mutex);
	}
	free(msg);
}