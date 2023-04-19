#include <stdlib.h>
#include "client_list.h"

client_node* client_list_init()
{
	client_node* client_list = (client_node*)malloc(sizeof(client_node));
	client_list->next = NULL;
	return client_list;
}

void client_list_add(client_node* client_list, int client_fd)
{
	if(client_list == NULL)
	{
		client_list = client_list_init();
	}
	client_node* new_node = (client_node*)malloc(sizeof(client_node));
	new_node->client_fd = client_fd;
	new_node->next = client_list->next;
	client_list->next = new_node;
}

void client_list_remove(client_node* client_list, int client_fd)
{
	if(client_list == NULL)
	{
		return;
	}
	else if(client_list->client_fd == client_fd)
	{
		client_node* temp = client_list;
		client_list = client_list->next;
		free(temp);
		return;
	}
	client_node* prev = client_list;
	client_node* curr = client_list->next;
	while (curr != NULL)
	{
		if (curr->client_fd == client_fd)
		{
			prev->next = curr->next;
			free(curr);
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

