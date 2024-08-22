#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 30000

int main(int argc, char** argv) {
    int sfd;
    char rcvbuf[MAX];
    char sndbuf[MAX];
    struct sockaddr_in servaddr;

    if(argc < 3) {
        printf("usage: ./client address port");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socekt error");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(connect(sfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return 1;
    }

    for(int i=0; i<MAX; i++) {
        sndbuf[i] = 'M';
    }

    write(sfd, sndbuf, sizeof(sndbuf));
    memset(rcvbuf, 0, sizeof(rcvbuf));
    read(sfd, rcvbuf, sizeof(rcvbuf));
    printf("Message from server: %s\n", rcvbuf);

    close(sfd);
    return 0;
}