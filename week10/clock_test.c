#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
    struct timeval start, stop;

    gettimeofday(&start, NULL);
    sleep(1);
    gettimeofday(&stop, NULL);

    printf("%lu ms\n", (stop.tv_sec - start.tv_sec) * 1000 + (stop.tv_usec - start.tv_usec) / 1000);
    return 0;
}