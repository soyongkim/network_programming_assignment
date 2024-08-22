#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/uio.h>
	
#define BUF_SIZE 1024
#define NAME_SIZE 5
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
	
char name[NAME_SIZE];
char req[BUF_SIZE];
	
int main(int argc, char *argv[])
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;

	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

    strcpy(name, argv[3]);

	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg);
	int opCount;
    struct iovec vec[2];

	vec[0].iov_base=name;
	vec[0].iov_len=NAME_SIZE;
	vec[1].iov_base=req;
	vec[1].iov_len=BUF_SIZE;

    while(1) {
        scanf("%d", &opCount);
        req[0] = (char)opCount;
        if(req[0] <= 0) {
            printf("Overflow value(%d) - client closed\n", req[0]);
            close(sock);
            exit(0);
        }

        for(int i=0; i<req[0]; i++) {
            scanf("%d", (int*)&req[(i*4)+1]);
        }

        for(int i=0; i<req[0]-1; i++) {
            scanf(" %c", &req[(opCount*4)+(1+i)]);
        }

        printf("%s\n", name);
        writev(sock, vec, 2);
        memset(req, 0, sizeof(req));
    }

	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];
	int str_len;
	while(1)
	{
		str_len=read(sock, name_msg, BUF_SIZE);
		if(str_len==-1) {
			return (void*)-1;
		}
        if(str_len==0) {
            exit(0);
        }
        printf("%s\n", name_msg);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
