#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>

#define __NR_puzzle_hash 294
#define __NR_puzzle_solve 295

int main(int argc, char** argv) {
    struct timeval start, end;
    unsigned int threshold = 4096;
    unsigned int nonce = 0;
    unsigned int target_ip = 0;
    unsigned int target_port = 0;
    unsigned char puzzle_type = 0;
    unsigned int result;
    double seconds;


    for(nonce = 0; nonce < 16; ++nonce) {
        result = syscall(__NR_puzzle_hash, nonce, target_ip, target_port, puzzle_type);
        printf("Hahs value of (%u, %u, %u %u): %u\n", nonce, target_ip, target_port, puzzle_type, result);
    }

    printf("\n\n\n\n\n\n");
    target_ip = 100;
    target_port = 2000;
    puzzle_type = 0;
    printf("Threshold: %d\n", threshold);

    for(puzzle_type = 0; puzzle_type < 4; ++puzzle_type) {
        gettimeofday(&start, NULL);
        nonce = syscall(__NR_puzzle_solve, threshold, target_ip, target_port, puzzle_type);
        gettimeofday(&end, NULL);
        seconds = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;
        printf("Puzzle nonce of (?, %u %u %u): %u (took %.3lf seconds)\n", target_ip, target_port, puzzle_type, nonce, seconds);
    }

    return 0;
}