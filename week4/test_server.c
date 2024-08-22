#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 30000

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t len;
    char buf[MAX];

    if(argc < 2) {
        printf("usage: ./server PORT");
        return -1;
    }


    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if(bind(sockfd, (const struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind error");
        return -1;
    }

    int rcv_len, total_len = 0;
    for(int i=0; i<3; i++) { 
        memset(buf, 0, sizeof(buf));
        len = sizeof(cli_addr);
        rcv_len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&cli_addr, &len);
        total_len += rcv_len;
        printf("rcv_len: %d total_len: %d\n", rcv_len, total_len);

        sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr*)&cli_addr, len);

        puts("sleep 1s...");
        sleep(1);
    }

    close(sockfd);
    return 0;
}

