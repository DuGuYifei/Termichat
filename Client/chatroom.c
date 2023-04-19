#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "chatroom.h"
#include "message.h"
#include "param.h"

#define FINISH_SIGN "!q"

const char file_name[] = "chat.txt";

typedef struct lock_args lock_args;
struct lock_args
{
	pthread_mutex_t mutex;
	int fd;
	char* username;
	char is_finish;
};

void *read_msg(void *arg)
{
	lock_args *args = (lock_args *)arg;

	message* msg = (message*)malloc(sizeof(message));

	while (1)
	{	
		if(args->is_finish == 't')
		{
			break;
		}
		if(read(args->fd, msg, MAX_BUFFER_SIZE) < 0)
		{
			continue;
		}
		else
		{
			if(strcmp(msg->username, args->username) == 0)
			{
				continue;
			}

			// lock the fp for multithread
			pthread_mutex_lock(&args->mutex);
			printf("\033[1G%s: %s\n", msg->username, msg->message);
			pthread_mutex_unlock(&args->mutex);
			memset(msg, 0, MAX_BUFFER_SIZE);
		}
	}
	free(msg);
	pthread_exit(NULL);
}

void send_msg(int client_fd, lock_args *lock_args, char *username, char *roomname)
{
	// send msg to server
	message* msg = (message*)malloc(sizeof(message));
	strcpy(msg->username, username);
	strcpy(msg->roomname, roomname);
	while (1)
	{
		fgets(msg->message, MAX_BUFFER_SIZE, stdin);
		if(strcmp(msg->message, "\n") == 0)
		{
			continue;
		}
		if(strcmp(msg->message, FINISH_SIGN) == 0)
		{
			lock_args->is_finish = 't';
			break;
		}

		// lock the fp for multithread
		pthread_mutex_lock(&lock_args->mutex);
		printf("\033[F\033[K%s: %s\n", msg->username, msg->message);
		pthread_mutex_unlock(&lock_args->mutex);
		
		write(client_fd, msg, MAX_BUFFER_SIZE);
		shutdown(client_fd, SHUT_WR);

		memset(msg->message, 0, MAX_BUFFER_SIZE);
	}
	free(msg);
}

void chatroom(char* username, char* roomname)
{
	// socket link to server
	// multithread:
	// 1. read msg
	// 2. write msg
	// lock print for multithread

	// create socket
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd == -1)
	{
		printf("socket error\n");
		exit(EXIT_FAILURE);
	}

	// serverip:8081
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT_CHAT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	// connet
	if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("connect error\n");
		exit(EXIT_FAILURE);
	}

	// create a lock_args struct
	lock_args *args = (lock_args *)malloc(sizeof(lock_args));
	pthread_mutex_init(&args->mutex, NULL);
	args->is_finish = 'f';
	args->fd = client_fd;

	// create a new thread to read msg from server
	pthread_t thread;
	pthread_create(&thread, NULL, read_msg, (void *)args);

	// send msg to server
	send_msg(client_fd, args, username, roomname);

	pthread_join(thread, NULL);
	close(client_fd);
	free(args);
}