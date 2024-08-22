#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>

#define MODE_SIZE 5
#define ID_SIZE 5
#define REQ_SIZE 1024
#define DATA_MAX 1024*2
#define LOAD_MAX 1024*3

int fdsA[2], fdsB[2];
typedef struct data {
    char id[ID_SIZE];
    char req[REQ_SIZE];
} data;

int mode_check(char* src, char* dst) {
    int res = strcmp(src, dst);
    return res == 0? 1 : 0;
}

int id_check(char* src, char* dst) {
    int res = strcmp(src, dst);
    return res == 0? 1 : 0;
}

void append_req(char* res, data src) {
    char subString[DATA_MAX] = { 0, };
    sprintf(subString, "%s: %s\n", src.id, src.req);
    strcat(res, subString);
    //printf("%s", res);
}

void store_loop() {
    char mode[MODE_SIZE], id[ID_SIZE], res[LOAD_MAX];
    char notFound[] = { "Not exist\n" };
    int cnt = 0;
    data dataArray[DATA_MAX];

    while (1) {
      read(fdsA[0], mode, MODE_SIZE);
      if (mode_check(mode, "save")) {
        read(fdsA[0], dataArray[cnt].id, ID_SIZE);
        read(fdsA[0], dataArray[cnt++].req, LOAD_MAX);
      } 
      else if (mode_check(mode, "load")) {
        read(fdsA[0], id, ID_SIZE);
        memset(res, 0, sizeof(res));
        for (int i = 0; i < cnt; i++) {
          if (id_check(id, dataArray[i].id)) {
            append_req(res, dataArray[i]);
          }
        }
        //if(res[0]) write(fdsB[1], res, strlen(res));
        if(res[0]) write(fdsB[1], res, DATA_MAX);
        else write(fdsB[1], "Not exist", strlen("Not exist"));
      }
      else if (mode_check(mode, "quit")) {
        //break;
        exit(0);
      }
    }
    return;
}

int calculate(char *calcReq, char* calcRes) {
    int opCount, opResult;
    int operand[DATA_MAX];
    char operator[10];
	char result[DATA_MAX], subResult[DATA_MAX];

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
    strcpy(calcRes, result);
	printf("Operation result: %d\n", opResult);
    return opResult;
}

void multiplex_serv(char* port) {
    int serv_sock, clnt_sock, fd_max, res, status, servLoop = 1;
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    struct iovec vec[3];
    char mode[MODE_SIZE], id[ID_SIZE], calc[REQ_SIZE], calcString[DATA_MAX], load[LOAD_MAX];
    fd_set reads, cpy_reads;
    socklen_t adr_sz;

    serv_sock=socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family=AF_INET;
    serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_adr.sin_port=htons(atoi(port));

    if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) {
        perror("bind error");
        return;
    }
    if(listen(serv_sock, 5)==-1) {
        perror("listen() error");
        return;
    }

    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    fd_max=serv_sock;
    while(servLoop) {
        cpy_reads=reads;
        timeout.tv_sec=5;
        timeout.tv_usec=0;

        if((res=select(fd_max+1, &cpy_reads, 0, 0, &timeout))==-1)
            break;
        
        if(res==0)
            continue;

        for(int i=0; i<fd_max+1; i++) {
            if(FD_ISSET(i, &cpy_reads)) {
                if(i==serv_sock) {
                    adr_sz=sizeof(clnt_adr);
                    clnt_sock= accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if(fd_max<clnt_sock)
                        fd_max=clnt_sock;
                }
                else {
                    vec[0].iov_base = mode;
                    vec[0].iov_len = MODE_SIZE;
                    vec[1].iov_base = id;
                    vec[1].iov_len = ID_SIZE;
                    vec[2].iov_base = calc;
                    vec[2].iov_len = REQ_SIZE;

                    readv(i, vec, 3);
                    write(fdsA[1], mode, MODE_SIZE);
                    if(mode_check(mode, "save")) {
                        write(fdsA[1], id, ID_SIZE);
                        int res = calculate(calc, calcString);
                        //write(fdsA[1], calcString, strlen(calcString));
                        write(fdsA[1], calcString, LOAD_MAX);
                        write(i, &res, sizeof(res));
                    } 
                    else if(mode_check(mode, "load")) {
                        write(fdsA[1], id, ID_SIZE);
                        memset(load, 0, sizeof(load));
                        read(fdsB[0], load, LOAD_MAX);
                        write(i, load, strlen(load));
                    } 
                    else if(mode_check(mode, "quit")) {
                        //wait(&status);
                        wait(NULL);
                        servLoop = 0;
                        break;
                    }
                    FD_CLR(i, &reads);
                    close(i);
                }
            }
        }
    }
    close(serv_sock);
}


int main(int argc, char *argv[]) {
    pipe(fdsA); 
    pipe(fdsB);
    pid_t pid = fork();
    if(pid == 0) {
        store_loop();
    } else {
        multiplex_serv(argv[1]);
    }
    return 0;
}
