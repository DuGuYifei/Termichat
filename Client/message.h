#include "param.h"

typedef struct message message;

struct message{
	char username[16];
	char roomname[16];
	char message[MAX_BUFFER_SIZE - 32];
};

