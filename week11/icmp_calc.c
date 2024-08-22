#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <resolv.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define REQMAX 1024
#define PACKETSIZE 64
#define ICMP_CALC_REQ 20
#define ICMP_CALC_RES 21
struct packet {
  struct icmphdr hdr;
  char msg[PACKETSIZE - sizeof(struct icmphdr)];
};

int pid = -1;
struct protoent* proto = NULL;

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void* b, int len) {
  unsigned short* buf = b;
  unsigned int sum = 0;
  unsigned short result;

  for (sum = 0; len > 1; len -= 2)
    sum += *buf++;
  if (len == 1)
    sum += *(unsigned char*)buf;
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  result = ~sum;
  return result;
}

int calculate(char *data) {
    int opCnt, result;
    int operand[REQMAX];
    char operator[REQMAX];

    opCnt = data[0];
    
    if(opCnt <= 1) {
        //printf("Overflow count(%d)\n", opCnt);
        return -1;
    }
    //printf("Operand Count: %d\n", opCnt);
    for(int i=0; i<opCnt; i++) {
        memcpy(&operand[i], data + (i*4)+1, sizeof(int)); 
        //printf("Operand[%d]: %d\n", i, operand[i]);
    }

    for(int i=0; i<opCnt-1; i++) {
        operator[i] = data[(opCnt*4)+1+i];
        //printf("Operator[%d]: %c\n", i, operator[i]);
    }

    result = operand[0];
    for(int i=0; i<opCnt-1; i++) {
        switch (operator[i]) {
        case '+':
            result += operand[i+1];
            break;
        case '-':
            result -= operand[i+1];
            break;
        case '*':
            result *= operand[i+1];
        }
    }

    //printf("Result: %d\n\n", result);
    return result;
}

void display(int sd, void* buf, int bytes) {
    int result;
    struct packet pckt;
    struct iphdr* ip = buf;
    struct icmphdr* icmp = buf + ip->ihl * 4;
    //char* data = buf + (ip->ihl * 4) + 8;
    char* data = buf + (ip->ihl * 4) + sizeof(*icmp);
    struct sockaddr_in r_addr;

    r_addr.sin_family = AF_INET;
    r_addr.sin_port = 0;
    r_addr.sin_addr.s_addr = ip->saddr;
    if (icmp->type == ICMP_CALC_REQ) {
        printf("ICMP_CALC_REQ: type[%d] id[%d] seq[%d]\n\n",
               icmp->type, icmp->un.echo.id,
               icmp->un.echo.sequence);
        result = calculate(data);
        bzero(&pckt, sizeof(pckt));
        pckt.hdr.type = ICMP_CALC_RES;
        pckt.hdr.un.echo.id = pid;
        memcpy(pckt.msg, &result, sizeof(result));
        pckt.hdr.un.echo.sequence = icmp->un.echo.sequence;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
        if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr,
                   sizeof(r_addr)) <= 0)
        perror("sendto");
    }

    if (icmp->type == ICMP_CALC_RES) {
        memcpy(&result, buf + (ip->ihl * 4) + 8, sizeof(result));
        printf("ICMP_CALC_RES: type[%d] id[%d] seq[%d] calc result: %d\n\n",
               icmp->type, icmp->un.echo.id, icmp->un.echo.sequence,
               result);
        if (result == -1) {
            exit(0);
        }
    }
}

void listener(void) {
  int sd;
  const int val = 255;
  struct sockaddr_in addr;
  unsigned char buf[REQMAX];

  sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
  if (sd < 0) {
    perror("socket");
    exit(0);
  }
  for (;;) {
    int bytes, len = sizeof(addr);
    bzero(buf, sizeof(buf));
    bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &len);
    if (bytes > 0) {
      display(sd, buf, bytes);
    } else
      perror("recvfrom");
  }
  exit(0);
}

void calc(struct sockaddr_in* addr) {
  const int val = 255;
  int cnt = 1;
  int sd, opCnt;
  struct packet pckt;

  sd = socket(PF_INET, SOCK_RAW, proto->p_proto);
  if (sd < 0) {
    perror("socket");
    return;
  }
  if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
    perror("Set TTL option");
  if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0)
    perror("Request nonblocking I/O");
  for (;;) {
    bzero(&pckt, sizeof(pckt));
    pckt.hdr.type = ICMP_CALC_REQ;
    pckt.hdr.un.echo.id = pid;
    pckt.hdr.un.echo.sequence = cnt++;

    memset(pckt.msg, 0, sizeof(pckt.msg));
    scanf("%d", &opCnt);
    pckt.msg[0] = (char)opCnt;

    for (int i = 0; i < pckt.msg[0]; i++) {
      scanf("%d", (int*)&pckt.msg[(i * 4) + 1]);
    }
    for (int i = 0; i < pckt.msg[0] - 1; i++) {
      scanf(" %c", &pckt.msg[(opCnt * 4) + (1 + i)]);
    }

    pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
    if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr,
               sizeof(*addr)) <= 0)
      perror("sendto");

    if(opCnt <= 1) {
        return;
    }
  }
}

int main(int count, char* argv[]) {
  struct sockaddr_in addr;

  if (count != 2) {
    printf("usage: %s <addr>\n", argv[0]);
    exit(0);
  }
  if (count > 1) {
    pid = getpid();
    proto = getprotobyname("ICMP");
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (fork() == 0) {
      listener();
    } else
      calc(&addr);
    wait(0);

  } else
    printf("usage: icmp_calc <addr>\n");
  return 0;
}
