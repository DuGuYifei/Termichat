#include <pthread.h>
#include <stdlib.h>
#include "http_listen.h"
#include "chat_server.h"
#include "database_mock.h"

int main(){
	// TODO
	// give function to control when to finish the threads of sockets.

	pthread_mutex_init(&room_list_mutex, NULL);

	// get http request - new thread
	pthread_t http_listen_thread;
	pthread_create(&http_listen_thread, NULL, listen_register, NULL);

	// chat server - new thread
	chat_listen();

	// join the threads;
	pthread_join(http_listen_thread, NULL);

	free(room_list);
	return 0;
}