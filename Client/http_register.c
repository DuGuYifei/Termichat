#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_register.h"
#include "param.h"

int send_register(char *msg)
{
	// create socket
	int client_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (client_fd == -1)
	{
		printf("socket error\n");
		exit(EXIT_FAILURE);
	}

	// serverip:8080
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT_HTTP);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

	// connet
	if(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		printf("connect error\n");
		exit(EXIT_FAILURE);
	}

	// send
	char buffer[MAX_BUFFER_SIZE] = {0};
	strcpy(buffer, "POST / HTTP/1.1\r\n\r\n");
	write(client_fd, strcpy(buffer + 20, msg), strlen(msg));
	shutdown(client_fd, SHUT_WR);

	memset(buffer, 0, MAX_BUFFER_SIZE);

	if (read(client_fd, buffer, MAX_BUFFER_SIZE) < 0)
	{
		perror("read failed");
		exit(EXIT_FAILURE);
	}

	printf("---------------\n%s\n", buffer);
	if(buffer[13] == 'O' && buffer[14] == 'K')
	{
		printf("Register success\n---------------\n\n");
	}
	else
	{
		printf("Register failed\n\n");
		exit(EXIT_FAILURE);
	}
	close(client_fd);
	return atoi(buffer + 19);
}