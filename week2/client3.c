#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX 1000
#define RMAX 30000

int main(int argc, char** argv) {
    int sfd;

    char sndbuf[MAX];
    char rcvbuf[RMAX];
    

    struct sockaddr_in servaddr;

    if(argc < 3) {
        printf("usage: ./client address port");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
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

    for(int i=0; i<3; i++) {
        write(sfd, sndbuf, sizeof(sndbuf));
        printf("send %d\n", i);
    }

    sleep(4);

    memset(rcvbuf, 0, sizeof(rcvbuf));
    int rcvlen = read(sfd, rcvbuf, sizeof(rcvbuf));
    printf("%s %d\n", rcvbuf, rcvlen);

    close(sfd);
    return 0;
}

