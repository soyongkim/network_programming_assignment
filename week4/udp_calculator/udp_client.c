#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sfd;
    char buf[MAX];
    struct sockaddr_in servaddr;
    socklen_t len;
    int opCount, opResult;

    if(argc < 3) {
        printf("usage: ./client address port");
        return -1;
    }

    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socekt error");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    printf("Operand count: ");
    scanf("%d", &opCount);
    buf[0] = (char)opCount;

    if(buf[0] < 0) {
        sendto(sfd, buf, 1, 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
        close(sfd);
        return 0;
    }

    for(int i=0; i<opCount; i++) {
        printf("Operand %d: ", i);
        scanf("%d", (int*)&buf[(i*4)+1]);
    }

    for(int i=0; i<opCount-1; i++) {
        printf("Operator %d: ", i);
        scanf(" %c", &buf[(opCount*4)+(i+1)]);
    }

    //printf("%d\n", 1+(opCount*4)+(opCount-1));
    sendto(sfd, buf, 1+(opCount*4)+(opCount-1), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    
    len = sizeof(servaddr);
    recvfrom(sfd, &opResult, 4, 0, (struct sockaddr*)&servaddr, &len);
    printf("Operation result: %d\n", opResult);

    close(sfd);
    return 0;
}