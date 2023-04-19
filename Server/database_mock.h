#include "client_list.h"
#include "pthread.h"

typedef struct room room;

struct room {
	char roomname[16];
	char password[16];
	client_node* client_list;
};

room **room_list;
pthread_mutex_t room_list_mutex;

room* find_room(char* roomname);
