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

typedef struct file_lock_fd file_lock_fd;
struct file_lock_fd
{
	FILE *fp;
	pthread_mutex_t mutex;
	int fd;
	char is_finish;
};

void *read_msg(void *arg)
{
	file_lock_fd *file = (file_lock_fd *)arg;

	message* msg = (message*)malloc(sizeof(message));

	while (1)
	{	
		pthread_mutex_lock(&file->mutex);
		if(file->is_finish == 't')
		{
			pthread_mutex_unlock(&file->mutex);
			break;
		}
		if(read(file->fd, msg, MAX_BUFFER_SIZE) < 0)
		{
			pthread_mutex_unlock(&file->mutex);
			continue;
		}
		else
		{
			// write to file
			// lock the fp for multithread
			fprintf(file->fp, "%s: %s\n", msg->username, msg->message);
			pthread_mutex_unlock(&file->mutex);
			memset(msg, 0, MAX_BUFFER_SIZE);
		}
	}
	free(msg);
	pthread_exit(NULL);
}

void send_msg(int client_fd, file_lock_fd *file, char *username, char *roomname)
{
	// send msg to server
	message* msg = (message*)malloc(sizeof(message));
	strcpy(msg->username, username);
	strcpy(msg->roomname, roomname);
	while (1)
	{
		scanf("%s", msg->message);
		if(strcmp(msg->message, FINISH_SIGN) == 0)
		{
			pthread_mutex_lock(&file->mutex);
			file->is_finish = 't';
			pthread_mutex_unlock(&file->mutex);
			break;
		}
		// write to file
		// lock the fp for multithread
		pthread_mutex_lock(&file->mutex);
		fprintf(file->fp, "[SEDING] %s: %s\n", msg->username, msg->message);
		pthread_mutex_unlock(&file->mutex);
		
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
	// 1. read msg from server and write into "chat.txt" file
	// 2. write msg from console to send to server, and write into "chat.txt" file
	// lock file for multithread
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
	server_addr.sin_port = htons(8081);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// connet
	if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("connect error\n");
		exit(EXIT_FAILURE);
	}

	// create a file_lock_fd struct
	file_lock_fd *file = (file_lock_fd *)malloc(sizeof(file_lock_fd));
	file->fp = fopen(file_name, "a");
	if (file->fp == NULL)
	{
		printf("open file error\n");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_init(&file->mutex, NULL);
	file->is_finish = 'f';

	// create a new thread to read msg from server
	pthread_t thread;
	pthread_create(&thread, NULL, read_msg, (void *)file);

	// send msg to server
	send_msg(client_fd, file, username, roomname);

	pthread_join(thread, NULL);
	close(client_fd);
	fclose(file->fp);
	free(file);
}