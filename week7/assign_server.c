#define _XOPEN_SOURCE 200
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

void error_handling(char * message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig)
{
    pid_t pid;
    int status;
    pid = waitpid(-1, &status, WNOHANG);
    printf("removed proc id: %d\n", pid);
}

int main(int argc, char** argv)
{
    int serv_sock, clnt_sock;
    struct sigaction act;
    socklen_t adr_sz;
    int str_len, state;
    //char buf[BUF_SIZE];
    unsigned char opCount; //
    char *operator;
    int result;
    int *operand;
    struct sockaddr_in serv_adr, clnt_adr;
    char buf[1024];
    socklen_t len = sizeof(clnt_adr);
    int fds[2];
    pid_t pid;
    char log[1024];
    int status;

    if(argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

    // if(argc < 2)
    // {
    //     printf("usage:./server localPort\n");
    //     return -1;
    // }

    // if((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    // {
    //     perror("socket creation failed");
    //     return -1;
    // }

    int enable = 1;
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);    //
    memset(&serv_adr, 0, sizeof(serv_adr));         //
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    //servaddr.sin_addr.s_addr = 0;
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    pipe(fds);
    pid = fork();
    if(pid == 0)
    {
        close(fds[1]);
        FILE * fp = fopen("log.txt", "w+");
        char msgbuf[BUF_SIZE];
        int len;
        int i = 0;
        while((len = read(fds[0], msgbuf, BUF_SIZE-1)) != 0)
        {
            msgbuf[len] = '\0';
            fwrite((void*)msgbuf, 1, len, fp);
        }
        fclose(fp);
        close(fds[0]);
        return 0;
    }

    while(1)
    {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &adr_sz);
        if(clnt_sock == -1)
            continue;
        else
            puts("new client connected...");

        pid = fork();
        if(pid == 0)
        {
            close(serv_sock);

            read(clnt_sock, &opCount, 1);

            buf[0] = (char)opCount;
            if(buf[0] <= 0)
            {
                close(clnt_sock);
                break;
            }
            
            operand = (int*)malloc(opCount * sizeof(int));
            for(int i = 0 ; i < opCount ; i++)
            {
                read(clnt_sock, &operand[i], 4);
            }

            operator = (char*)malloc((opCount - 1) * sizeof(char));
            for(int i = 1 ; i < opCount ; i++)
            {
                read(clnt_sock, &operator[i], 1);
            }

            result = operand[0];
            for (int i = 1 ; i < opCount ; i++)
            {
                switch (operator[i])
                {
                    case '+':
                        result += operand[i];
                        break;
                    case '-':
                        result -= operand[i];
                        break;
                    case '*':
                        result *= operand[i];
                        break;
                }
            }
            write(clnt_sock, &result, 4);

            printf("%d: ", getpid());
            sprintf(log, "%d: ", getpid());

            for(int i = 0 ; i < opCount ; i ++)
            {
                printf("%d%c", operand[i], operator[i+1]);
            }
            for(int i = 0 ; i < opCount ; i ++)
            {
                sprintf(log + strlen(log), "%d%c", operand[i], operator[i+1]);
            }

            printf("=%d\n", result);
            sprintf(log + strlen(log), "=%d\n", result);

            write(fds[1], log, strlen(log)); // 파이프를 통해 앞서 만든 자식 프로세스에게 계산 정보를 자신의 process id 와 함께 전달
            close(fds[1]);
            wait(NULL);
            close(clnt_sock);
            return 0;
        }
        else
            close(clnt_sock);
    }

    printf("Save file(%d)\n", buf[0]);
    close(serv_sock);
    return 0;
}