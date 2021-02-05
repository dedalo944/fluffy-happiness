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
//calcolo distanza tra giocatore ed oggetto
int distanza(int x1, int y1, int x2, int y2){
	int result=sqrt(((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1)));
	return result;
}




int* calculateFog(int* prevMap, int row, int col, int x, int y){

	int* maptmp= (int*) calloc(20*20,sizeof(int));
	for(int i=0;i<row;i++){
		for(int j=0;j<col;j++){
			if(distanza(x,y,i,j)<=1 || *(prevMap+i*col+j)==1){
				*(maptmp+(i*col)+j)=1;
				}
		}
	}
	return maptmp;
}


int* draw_map(int conn, int* prevMap){
	char sck_msg[MSGLEN];
	int row,col,x,y,point;

	write(conn,"map",sizeof("map"));
	read(conn,sck_msg,MSGLEN);

	if(sck_msg[0] =='M' && sck_msg[1] == 'A' && sck_msg[2] =='P'){
	memmove (sck_msg, sck_msg+3, strlen (sck_msg+3) + 1);
	sscanf(sck_msg, "%d %d %d %d %d %s",&row,&col,&x,&y,&point,sck_msg);
	int* FogMap;
	FogMap=calculateFog(prevMap, row, col, x, y);
	free(prevMap);
	prevMap=FogMap;

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
			if((level[i][j]=='1' ||  level[i][j]=='9'))
					printf(RED "%c ",level[i][j]);
		    else if((level[i][j]=='2' || level[i][j]=='8'))
					printf(BLUE "%c ", level[i][j]);
		    else if((level[i][j]=='3' || level[i][j]=='7'))
					printf(GREEN "%c ", level[i][j]);
		    else if((level[i][j]=='4' ||level[i][j]=='6'))
					printf(YELLOW "%c ", level[i][j]);
                    else if(*(prevMap+i*col+j)==1 ){
                    		printf(RESET "%c ",level[i][j]);
                    }
                    else
                    	 printf(RESET "@ ");
			k++;
		}
		printf("\n");
	}

	printf("[Points: %d]\n",point);
	}
	return prevMap;
}
