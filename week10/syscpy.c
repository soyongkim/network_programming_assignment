#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#define BUF_SIZE 3

int main(int argc, char *argv[])
{
	int fd1, fd2, len;
	char buf[BUF_SIZE];
    struct timeval start, stop;

	fd1=open("news.txt", O_RDONLY);
	fd2=open("cpy.txt", O_WRONLY|O_CREAT|O_TRUNC);

    gettimeofday(&start, NULL);
	while((len=read(fd1, buf, sizeof(buf)))>0)
		write(fd2, buf, len);
    gettimeofday(&stop, NULL);
	
    printf("%lu ms\n", (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);


	close(fd1);
	close(fd2);
	return 0;
}