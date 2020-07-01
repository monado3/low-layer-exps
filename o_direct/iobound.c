#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#define NLOOPS 100000
#define SECTSIZE 512*100

double get_dtime(struct timeval tv){
  return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 0.001 * 0.001);
}

double calc_elapsed(struct timeval start_tv, struct timeval end_tv){
    return (end_tv.tv_sec - start_tv.tv_sec) + (end_tv.tv_usec - start_tv.tv_usec) / 1000000.0;
}

void perror_exit(char* msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    struct timeval end_tv;
    struct timeval start_tv;

    gettimeofday(&start_tv, NULL);

    char sectbuf[SECTSIZE];

    int fd = open("/dev/sda", O_RDONLY);
    // int fd = open("/dev/sda", O_RDONLY|O_DIRECT);
    // int fd = open("/dev/nvme0n1p3", O_RDONLY);
    // int fd = open("/dev/nvme0n1p3", O_RDONLY|O_DIRECT);
    if (fd < 0) perror_exit("open error");

    off_t ssd_size = lseek(fd, 0, SEEK_END);
    // printf("SSD size = %ld Bytes\n", ssd_size);
    lseek(fd, 0, SEEK_SET);

    for (int i = 0; i < NLOOPS; i++){
        read(fd, sectbuf, SECTSIZE);
        // lseek(fd, rand() % ssd_size, SEEK_SET);
    }

    close(fd);

    gettimeofday(&end_tv, NULL);

    struct rusage usg;
    if (getrusage(RUSAGE_SELF, &usg)) perror_exit("getrusage");

    double user_cpu = get_dtime(usg.ru_utime);
    double sys_cpu = get_dtime(usg.ru_stime);
    double real = calc_elapsed(start_tv, end_tv);
    double other = real - user_cpu - sys_cpu;
    printf("%f,%f,%f,%f\n", real, user_cpu, sys_cpu, other);

    return EXIT_SUCCESS;
}
