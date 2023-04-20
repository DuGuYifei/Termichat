typedef struct client_node client_node; 
struct client_node{
	int client_fd;
	client_node* next;
};

client_node* client_list_init();

client_node* client_list_add(client_node* head, int client_fd);

client_node* client_list_remove(client_node* head, int client_fd);