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
int send_to_all(room *room, message *msg)
{
	client_node *cur = room->client_list;
	while (cur != NULL)
	{
		if (write(cur->pipe_fd[1], msg, MAX_BUFFER_SIZE) < 0)
		{
			printf("write error - REMOVE HIM\n");
			client_node *temp = cur;
			cur = cur->next;
			room->client_list = client_list_remove(room->client_list, temp->client_fd);
		}
		else
		{
			cur = cur->next;
		}
	}
	return 0;
}

void *handle_client(void *c_fd)
{
	int client_fd = *(int *)c_fd;
	int pipe_fd[2] = {0};

	if (pipe(pipe_fd) < 0)
	{
		printf("pipe error\n");
		close(client_fd);
		pthread_exit(NULL);
	}

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
	room->client_list = client_list_add(room->client_list, client_fd, pipe_fd);

	int epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		// handle error
		pthread_mutex_unlock(&room_list_mutex);
		perror("epoll_create1 error");
		return NULL;
	}

	struct epoll_event events[MAX_BUFFER_SIZE];
	struct epoll_event event1;
	event1.events = EPOLLIN; // focus on read event
    event1.data.fd = client_fd;
	struct epoll_event event2;
	event2.events = EPOLLIN; // focus on read event
    event2.data.fd = pipe_fd[0];
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event1);
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, pipe_fd[0], &event2);

	send_to_all(room, msg);
	pthread_mutex_unlock(&room_list_mutex);

	int pass = 1;

	while (pass)
	{
		// epoll
		int num_events = epoll_wait(epoll_fd, events, MAX_BUFFER_SIZE, -1);
		if (num_events == -1)
		{
			// have sth wrong
			continue;
		}
		// handle event
		for (int i = 0; i < num_events; i++)
		{
			int fd = events[i].data.fd;
			if (fd == client_fd)
			{
				// read
				if (read(client_fd, msg, MAX_BUFFER_SIZE) < 0)
				{
					pass = 0;
					break;
				}

				printf("%s\n", msg->message);

				if (strcmp(msg->message, "!q") == 0)
				{
					pass = 0;
					break;
				}

				// send to all clients in this room
				pthread_mutex_lock(&room_list_mutex);
				send_to_all(room, msg);
				pthread_mutex_unlock(&room_list_mutex);
			}
			else
			{
				// write to client_fd
				if (read(fd, msg, MAX_BUFFER_SIZE) < 0)
				{
					pass = 0;
					break;
				}
				if (write(client_fd, msg, MAX_BUFFER_SIZE) < 0)
				{
					pass = 0;
					break;
				}
			}
		}
		memset(&events, 0, MAX_BUFFER_SIZE * sizeof(struct epoll_event));
	}
	client_list_remove(room->client_list, client_fd);
	close(client_fd);
	close(pipe_fd[0]);
	close(pipe_fd[1]);
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