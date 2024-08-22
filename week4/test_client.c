#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX 1000
#define RMAX 30

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t len;
    char sndbuf[MAX];
    char rcvbuf[RMAX];
    //char* buf = "Send Message";

    if(argc < 3) {
        printf("usage: ./client IP PORT");
        return -1;
    }


    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    len = sizeof(serv_addr);
    //scanf("%s", buf);
    //printf("%s\n", buf);

    for(int i=0; i<MAX; i++) {
        sndbuf[i] = 'M';
    }

    for(int i=0; i<3; i++) {
        sendto(sockfd, sndbuf, strlen(sndbuf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        printf("send %d\n", i);
    }

    sleep(4);

    // memset(rcvbuf, 0, sizeof(rcvbuf));
    // int rcvlen = recvfrom(sockfd, rcvbuf, sizeof(rcvbuf), 0, (struct sockaddr*)&serv_addr, &len);
    // printf("%s %d\n", rcvbuf, rcvlen);

    for(int i=0; i<3; i++) {
        memset(rcvbuf, 0, sizeof(rcvbuf));
        int rcvlen = recvfrom(sockfd, rcvbuf, sizeof(rcvbuf), 0, (struct sockaddr*)&serv_addr, &len);
        printf("%s %d\n", rcvbuf, rcvlen);
    }

    close(sockfd);
    return 0;
}