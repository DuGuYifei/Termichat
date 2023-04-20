#include "param.h"

typedef struct message message;

struct message{
	char username[16];
	int token;
	char message[MAX_BUFFER_SIZE - 20];
};

