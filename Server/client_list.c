#include <stdlib.h>
#include "client_list.h"

client_node *client_list_init()
{
	client_node *client_list = (client_node *)malloc(sizeof(client_node));
	client_list->next = NULL;
	return client_list;
}

client_node *client_list_add(client_node *client_list, int client_fd)
{
	client_node *new_node = (client_node *)malloc(sizeof(client_node));
	new_node->client_fd = client_fd;
	new_node->next = client_list;
	client_list = new_node;
	return client_list;
}

client_node* client_list_remove(client_node *client_list, int client_fd)
{
	client_node *prev = NULL;
	client_node *curr = client_list;
	while (curr != NULL)
	{
		if (curr->client_fd == client_fd)
		{
			if (prev)
			{
				prev->next = curr->next;
			}else{
				client_list = curr->next;
			}
			free(curr);
			return client_list;
		}
		prev = curr;
		curr = curr->next;
	}
	return client_list;
}

int client_list_contain(client_node * client_list, int client_fd){
	client_node *cur = client_list;
	while(cur != NULL){
		if(cur->client_fd == client_fd){
			return 1;
		}
		cur = cur->next;
	}
	return 0;
}