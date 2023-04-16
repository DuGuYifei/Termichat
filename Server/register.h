#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>

void listen_register(){
	// create server socket
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(server_fd == -1){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// bind
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in))){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// listen
	if(listen(server_fd, 10) < 0){
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	// accept
	int client_fd;
	struct sockaddr_in client_addr;
	char buffer[1024] = {0};
	socklen_t client_addr_len = sizeof(client_addr);
	while(client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len)){
		if(client_fd < 0){
			perror("accept failed");
		}

		// read
		if(read(client_fd, buffer, 1024) < 0){
			perror("read failed");
		}
		printf("%s", buffer);
		close(client_fd);
	}
}