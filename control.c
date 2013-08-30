#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void* connection_handler( void* );

int main( int argc, char* argv[] )
{
	int socket_desc, client_sock, c, *new_sock;
	struct sockaddr_in server, client;

	/* Create socket */
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
    }
	puts("Socket created");
     
	/* Prepare the sockaddr_in structure */
	server.sin_family		= AF_INET;
	server.sin_addr.s_addr	= INADDR_ANY;   
	server.sin_port			= htons( 8888 );
     
	/* Bind */
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		/* print the error message */
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	/* Listen */
	listen(socket_desc , 3);
     
	/* Accept and incoming connection */
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		puts("Client connection accepted");

		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;

		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}

		/* Now join the thread , so that we dont terminate before the thread */
		//pthread_join( sniffer_thread , NULL);
		puts("Handler assigned");
	}

	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
	/* Get the socket descriptor */
	int sock = *(int*)socket_desc;
	int read_size;
	char message[100], client_message[100];

	/* Receive a message from client */
	while( (read_size = recv(sock , client_message , 100 , 0)) > 0 )
	{
		if ( strcmp( client_message, "get" ) == 0 ) {
			const int signalInterval = 30;
			time_t t;
			struct tm* pt;
		
			time( &t );
			pt = localtime( &t );

			int h = pt->tm_hour;
			int m = h*60 + pt->tm_min;
			int s = m*60 + pt->tm_sec;

			int signalQ[4] = { 0, 1, 2, 3 };
			int qIdx = s / signalInterval % 4;
			message[0] = '0' + signalQ[ qIdx ];
			message[1] = 0;
		}
		else if ( strcmp( client_message, "set" ) == 0 ) {
			strcpy( message, "abcd" );
		}

		/* Send the message back to client */
		write(sock , message , strlen( message ) );
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}

	/* Free the socket pointer */
	free(socket_desc);

	return 0;
}
