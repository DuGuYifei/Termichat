#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "http_register.h"
#include "chatroom.h"

int main(){
	// wait user input username
	char username[16] = {0};
	printf("Please input your username: ");
	scanf("%s", username);
	
	// send http request of username
	char usermsg[21] = {0};
	strcpy(usermsg, "user:");
	strcpy(usermsg + 5, username);
	send_register(usermsg);

	// wait user input room
	char roomname[16] = {0};
	printf("Please input the roomname: ");
	scanf("%s", roomname);
	char password[16] = {0};
	printf("Please input the password of room: ");
	scanf("%s", password);

	// send http request of room
	char roommsg[50] = {0};
	strcpy(roommsg, "room:");
	strcpy(roommsg + 5, roomname);
	strcpy(roommsg + 5 + strlen(roomname), "\r\n");
	strcpy(roommsg + 5 + strlen(roomname) + 2, "password:");
	strcpy(roommsg + 5 + strlen(roomname) + 2 + 9, password);
	send_register(roommsg);

	// in chatting room
	chatroom(username, roomname);

	return 0;
}