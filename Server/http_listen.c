#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_listen.h"
#include "register_room.h"
#include "param.h"

int distinguish_http_request(char *buffer, int client_fd)
{
	char *token;
	char *key;
	char *value, *psd_value;
	int idx = 0;

	// split the http request to handle it
	while ((token = strtok(buffer + idx, "\r")) != NULL)
	{
		idx += strlen(token) + 2;

		// skip the empty line before http body
		if (buffer[idx] == '\r')
		{
			idx += 2;
		}

		key = strtok(token, ":");
		if (key == NULL)
		{
			continue;
		}
		value = strtok(NULL, "\0");
		// if user or room
		if (strcmp(key, "user") == 0)
		{
			printf("%s:%s\n", key, value);

			// handle user register to database in the future
			// TODO
			return 1;
		}
		else if (strcmp(key, "room") == 0)
		{
			printf("%s:%s\n", key, value);

			// next line is the room password
			token = strtok(buffer + idx, "\r");
			idx += strlen(token) + 2;
			psd_value = strtok(token, ":");
			psd_value = strtok(NULL, "\0");
			printf("psd:%s\n", psd_value);

			// handle room register
			return register_room(value, psd_value, client_fd)? 1 : -1;
		}
	}
	return 0;
}

void *listen_register()
{
	// create server socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// bind
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT_HTTP);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)))
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// listen
	if (listen(server_fd, 10) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	// accept
	int client_fd;
	struct sockaddr_in client_addr;
	char buffer[MAX_BUFFER_SIZE] = {0};
	socklen_t client_addr_len = sizeof(client_addr);
	int check = 0;
	while (client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len))
	{
		if (client_fd < 0)
		{
			perror("accept failed");
		}

		// read
		if (read(client_fd, buffer, MAX_BUFFER_SIZE) < 0)
		{
			if (write(client_fd, "HTTP/1.1 502 ERROR\r\n\r\n", 22) < 0)
			{
				perror("write failed");
			}
			close(client_fd);
			perror("read failed");
			continue;
		}

		//printf("%s\n", buffer);
		check = distinguish_http_request(buffer, client_fd);
		if (check == 1)
		{
			if (write(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19) < 0)
			{
				perror("write failed");
			}
		}
		else if(check == -1)
		{
			if (write(client_fd, "HTTP/1.1 403 ERROR\r\n\r\nWrong Password", 36) < 0)
			{
				perror("write failed");
			}
		}
		else
		{
			if (write(client_fd, "HTTP/1.1 502 ERROR\r\n\r\n", 22) < 0)
			{
				perror("write failed");
			}
		}

		memset(buffer, 0, MAX_BUFFER_SIZE);

		// printf("%d\n", client_addr.sin_addr.s_addr);

		close(client_fd);
	}

	close(server_fd);
}