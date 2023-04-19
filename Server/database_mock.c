#include <stdlib.h>
#include <string.h>
#include "database_mock.h"

room* find_room(char* roomname){
	for(int i = 0; room_list[i] != NULL; i++){
		if(strcmp(room_list[i]->roomname, roomname) == 0){
			return room_list[i];
		}
	}
	return NULL;
}