#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/uio.h>
#include <pthread.h>

#define BUF_SIZE 1024
#define NAME_SIZE 5
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++]=clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
	return 0;
}

void calculate(char *name, char *calcReq, char* res) {
    int opCount, opResult;
    int operand[BUF_SIZE];
    char operator[10];
	char result[BUF_SIZE], subResult[BUF_SIZE];

	opCount = calcReq[0];
	printf("Operand count: %d\n", opCount);

	for(int i=0; i<opCount; i++) {
		memcpy(&operand[i], calcReq + (i*4)+1, 4);
		printf("Operand %d: %d\n", i, operand[i]);
	}

	for(int i=0; i<opCount-1; i++) {
		operator[i] = calcReq[opCount*4+1+i];
	}

	opResult = operand[0];
	sprintf(result, "%d", operand[0]);
	for(int i=0; i<opCount-1; i++) {
		memset(subResult, 0, sizeof(subResult));
		sprintf(subResult, "%c%d", operator[i], operand[i+1]);
		strcat(result, subResult);
		switch (operator[i]) {
		case '+':
			opResult += operand[i+1];
			break;
		case '-':
			opResult -= operand[i+1];
			break;
		case '*':
			opResult *= operand[i+1];
		}
	}
	memset(subResult, 0, sizeof(subResult));
	sprintf(subResult, "=%d", opResult);
	strcat(result, subResult);
	printf("Operation result: %d\n", opResult);

    memset(res, 0, sizeof(res));
    sprintf(res, "[%s] %s", name, result);
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg);
	int str_len=0, i;
    char res[BUF_SIZE*2];

    struct iovec vec[2];
    char name[NAME_SIZE];
    char calcReq[BUF_SIZE];

	vec[0].iov_base=name;
	vec[0].iov_len=NAME_SIZE;
	vec[1].iov_base=calcReq;
	vec[1].iov_len=BUF_SIZE;

    // calculate a client request
    while(1) {
        if(readv(clnt_sock, vec, 2) == 0) {
			printf("client close(%d)\n", clnt_cnt);
            break;
        }

		calculate(name, calcReq, res);
        send_msg(res, strlen(res));
        memset(calcReq, 0, sizeof(calcReq));
    }

	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);

	close(clnt_sock);
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len);
	pthread_mutex_unlock(&mutx);
}
void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}