#include "server_head.h"


int socketfd;
pthread_attr_t attr;
pthread_mutex_t mutex[LOCKS];

//SERVER SHUTDOWN
void close_handler(){
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(mutex);
	close(socketfd);
	exit(1);
}

int main(int argc, char* argv[]){

	map *level;
	
	pthread_t tid, tid_store[100];
	argm arguments;
	int ret,sfd,k=0;


	if (argc != 2){
		printf("[Argument Error] Pattern: ./server <port> !!\n");
		return 1;
	}

	srand(time(NULL));
	
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, close_handler);

	pthread_mutex_init(&mutex[0], NULL);
	pthread_mutex_init(&mutex[1], NULL); 
	pthread_mutex_init(&mutex[2], NULL); 
	pthread_mutex_init(&mutex[3], NULL); 
	
	level=(map*)malloc(ROW*COLUMN*sizeof(map));
	loadlevel(level);
	
	ret = pthread_attr_init(&attr);
	
	if(!ret)
		ret = pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	else
		return 1;
		
		
	if (	(socketfd=bind_server( atoi(argv[1]) ) ) != -1){
		while( ( sfd=accept_client(socketfd) ) != -1 ){
			arguments.sfd = sfd;
			arguments.level = level;
			arguments.mut = mutex;		
			pthread_create(&tid,&attr, &client_manager,&arguments);
		}	
	}
	else printf("[Error] Cannot bind to that port\n");
	free(level);
	return 0;
}
