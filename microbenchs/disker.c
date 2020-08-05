#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

#define HDDFILE "/dev/sdb"

#define DRCREADGB 20

#define SEQREADBUFGB 1
#define SEQREADGB 10

#define RANDBUFB 1
#define RANDNREAD 50000

double get_dtime(struct timeval tv)
{
    return ((double)(tv.tv_sec) + (double)(tv.tv_usec) * 1e-6);
}

double calc_elapsed(struct timeval start_tv, struct timeval end_tv)
{
    return (end_tv.tv_sec - start_tv.tv_sec) + (end_tv.tv_usec - start_tv.tv_usec) / 1e6;
}

void perror_exit(char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void show_usage(char *program_name)
{
    fprintf(stderr, "Usage: %s [-d (drop RAID cache)| -r (measure rand rps)| -s (measure seq throughput)]\n", program_name);
    exit(EXIT_FAILURE);
}

void drop_raid_cache()
{
    size_t bufbyte = SEQREADBUFGB * 1e9;

    char *blkbuf = (char *)malloc(bufbyte);

    int fd = open(HDDFILE, O_RDONLY);
    if (fd < 0)
        perror_exit("open error");

    off_t hdd_size = lseek(fd, 0, SEEK_END);
    int hdd_gb = hdd_size / 1e9;
    printf("HDD size = %d GB\n", hdd_gb);
    off_t hdd_rear = (off_t)hdd_size * (4. / 5);
    lseek(fd, hdd_rear, SEEK_SET); // set seek to the rear part of HDD

    size_t i;
    size_t nloops = DRCREADGB / SEQREADBUFGB;
    for (i = 0; i < nloops; i++)
    {
        if (read(fd, blkbuf, bufbyte) == -1)
            perror_exit("read error");
    }

    close(fd);

    if (system("sync; echo 1 > /proc/sys/vm/drop_caches") == 127)
        perror_exit("drop page cache error");
    else
        printf("droped page cache\n");

    printf("droped RAID cache\n");
}

void measure_seq_tp()
{
    struct timeval end_tv;
    struct timeval start_tv;

    size_t bufbyte = SEQREADBUFGB * 1e9;

    char *blkbuf = (char *)malloc(bufbyte);
    int fd = open(HDDFILE, O_RDONLY);

    // static char blkbuf[bufbyte] __attribute__((__aligned__(512)));
    // int fd = open(HDDFILE, O_RDONLY | O_DIRECT);

    if (fd < 0)
        perror_exit("open error");

    lseek(fd, 0, SEEK_SET);

    size_t i;
    size_t nloops = SEQREADGB / SEQREADBUFGB;

    gettimeofday(&start_tv, NULL);
    for (i = 0; i < nloops; i++)
    {
        if (read(fd, blkbuf, bufbyte) == -1)
            perror_exit("read error");
    }
    gettimeofday(&end_tv, NULL);

    close(fd);

    double real = calc_elapsed(start_tv, end_tv);
    double tp = (double)SEQREADGB * 1e3 / real; // Seq. Read Throughput (MB/sec)
    printf("%s:\n"
           "\tSeq. read throughput: %0.f MB in %f sec = %f (MB/sec)\n",
           HDDFILE, SEQREADGB * 1e3, real, tp);
}

void measure_rand_rps()
{
    struct timeval end_tv;
    struct timeval start_tv;

    size_t bufbyte = RANDBUFB;
    char blkbuf[bufbyte];
    int fd = open(HDDFILE, O_RDONLY);

    // static char blkbuf[bufbyte] __attribute__((__aligned__(512)));
    // int fd = open(HDDFILE, O_RDONLY | O_DIRECT);

    if (fd < 0)
        perror_exit("open error");

    lseek(fd, 0, SEEK_SET);

    size_t i;
    size_t nloops = RANDNREAD;
    off_t hdd_size = lseek(fd, 0, SEEK_END);
    off_t read_area = hdd_size * (4. / 5);
    gettimeofday(&start_tv, NULL);
    for (i = 0; i < nloops; i++)
    {
        lseek(fd, rand() % read_area, SEEK_SET);
        if (read(fd, blkbuf, RANDBUFB) == -1)
            perror_exit("read error");
    }
    gettimeofday(&end_tv, NULL);

    close(fd);

    double real = calc_elapsed(start_tv, end_tv);
    double rps = (double)nloops / real; // Num of Rand. read / sec
    printf("%s:\n"
           "\tRand. Num of reads: %zu times in %f sec = %f (times/sec)\n",
           HDDFILE, nloops, real, rps);
}

void argparse(int argc, char **argv)
{
    if (argc != 2)
    {
        show_usage(argv[0]);
    }

    int opt;
    while ((opt = getopt(argc, argv, "drs")) != -1)
    {
        switch (opt)
        {
        case 'd': // drop RAID Controller cache
            drop_raid_cache();
            break;
        case 'r': // measure rand access read per sec to HDD
            drop_raid_cache();
            measure_rand_rps();
            break;
        case 's': // measure seq access throughput to HDD
            drop_raid_cache();
            measure_seq_tp();
            break;
        default:
            show_usage(argv[0]);
        }
    }
}

int main(int argc, char **argv)
{
    argparse(argc, argv);

    return EXIT_SUCCESS;
}
