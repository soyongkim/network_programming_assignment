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

    if(argc < 3) {
        printf("usage: ./client address port");
        return -1;
    }

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socekt error");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(connect(sfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    while(1) {
        fputs("Input message(Q to quit):", stdout);
        fgets(buf, MAX, stdin);
        printf("%s", buf);

        if(!strcmp(buf, "q\n") || !strcmp(buf, "Q\n"))
            break;

        int str_len;
        if((str_len = write(sfd, buf, strlen(buf))) < 0) {
            perror("write error");
            return -1;
        }

        memset(buf, 0, sizeof(buf));
        if((str_len = read(sfd, buf, sizeof(buf))) < 0) {
            perror("read error");
            return -1;
        }

        buf[str_len-1] = 0;
        printf("Message from server: %s\n", buf);
    }

    close(sfd);
    return 0;
}