#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sockfd;
    struct sockaddr_in serv_addr;
    socklen_t len;
    char buf[MAX];

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
    while(1) {
        scanf("%s", buf);
        printf("%s\n", buf);
        int res = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if(res < 0) {
            perror("send error");
            break;
        }

        if(buf[0] == 'q' || buf[0] == 'Q') {
            break;
        }

        memset(buf, 0, sizeof(buf));
        res = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr*)&serv_addr, &len);
        if(res < 0) {
            perror("recvfrom error");
            break;
        }
        
        printf("%s", buf);
    }
 
    close(sockfd);
    return 0;
}