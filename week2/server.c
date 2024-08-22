#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 30000

int main(int argc, char** argv) {
    int sfd, cfd;
    char sndbuf[MAX] = { 0, };
    char rcvbuf[MAX] = { 0, };
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

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind error");
        return 1;
    }       

    // int enable = 1;
    // setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if(listen(sfd, 5) < 0) {
        perror("listen error");
        return 1;
    }    

    printf("before: %d\n", len);
    if((cfd = accept(sfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("accept error");
        return 1;
    }
    printf("after: %d\n", len);

    memset(rcvbuf, 0, sizeof(rcvbuf));

    int rcv_len, total_len = 0;
    while((rcv_len = read(cfd, rcvbuf, MAX)) != 0) {
        total_len += rcv_len;
        printf("%s %d %d\n", rcvbuf, rcv_len, total_len);
        //strcat(rcvbuf, " by server");
        write(cfd, rcvbuf, strlen(rcvbuf));
    }

    close(cfd);
    close(sfd);
    return 0;
}