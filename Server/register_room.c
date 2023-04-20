#include <stdlib.h>
#include <string.h>
#include "register_room.h"
#include "database_mock.h"

// sucess: >= 1 (room token)
// fail: 0
int register_room(char *roomname, char *password)
{
	pthread_mutex_lock(&room_list_mutex);
	int room_num = 0;
	if (room_list == NULL)
	{
		room_list = (room **)malloc(sizeof(room));
		room_num++;
	}
	else
	{
		// check if roomname is already used
		for (int i = 0; room_list[i] != NULL; i++)
		{
			if (strcmp(room_list[i]->roomname, roomname) == 0)
			{
				if (strcmp(room_list[i]->password, password) == 0)
				{
					pthread_mutex_unlock(&room_list_mutex);
					return room_list[i]->token;
				}
				else
				{
					pthread_mutex_unlock(&room_list_mutex);
					return 0;
				}
			}
		}
	}

	// add room to database
	if(room_num == 0)
	{
		room_num = sizeof(room_list) / sizeof(room) + 1;
	}
	room_list = (room **)realloc(room_list, sizeof(room) * room_num);
	room_list[room_num - 1] = (room *)malloc(sizeof(room));
	strcpy(room_list[room_num - 1]->roomname, roomname);
	strcpy(room_list[room_num - 1]->password, password);
	room_list[room_num - 1]->token = room_num;
	pthread_mutex_unlock(&room_list_mutex);
	return room_list[room_num - 1]->token;
}