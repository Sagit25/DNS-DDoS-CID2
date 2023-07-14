#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024

void err(char* str) {
    fputs(str, stderr);
    fputc('\n', stderr);
    exit(1);
}

int main(int argc, char* argv[]) {
    int target_sock;
    char msg[BUF_SIZE]; // get data from dns using UDP
    char puzzle_data[BUF_SIZE];
    int str_len;
    socklen_t dns_adr_sz;
    struct sockaddr_in dns_adr, target_adr;

    if (argc != 2) {
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }

    target_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (target_sock == -1) err("UDP socket creation error");
    memset(&target_adr, 0, sizeof(target_adr));
    target_adr.sin_family = AF_INET;
    target_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    target_adr.sin_port = htons(atoi(argv[1]));
    if (bind(target_sock, (struct sockaddr*)&target_adr, sizeof(target_adr)) == -1) err("bind() error");

    while (1) {
        dns_adr_sz = sizeof(dns_adr);
        str_len = recvfrom(target_sock, msg, BUF_SIZE, 0, (struct sockaddr*)&dns_adr, &dns_adr_sz);
        sendto(target_sock, puzzle_data, str_len, 0, (struct sockaddr*)&dns_adr, dns_adr_sz);
    }

    close(target_sock);
    return 0;
}
