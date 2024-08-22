#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sfd, cfd;
    char sndbuf[MAX];
    char rcvbuf[MAX];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    if(argc < 2) {
        printf("usage:./server localPort\n");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return 1;
    }

    int enable = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        return 1;
    }       

    if(listen(sfd, 5) < 0) {
        perror("listen error");
        return 1;
    }    

    printf("before: %d\n", len);

    for(int i=0; i<5; i++) {
        if((cfd = accept(sfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
            perror("accept error");
            return 1;
        }
        printf("after: %d\n", len);

        int str_len;
        memset(rcvbuf, 0, sizeof(rcvbuf));
        while((str_len = read(cfd, rcvbuf, MAX)) != 0)
            write(cfd, rcvbuf, strlen(rcvbuf));

        printf("close client(%d)\n", i);
        close(cfd);
    }


    close(sfd);
    return 0;
}