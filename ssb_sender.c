#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 

int sock;                         /* Socket */
int broadcastPermission;          /* Socket opt to set permission to broadcast */
struct sockaddr_in broadcastAddr; /* Broadcast address */

char * send_buff;


void send_data(char * timestamp, int out_index, int current_signal, int count ){
    
    sprintf(send_buff, "%s;%d;%d;%d", timestamp, out_index, current_signal, count);
    printf("send - %s", timestamp);
    if (sendto(sock, send_buff, strlen(send_buff), 0, (struct sockaddr *) 
           &broadcastAddr, sizeof(broadcastAddr)) != strlen(send_buff))
        error("sendto() sent a different number of bytes than expected");
}

int main(int argc, char *argv[])
{
    time_t nowtime;
    char* current_timestamp;
    current_timestamp = (char *)calloc(16, sizeof(char));
    send_buff = (char *)calloc(128, sizeof(char));

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        error("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, 
          sizeof(broadcastPermission)) < 0)
        error("setsockopt() failed");

    printf("socket connect");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;/* Broadcast IP address */
    broadcastAddr.sin_port = htons(5000);         /* Broadcast port */
    
    
    for (;;) /* Run forever */
    {
        nowtime = time(NULL);
        sprintf(current_timestamp, "%ld", nowtime);

        send_data(current_timestamp, 1, 3, 1);
        sleep(1);   /* Avoids flooding the network */
    }
}
