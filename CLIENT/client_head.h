#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <math.h>

#ifndef C_HD_

#define MSGLEN 512
#define MAXLEN 25

#define RED "\x1B[31m"
#define BLUE "\x1B[34m"
#define YELLOW "\x1B[33m"
#define GREEN "\x1B[32m"
#define RESET "\x1B[0m"

struct userstr{
	char username[MAXLEN];
	char password[MAXLEN];
};

typedef struct userstr user;

int* draw_map(int conn, int* prevMap);
int copen(char *ip_addrs, int port); //open connection to server


#endif
