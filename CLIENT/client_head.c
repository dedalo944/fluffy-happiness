#include "client_head.h"


int copen(char *ip_addrs, int port){
	struct sockaddr_in serv_addr;
    	struct hostent *server;
	int sockfd;

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	server=gethostbyname(ip_addrs);
	serv_addr.sin_family= AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port=htons(port);
   	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
		printf("[Error]");
		return -1;
	}
    	return sockfd;
}



int* draw_map(int conn, int* prevMap){
	char sck_msg[MSGLEN];
	int row,col,x,y,point;

	write(conn,"map",sizeof("map"));
	read(conn,sck_msg,MSGLEN);

	if(sck_msg[0] =='M' && sck_msg[1] == 'A' && sck_msg[2] =='P'){
	memmove (sck_msg, sck_msg+3, strlen (sck_msg+3) + 1);
	sscanf(sck_msg, "%d %d %d %d %d %s",&row,&col,&x,&y,&point,sck_msg);
	

	char level[row][col];
	int flag[row][col];
	int k=0;

	//MAP DOWNLOADED

	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
                    level[i][j] = sck_msg[k];
                    k++;

		}
	}
	write(conn,"flag",sizeof("flag"));
	read(conn,sck_msg,MSGLEN);
	//MAP STORED
	k=0;
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			flag[i][j] = (int)sck_msg[k];
			
                     if(*(prevMap+i*col+j)==1 ){
                    		printf(RESET "%c ",level[i][j]);
                    }
                    else
                    	 printf(RESET "%c ",level[i][j]);
			k++;
		}
		printf("\n");
	}

	printf("[Points: %d]\n",point);
	}
	return prevMap;
}
