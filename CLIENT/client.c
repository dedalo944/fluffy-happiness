#include <ctype.h>
#include "client_head.h"

void clear (void)
{
  while ( getchar() != '\n' );
}

int main(int argc, char* argv[]){

	int conn;
  	int* prevMap=(int*)calloc(20*20,sizeof(int));
	int exit_program=0,choice;
	char reply,next;
	user client_user;
	char sck_msg[MSGLEN];
	int winmatch=0;
	char direction;

	if(argc != 3){
		printf("[Argument Error] Pattern: ./client <ip addrs> <port> !!\n");
		return 1;
	}

	conn = copen(argv[1],atoi(argv[2])); //open connection to the server

	if(conn != -1){

		do{
			do{
				printf("\nIf you are playing this game for the first time, you need to register yourself first. Otherwise you can directly login.\n");
				printf("So, Press the key listed before in order to access the game:\n\n");
				printf("\t [1]: Register yourself!\n");
				printf("\t [2]: Login!!\n");
				printf("\t [3]: Exit\n");
				scanf("%d",&choice);
			}while(choice < 1 || choice > 3);

			switch(choice){
				case 1:
					do{

						memset(client_user.username, '\0', MAXLEN);
						memset(client_user.password, '\0', MAXLEN);

						write(conn,"register",sizeof("register"));

						read(conn,sck_msg,MSGLEN);
						printf("%s",sck_msg);
						memset(sck_msg, '\0', MSGLEN);
						scanf("%s",client_user.username);
						write(conn,client_user.username,MAXLEN);

						read(conn,sck_msg,MSGLEN);
						printf("%s",sck_msg);
						memset(sck_msg, '\0', MSGLEN);
						scanf("%s",client_user.password);
						write(conn,client_user.password,MAXLEN);

						read(conn,sck_msg,MSGLEN);
						printf("\n%s\n",sck_msg);
						memset(sck_msg, '\0', MSGLEN);


					} while(strlen(client_user.username)==0 || strlen(client_user.password)==0);
					break;
				case 2:

					do{

						memset(client_user.username, '\0', MAXLEN);
						memset(client_user.password, '\0', MAXLEN);

						write(conn,"login",sizeof("login"));
						//username
						read(conn,sck_msg,MSGLEN);
						printf("%s",sck_msg);
						memset(sck_msg, '\0', MSGLEN);
						scanf("%s",client_user.username);
						write(conn,client_user.username,MAXLEN);

						//password
						read(conn,sck_msg,MSGLEN);
						printf("%s",sck_msg);
						memset(sck_msg, '\0', MSGLEN);
						scanf("%s",client_user.password);
						write(conn,client_user.password,MAXLEN);

						read(conn,sck_msg,MSGLEN);
						if(strcmp(sck_msg,"Login failed!")==0)
							printf("%s\n",sck_msg);
					} while(strcmp(sck_msg,"Login failed!")==0);

					

					memset(sck_msg, '\0', MSGLEN);


					//Starting game mechanics se winmatch non è uguale a 40 continua a giocare
					while(!winmatch){

						printf("\033[H\033[J"); //system cls
						prevMap=draw_map(conn, prevMap);

						clear();
						scanf("%c",&direction);
						write(conn,&direction,sizeof(char));

						//got a 'quit' request
						if(direction == 'q'){
							close(conn);
							winmatch=1;
						}

						//got a 'list' request
						if(direction == 'l'){
							write(conn,"log",sizeof("log"));
							memset(sck_msg, '\0', MSGLEN);
							read(conn,sck_msg,MSGLEN);
							printf("User List: %s\n",sck_msg);
							write(conn,"ok",sizeof("ok"));
							do{
								printf("\nPress <Y> to continue: ");
								clear();
								scanf("%c",&next);
							} while(tolower(next)!='y');
						}
						

						//got the time remaining
						if(direction == 't'){
							write(conn,"time",sizeof("time"));
							memset(sck_msg, '\0', MSGLEN);
							read(conn,sck_msg,MSGLEN);
							printf("Time remaining: %s \n",sck_msg);
							write(conn,"ok",sizeof("ok"));
							do{
								printf("\nPress <Y> to continue: ");
								clear();
								scanf("%c",&next);
							} while(tolower(next)!='y');
						
						
						}


						memset(sck_msg, '\0', MSGLEN);
						read(conn,sck_msg,MSGLEN);

						//result of the movement (hidden if nothing happened)
						if(strcmp(sck_msg,"no") != 0)
							printf("\n%s\n",sck_msg);

						if(strstr(sck_msg," has won!")!=NULL)
							winmatch = 1;
//						
					}

					memset(sck_msg, '\0', MSGLEN);
					memset(client_user.username, '\0', MAXLEN);
					memset(client_user.password, '\0', MAXLEN);
					exit_program = 1;
					break;
				case 3:
					exit_program = 1;
					break;
			}
		} while(!exit_program);


	} else {
		printf("Connection server: failed!\n");
		close(conn);
		return 1;
	}
	close(conn);
	return 0;
}
