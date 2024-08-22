#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#define MAX 1024*2
#define ID_SIZE 5
#define MODE_SIZE 5

int mode_check(char* src, char* dst) {
    int res = strcmp(src, dst);
    return res == 0? 1 : 0;
}

int main(int argc, char** argv) {
    int sfd;
    char buf[MAX], id[ID_SIZE], mode[MODE_SIZE];
    struct sockaddr_in servaddr;
    struct iovec vec[3];
    int opCount, opResult;

    if(argc < 3) {
        printf("usage: ./client address port");
        return -1;
    }

    vec[0].iov_base = mode;
    vec[0].iov_len = MODE_SIZE;
    vec[1].iov_base = id;
    vec[1].iov_len = ID_SIZE;
    vec[2].iov_base = buf;
    vec[2].iov_len = MAX;

    printf("Mode: ");
    scanf("%s", mode);

    if((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if(connect(sfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }

    puts("Connected....");

    if(mode_check(mode, "quit")) {
       printf("%s\n", mode);
       writev(sfd, vec, 3);
       return 0;
    }

    printf("ID: ");
    scanf("%s", id);

    if(mode_check(mode, "load")) {
       writev(sfd, vec, 3);
       int len = read(sfd, buf, MAX);
       buf[len] = 0;
       printf("%d: %s", len, buf);
       return 0;
    }


    printf("Operand count: ");
    scanf("%d", &opCount);
    buf[0] = (char)opCount;

    if(buf[0] <= 0) {
        write(sfd, buf, 1);
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

    //write(sfd, buf, 1+(opCount*4)+(opCount-1));

    writev(sfd, vec, 3);
    read(sfd, &opResult, 4);
    printf("Operation result: %d\n", opResult);

    close(sfd);
    return 0;
}