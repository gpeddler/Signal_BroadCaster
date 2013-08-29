#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <curl/curl.h>

#define MAX 255

/*

signal_callback_handler함수는 작업수행중 interrupt가 들어오는 경우
실행되는 함수이다.

지금까지 쌓아온 신호데이터를 담고 있는 data.txt로 부터 모든 데이터를
불러와 시뮬레이션 서버로 데이터를 전송해주는 역할을 수행한다.
데이터를 전송하는 방식은 http request를 이용하며 post방식으로 데이터를
전달하게 된다.

*/
void signal_callback_handler(int signum){
    printf("Caught signal %d\n",signum);

    FILE *pFile;
    char getdata[MAX];
    char postdata[MAX];
    int i;

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        pFile = fopen ("data.txt" , "r");
        
        while(fgets(getdata, sizeof(getdata), pFile) != NULL){
            fputs(getdata, pFile);
            for(i = 0; i < MAX; i++){
                if(getdata[i] == '\n'){
                    getdata[i] = '\0';
                    break;
                }
            }
            printf("%s -- %d\n", getdata, sizeof(getdata));
            sprintf(postdata, "data=%s", getdata);
            curl_easy_setopt(curl, CURLOPT_URL, "http://smart-signal.appspot.com/flush");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed: %s\n",

            curl_easy_strerror(res));
            curl_easy_cleanup(curl);
        }
        fclose (pFile);
    }
    curl_global_cleanup();

    exit(signum);
}


int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in broadcastAddr;
    unsigned short broadcastPort;
    char recvString[MAX+1];
    int recvStringLen;
 
    FILE *fp;

    signal(SIGINT, signal_callback_handler);

/*

ssb_receiver는 영상으로 부터 확인된 데이터를 받아 저장하는 역할을 수행한다.
기본적인 형태는 'timestamp; rasp index; signal index; count'로
data.txt에 저장된다.    

*/

    while(1){

        broadcastPort = 5000;

        if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
            error("socket() failed");

        memset(&broadcastAddr, 0, sizeof(broadcastAddr));
        broadcastAddr.sin_family = AF_INET;
        broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        broadcastAddr.sin_port = htons(broadcastPort);

        if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
            error("bind() failed");

        if ((recvStringLen = recvfrom(sock, recvString, MAX, 0, NULL, 0)) < 0)
            error("recvfrom() failed");

        recvString[recvStringLen] = '\n';
        recvString[recvStringLen+1] = '\0';
        printf("%s\n", recvString);

        close(sock);

        fp = fopen("data.txt","r+b");
        fseek(fp, 0, SEEK_END);
        fwrite(recvString, strlen(recvString), 1, fp);
        
        fclose(fp);
    }

    return EXIT_SUCCESS;
}
