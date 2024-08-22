#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define BUF_SIZE 3
int main(int argc, char *argv[])
{
	int fd, len, dfd;
	char buf[BUF_SIZE];

	fd=open("test.txt", O_RDONLY);
    dfd = dup(fd);

	len=read(fd, buf, sizeof(buf));
    printf("fd: %s\n", buf);
    close(fd);

    len = read(dfd, buf, sizeof(buf));
    printf("dfd: %s\n", buf);
	close(dfd);

	return 0;
}

