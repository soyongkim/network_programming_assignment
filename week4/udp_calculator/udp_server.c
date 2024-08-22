#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define MAX 1024

int main(int argc, char** argv) {
    int sfd, cfd;
    char buf[MAX];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    char opCount;
    int opResult;
    int operand[MAX];
    char operator[MAX];

    if(argc < 2) {
        printf("usage:./server localPort\n");
        return 1;
    }

    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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

    while(1) {
        memset(buf, 0, sizeof(buf));
        len = sizeof(cliaddr);
        int recv_len = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*)&cliaddr, &len);
        if(recv_len < 0) {
            perror("recvfrom error");
            break;
        }

        opCount = buf[0];
        if (opCount >= 0) {
            printf("Operand count: %d\n", opCount);
            for (int i = 0; i < opCount; i++) {
              // operand[i] = buf[(i*4)+1] & 0xFF | (buf[(i*4)+2] << 8) & 0xFF00
              // | (buf[(i*4)+3] << 16) & 0xFF0000 | (buf[(i*4)+4] << 24) & 0xFF000000;

              //for(int k=0; k<4; k++) operand[i] |= (buf[(i*4)+(k+1)] << k*8) & (255 << k*8);

              //memcpy(&operand[i], buf + (i * 4) + 1, 4);
              
              //operand[i] = buf[(i*4)+1]; (답 X)

              operand[i] = *(int*)(buf + 4*i+1);
              printf("Operand %d: %d\n", i, operand[i]);
            }

            for (int i = 0; i < opCount - 1; i++) {
              //operator[i] = buf[opCount * 4 + 1 + i];
              operator[i] = buf + opCount*4 + 1 + i;
            }

            opResult = operand[0];
            for (int i = 0; i < opCount - 1; i++) {
              switch (operator[i]) {
                case '+':
                  opResult += operand[i + 1];
                  break;
                case '-':
                  opResult -= operand[i + 1];
                  break;
                case '*':
                  opResult *= operand[i + 1];
              }
            }
            printf("Operation result: %d\n", opResult);
        }

        int snd_len = sendto(sfd, &opResult, sizeof(int), 0, (const struct sockaddr*)&cliaddr, sizeof(cliaddr));
        if(snd_len < 0) {
            perror("sendto error");
            break;
        }
    }
    close(sfd);
    return 0;
}