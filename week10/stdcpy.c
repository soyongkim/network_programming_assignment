#include <stdio.h>
#include <sys/time.h>
#define BUF_SIZE 3

int main(int argc, char *argv[])
{
	FILE * fp1;
	FILE * fp2;
	char buf[BUF_SIZE];
    struct timeval start, stop;

	fp1=fopen("news.txt", "r");
	fp2=fopen("cpy.txt", "w");

    gettimeofday(&start, NULL);
	while(fgets(buf, BUF_SIZE, fp1)!=NULL)
		fputs(buf, fp2); 
    gettimeofday(&stop, NULL);

    printf("%lu ms\n", (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);

	fclose(fp1);
	fclose(fp2);
	return 0;
}