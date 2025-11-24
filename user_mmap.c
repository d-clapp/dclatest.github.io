#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PAGE_SIZE 4096

int main(int argc, char *argv[])
{
    int fd;
    int i;
    unsigned char *p_map;
    char *name = "Daniel"; 

    fd = open("/proc/mydir/myinfo", O_RDWR);
    if (fd < 0) {
        printf("open fail\n");
        exit(1);
    } else {
        printf("open successfully by %s\n", name);
    }

    p_map = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (p_map == MAP_FAILED) {
        printf("mmap fail\n");
        close(fd);
        exit(1);
    }

    for (i = 0; i < 12; i++) {
        printf("%u\n", p_map[i]);
    }

    printf("Printed by %s\n", name);

    munmap(p_map, PAGE_SIZE);
    close(fd);
    return 0;
}
