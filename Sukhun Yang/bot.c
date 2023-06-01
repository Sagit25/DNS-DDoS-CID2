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
    int bot_sock;
    char msg[BUF_SIZE];
    int str_len;
    socklen_t dns_adr_sz;
    struct sockaddr_in dns_adr, bot_adr;

    if (argc > 3 || argc < 2) {
        printf("Usage : %s <port> \n", argv[0]);
        exit(1);
    }

    bot_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (bot_sock == -1) err("UDP socket creation error");
    memset(&bot_adr, 0, sizeof(bot_adr));
    bot_adr.sin_family = AF_INET;
    bot_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    bot_adr.sin_port = htons(atoi(argv[1]));
    if (bind(bot_sock, (struct sockaddr*)&bot_adr, sizeof(bot_adr)) == -1) err("bind() error");

    while (1) {
        dns_adr_sz = sizeof(dns_adr);
        sendto(bot_sock, msg, str_len, 0, (struct sockaddr*)&dns_adr, dns_adr_sz);
        str_len = recvfrom(bot_sock, msg, BUF_SIZE, 0, (struct sockaddr*)&dns_adr, &dns_adr_sz);
    }

    close(bot_sock);
    return 0;
}
