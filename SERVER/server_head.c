#include "server_head.h"


#define STDIN 0

char victory[TEXTLEN];
char winner[TEXTLEN];



void loadlevel(map* level){

        //generazione bound
	for(int i = 0; i<ROW; i++){
		for(int j = 0; j<COLUMN; j++){
			(level+(i*COLUMN)+j)->symbol='.';
			strcpy((level+(i*COLUMN)+j)->title,"area");
			(level+(i*COLUMN)+j)->solid=1;
			(level+(i*COLUMN)+j)->flag=0;
			(level+(i*COLUMN)+j)->points=0;
		}
	}

        
	return;
}

void log_msg(char msg[MSGLEN],int sfd,pthread_mutex_t *mutex){

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);


	struct sockaddr_in addr;
	char hostname[TEXTLEN], service[TEXTLEN];
	int addr_size=sizeof(struct sockaddr_in);;

	getpeername(sfd, (struct sockaddr *)&addr, &addr_size);

	getnameinfo((struct sockaddr*)&addr, addr_size, hostname, sizeof(hostname),service, sizeof(service), 0 );

	pthread_mutex_lock(&mutex[3]);
	FILE *fp;
	int fg = open("log.txt",O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

	if(fg < 0)
		return;
	close(fg);

	fp = fopen("log.txt","a");
	if(fp == NULL)
		return;

	fprintf(fp, "[%d-%d-%d %d:%d:%d] %s|%s: %s\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,hostname,inet_ntoa(addr.sin_addr),msg);

	fclose(fp);
	pthread_mutex_unlock(&mutex[3]);
	return;
}

void *client_manager(void *arguments){

	argm arg = *(argm*)arguments;
	pthread_mutex_t* mutex = arg.mut;
	int sdf = arg.sfd;
	
	map* level = arg.level;
	char *userlist,*tempo,sub[MSGLEN];

	log_msg("User has been connected",sdf,mutex);

	char socket_msg[MSGLEN],username[TEXTLEN],password[TEXTLEN];
	char reply;
	int x=0,y=0,flag,bad_positioning,win_flag = 0;

	while(1){
		memset(socket_msg, '\0', MSGLEN);

		if(read(sdf,socket_msg,TEXTLEN)>0){

		if(strcmp(socket_msg,"register")==0){
		   	write(sdf,"Enter your Username: ",sizeof("Enter your Username: "));
		      read(sdf,username,sizeof(username));
		      write(sdf,"Enter your Password: ",sizeof("Enter your Password: "));
		      read(sdf,password,sizeof(password));
				if(credential_saving(username,password,mutex))
					write(sdf,"You have been registered",sizeof("You have been registered") );
				else
					write( sdf,"Cannot register your account: maybe he is already registered!",sizeof("Cannot register your account: maybe he is already registered!") );

		     }

			if(strcmp(socket_msg,"login")==0){

				write(sdf,"Enter your Username: ",sizeof("Enter your Username: "));
				read(sdf,username,sizeof(username));
				write(sdf,"Enter your Password: ",sizeof("Enter your Password: "));
				read(sdf,password,sizeof(password));

				if(check_user_exsist(username,password)){
					write(sdf,"Success",sizeof("Success"));

                                        flag = 1 + (rand() % 100000);

					do{
                                            x = (rand() % ((COLUMN-1) + 1 - 1)) + 1;
                                            y = (rand() % ((ROW-1) + 1 - 1)) + 1;

					}while((level+(x*COLUMN)+y)->symbol != '.');

				
					//Drawing user on map
					pthread_mutex_lock(&mutex[2]);
					(level+(x*COLUMN)+y)->symbol = 'O';
					strcpy((level+(x*COLUMN)+y)->title,username);
					(level+(x*COLUMN)+y)->flag=flag;
					(level+(x*COLUMN)+y)->solid=0;
					(level+(x*COLUMN)+y)->points=1;
					pthread_mutex_unlock(&mutex[2]);
			
					time_t start=time(0);
					int timer=30;
							
									
				
					while(strlen(victory)==0){
												
						sendMapToClient(level,sdf,x,y);

						if(read(sdf,&reply,sizeof(char))<=0){ //if any error occurs, so thread dies.
							leave_map(level,x,y);
							pthread_exit(NULL);
						} else {
							if(reply == 'l'){
								char* territories;
								read(sdf,socket_msg,MSGLEN);
								userlist = (char*)malloc(MSGLEN*sizeof(char));
								if(socket_msg[0]=='l'&& socket_msg[1]=='o' && socket_msg[2]=='g'){
									memset(userlist,'\0',MSGLEN);

									//catching users' name from the map
									for(int i=0;i<ROW;i++){
										for(int j=0;j<COLUMN;j++){
											if((level+(i*COLUMN)+j)->symbol == 'O'){
												strcat(userlist,(level+(i*COLUMN)+j)->title);
												for(int t=0;t<ROW;t++){
													for(int u=0;u<COLUMN;u++){
														//catching the users'territories
														if((level+(i*COLUMN)+j)->flag == (level+(t*COLUMN)+u)->flag){
															territories=(char*)malloc(MSGLEN*sizeof(char));
															sprintf(territories," %d " "-" " %d ",t,u);
															strcat(userlist,territories);	
														}
													}
												}
												strcat(userlist," - \n");
												}
										}
									}

									//removing last ' - '
									substring(userlist, sub, 1, strlen(userlist)-3);
									write(sdf,sub,sizeof(sub));
									memset(socket_msg,'\0',MSGLEN);
									read(sdf,socket_msg,MSGLEN);
								}
								free(userlist);
 
                                                        } 
                                                        if(reply == 't'){
                                                        	//restituisce il  tempo rimanente

                                                        	read(sdf, socket_msg,MSGLEN);
                                                        	tempo = (char*)malloc(MSGLEN*sizeof(char));
                                                        	if(socket_msg[0]=='t'&& socket_msg[1]=='i' && socket_msg[2]=='m' && socket_msg[3]=='e'){

                                                        		memset(tempo,'\0',MSGLEN);
                                                        		//calculating time remaining and stamp on video
                                                        		long remaining= timer-(time(0)-start);
                                                        		sprintf(tempo,"%ld",remaining);
                                                        		write(sdf,tempo,sizeof(tempo));
									memset(socket_msg,'\0',MSGLEN);
									read(sdf,socket_msg,MSGLEN);
                                                        	}

                                                        	free(tempo);
                                                        }
                                                        
                                                        else {
								pthread_mutex_lock(&mutex[2]);
								move_actor(level,&x,&y,reply,&win_flag);

								pthread_mutex_unlock(&mutex[2]);
							}


							long elapsed= time(0)-start;
							
							//checking winner
							if(win_flag || elapsed>timer ){
								pthread_mutex_lock(&mutex[1]);
								//findWinner(level);
								if(win_flag==1){
									strcat(victory,winner);
									}
								else{
									findWinner(level);
									strcat(victory,winner);
								}
								if(elapsed>timer){
									log_msg(strcat(victory," has won! (due to timeout)"),sdf,mutex);
								} 
								else{
									log_msg(strcat(victory," has won!"),sdf,mutex);
								}
								pthread_mutex_unlock(&mutex[1]);
								write(sdf,victory,sizeof(victory));
								//clearing map from the player
								leave_map(level,x,y);
								//wait that other clients disconnect themselves
								wait_disconn(level);
								//restoring session
								memset(victory, '\0', MSGLEN);
								loadlevel(level);
								close(sdf);
								pthread_exit(NULL);
							} else {
								//notify victory and clearing map from the player
								if(strlen(victory)>0){
									write(sdf,victory,sizeof(victory));
									leave_map(level,x,y);
									close(sdf);
									pthread_exit(NULL);
								}
								else write(sdf,"no",sizeof("no"));
							}
						}
					}
					
					close(sdf);
					pthread_exit(NULL);

				}
				else write(sdf,"Login failed!",sizeof("Login failed!"));

		     }



		} else pthread_exit(NULL);

	}
	pthread_exit(NULL);
}

void substring(char s[], char sub[], int p, int l){
	int c = 0;

   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';
}

void wait_disconn(map* level){
	int found;
	int i,j;
	do{
		found=0;
		for(i=0;i<ROW;i++){
			for(j=0;j<COLUMN;j++){
				if((level+(i*COLUMN)+j)->symbol=='O')
					found =1;
			}
		}
	} while(found);

	return;
}


void leave_map(map* level,int x,int y){
	(level+(x*COLUMN)+y)->symbol = '.';
	strcpy((level+(x*COLUMN)+y)->title,"area");
	(level+(x*COLUMN)+y)->flag=0;
	(level+(x*COLUMN)+y)->solid=1;
	(level+(x*COLUMN)+y)->points=0;
}

void move_actor(map* level,int* startx,int* starty, char direction,int* win_flag){

	//saving user stuffs into some variables
	int x=*startx, y=*starty;
	int flag = (level+(x*COLUMN)+y)->flag;
	int points = (level+(x*COLUMN)+y)->points;
	char username[TEXTLEN];
	strcpy(username,(level+(x*COLUMN)+y)->title);

	//clearing map from the player
	(level+(x*COLUMN)+y)->symbol='.';
	strcpy((level+(x*COLUMN)+y)->title,"area");
	(level+(x*COLUMN)+y)->flag=0;
	(level+(x*COLUMN)+y)->solid=1;
	(level+(x*COLUMN)+y)->points=0;

	switch(direction){
		case 'd':
			y=y+1;
			break;
		case 'a':
			y=y-1;
			break;
		case 'w':
			x=x-1;
			break;
		case 's':
			x=x+1;
			break;
		default:
			break;
	}

	//if flag==0 || flag==startx) fare +1 a points
	if( ((level+x*COLUMN)+y)->solid == 1 && ( *startx != x || *starty!=y) ) {
		//remove past position
		if(((level+x*COLUMN)+y)->flag == 0){
			(level+((*startx)*COLUMN)+(*starty))->symbol='.';
			strcpy((level+((*startx)*COLUMN)+(*starty))->title,"area");
			(level+((*startx)*COLUMN)+(*starty))->flag= flag;
			(level+((*startx)*COLUMN)+(*starty))->solid=1;
			(level+((*startx)*COLUMN)+(*starty))->points=0;
			//add new symbol
			(level+(x*COLUMN)+y)->symbol ='O';
			strcpy((level+(x*COLUMN)+y)->title,username);
			(level+(x*COLUMN)+y)->flag=flag;
			(level+(x*COLUMN)+y)->solid=0;
			(level+(x*COLUMN)+y)->points=points+1;
			}
		else

			if(((level+x*COLUMN)+y)->flag != flag){
			int esito = lanciodadi();

			if(esito==1){  //attaccante vince
				int perdente= (level+(x*COLUMN)+y)->flag;
				
				(level+((*startx)*COLUMN)+(*starty))->symbol='.';
				strcpy((level+((*startx)*COLUMN)+(*starty))->title,"area");
				(level+((*startx)*COLUMN)+(*starty))->flag= flag;
				(level+((*startx)*COLUMN)+(*starty))->solid=1;
				(level+((*startx)*COLUMN)+(*starty))->points=0;

		   		
		   		aggiornaPerdente(perdente,level);
		   		(level+(x*COLUMN)+y)->symbol ='O';
				strcpy((level+(x*COLUMN)+y)->title,username);
				(level+(x*COLUMN)+y)->flag=flag;
				(level+(x*COLUMN)+y)->solid=0;
				(level+(x*COLUMN)+y)->points=points+1;
			}
			else {

				(level+((*startx)*COLUMN)+(*starty))->symbol='.';
				strcpy((level+((*startx)*COLUMN)+(*starty))->title,"area");
				(level+((*startx)*COLUMN)+(*starty))->flag= flag;
				(level+((*startx)*COLUMN)+(*starty))->solid=1;
				(level+((*startx)*COLUMN)+(*starty))->points=0;
				
				//add new symbol
				(level+(x*COLUMN)+y)->symbol ='O';
				strcpy((level+(x*COLUMN)+y)->title,username);
				(level+(x*COLUMN)+y)->flag=flag;
				(level+(x*COLUMN)+y)->solid=0;
				(level+(x*COLUMN)+y)->points=points;
				}
			}
			else
			{
				(level+((*startx)*COLUMN)+(*starty))->symbol='.';
				strcpy((level+((*startx)*COLUMN)+(*starty))->title,"area");
				(level+((*startx)*COLUMN)+(*starty))->flag= flag;
				(level+((*startx)*COLUMN)+(*starty))->solid=1;
				(level+((*startx)*COLUMN)+(*starty))->points=0;
				//add new symbol
				(level+(x*COLUMN)+y)->symbol ='O';
				strcpy((level+(x*COLUMN)+y)->title,username);
				(level+(x*COLUMN)+y)->flag=flag;
				(level+(x*COLUMN)+y)->solid=0;
				(level+(x*COLUMN)+y)->points=points;
			}
		
		*startx = x;
		*starty = y;
	} 
	 
	else {

		//ripristino giocatore
		(level+((*startx)*COLUMN)+(*starty))->symbol ='O';
		strcpy((level+((*startx)*COLUMN)+(*starty))->title,username);
		(level+((*startx)*COLUMN)+(*starty))->flag=flag;
		(level+((*startx)*COLUMN)+(*starty))->solid=0;
		(level+((*startx)*COLUMN)+(*starty))->points=points;
	}

	if(((level+(x*COLUMN)+y)->points) == 20 ){
		*win_flag=1;
		strcpy(winner,username);
	}
	return;
}

int lanciodadi()    // Funzione lancio dadi
{
   
   int attaccante = 1 + (rand() % 6);
   int difensore = 1 + (rand() % 6);
   if(attaccante>difensore)
	   
      return 1;

   return 0;


} 
void aggiornaPerdente(int flag_loser, map* level){
	
	for(int i=0;i<ROW;i++){
		for(int j=0;j<COLUMN;j++){
			if((level+i*COLUMN+j)->symbol=='O'){
				if((level+i*COLUMN+j)->flag==flag_loser){
					(level+i*COLUMN+j)->points=(level+i*COLUMN+j)->points-1;
					return;
				}
		    }
		}
	}
}




void findWinner(map* level){
int max=0;
for(int i=0;i<ROW;i++){
	for(int j=0;j<COLUMN;j++){
		if((level+i*COLUMN+j)->symbol=='O'){
			if(max<(level+i*COLUMN+j)->points)
				max=(level+i*COLUMN+j)->points;

			strcpy(winner,(level+i*COLUMN+j)->title);
		}
		}
	}
}


void sendMapToClient(map* level,int sdf,int x,int y){

	char map[ROW][COLUMN],buffer[MSGLEN];
	char flag_matrix[ROW][COLUMN];
	char reply[TEXTLEN];

	read(sdf,reply,TEXTLEN);
	if(strcmp(reply,"map")==0){

		for(int i=0;i<ROW;i++){
			for(int j=0;j<COLUMN;j++){
				map[i][j] = (level+(i*COLUMN)+j)->symbol;
				flag_matrix[i][j] = (level+(i*COLUMN)+j)->flag;
			}

		}

		sprintf(buffer,"%s %d %d %d %d %d %s","MAP",ROW,COLUMN,x,y,(level+(x*COLUMN)+y)->points,&(map[0][0]));
		write(sdf,buffer,MSGLEN);

		read(sdf,reply,TEXTLEN);
		if(strcmp(reply,"flag")==0){
                    write(sdf,flag_matrix,sizeof(flag_matrix));
		}

	}

	return;
}

int check_user_exsist(char username[],char password[]){ //ritorna 1 se l utente è stato trovato

	char buffer1[MSGLEN],buffer2[MSGLEN];

	FILE *fp = fopen("users.txt","r");

	if(fp == NULL)
		return 0;
	while(fscanf(fp, "%s %s", buffer1, buffer2) == 2){
		if(strcmp(buffer1,username)==0 && strcmp(buffer2,password)==0){
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);

	return 0;
}

int credential_saving(char username[],char password[],pthread_mutex_t *mutex){ //ritorna 1 se l utente è stato registrato

	int registered = check_user_exsist(username,password);
	printf("%d",registered);
	char buffer[strlen(username)+strlen(password)+3];

	if(!registered){

		int fd = open("users.txt",O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

		if(fd < 0)
			return 0;
		else{
			sprintf(buffer, "%s %s\n", username, password);
			pthread_mutex_lock(&mutex[0]);
			write(fd,buffer,strlen(buffer));
			pthread_mutex_unlock(&mutex[0]);
			return 1;
		}

		close(fd);

	} else return 0;
}

int bind_server(int port){

	struct sockaddr_in serv_addr;

	int sockfd, flag=1;
	sockfd= socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0){
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1){
			printf("Error creating new socket\n");
			return -1;
		}
	}

	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port= htons(port);
	serv_addr.sin_addr.s_addr= htonl(INADDR_ANY);
	if ( bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		return -1;
	}

	listen(sockfd, MAXCONN);
	return sockfd;
}

int accept_client(int sockfd){
	struct sockaddr_in cli_addr;
	int addr_length = sizeof(cli_addr);
	return accept(sockfd, (struct sockaddr *)&cli_addr, &addr_length);

}
