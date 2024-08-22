#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define NAME_SIZE 5
#define MAX_CLNT 256

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
static sem_t sem;

void* handle_clnt(void* arg);
void send_msg(char* msg, int len);
void error_handling(char* msg);

int main(int argc, char* argv[]) {
  int serv_sock, clnt_sock;
  struct sockaddr_in serv_adr, clnt_adr;
  int clnt_adr_sz;
  pthread_t t_id;
  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    exit(1);
  }

  sem_init(&sem, 0, 1);
  serv_sock = socket(PF_INET, SOCK_STREAM, 0);

  memset(&serv_adr, 0, sizeof(serv_adr));
  serv_adr.sin_family = AF_INET;
  serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_adr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
    error_handling("bind() error");
  if (listen(serv_sock, 5) == -1)
    error_handling("listen() error");

  while (1) {
    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    sem_wait(&sem);
    clnt_socks[clnt_cnt++] = clnt_sock;
    sem_post(&sem);

    pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
    pthread_detach(t_id);
    printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
  }
  sem_destroy(&sem);
  close(serv_sock);
  return 0;
}

void calculate(char* name, char* calcReq, char* res) {
  int opCount, opResult;
  int operand[BUF_SIZE];
  char operator[10];
  char result[BUF_SIZE], subResult[BUF_SIZE];

  opCount = calcReq[0];
  printf("Operand count: %d\n", opCount);

  for (int i = 0; i < opCount; i++) {
    memcpy(&operand[i], calcReq + (i * 4) + 1, 4);
    printf("Operand %d: %d\n", i, operand[i]);
  }

  for (int i = 0; i < opCount - 1; i++) {
    operator[i] = calcReq[opCount * 4 + 1 + i];
  }

  opResult = operand[0];
  sprintf(result, "%d", operand[0]);
  for (int i = 0; i < opCount - 1; i++) {
    memset(subResult, 0, sizeof(subResult));
    sprintf(subResult, "%c%d", operator[i], operand[i + 1]);
    strcat(result, subResult);
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
  memset(subResult, 0, sizeof(subResult));
  sprintf(subResult, "=%d", opResult);
  strcat(result, subResult);
  printf("Operation result: %d\n", opResult);

  memset(res, 0, sizeof(res));
  sprintf(res, "[%s] %s", name, result);
}

void* handle_clnt(void* arg) {
  int clnt_sock = *((int*)arg);
  char res[BUF_SIZE * 2];

  struct iovec vec[2];
  char name[NAME_SIZE];
  char calcReq[BUF_SIZE];

  vec[0].iov_base = name;
  vec[0].iov_len = NAME_SIZE;
  vec[1].iov_base = calcReq;
  vec[1].iov_len = BUF_SIZE;

  // calculate a client request
  while (1) {
    if (readv(clnt_sock, vec, 2) == 0) {
      printf("client close(%d)\n", clnt_cnt);
      break;
    }

    calculate(name, calcReq, res);
    send_msg(res, strlen(res));
    memset(calcReq, 0, sizeof(calcReq));
  }

  sem_wait(&sem);
  for (int i = 0; i < clnt_cnt; i++) 
  {
    if (clnt_sock == clnt_socks[i]) {
      while (i++ < clnt_cnt - 1)
        clnt_socks[i] = clnt_socks[i + 1];
      break;
    }
  }
  clnt_cnt--;
  sem_post(&sem);

  close(clnt_sock);
  return NULL;
}
void send_msg(char* msg, int len) {
  int i;
  sem_wait(&sem);
  for (i = 0; i < clnt_cnt; i++)
    write(clnt_socks[i], msg, len);
  sem_post(&sem);
}
void error_handling(char* msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
  exit(1);
}