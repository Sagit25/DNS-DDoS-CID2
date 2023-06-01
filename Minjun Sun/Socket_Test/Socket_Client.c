#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define HOST_IP "192.168.55.63"

#define PORT 80
#define BUF_SIZE 1024
int main(void) {
    int socket_fd;
    struct sockaddr_in host_addr;
    socklen_t size;
    int recv_length;
    char buffer[BUF_SIZE];

    fd_set fdset;
    struct timeval tv;

    // set host address
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = inet_addr(HOST_IP);

    // open socket as nonblock to set timeout shorter
    socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);

    printf("Try Connecting >> %s : %d\n", inet_ntoa(host_addr.sin_addr), ntohs(host_addr.sin_port));
    connect(socket_fd, (struct sockaddr_in*)&host_addr, sizeof(host_addr));

    FD_ZERO(&fdset);
    FD_SET(socket_fd, &fdset);
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    // wait socket_fd changed till timeout
    if (select(socket_fd+1, NULL, &fdset, NULL, &tv) == 1) {
        int so_error;
        socklen_t len = sizeof(so_error);

	  //check socket_fd info
        getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0) { 
            printf("CONNECTED >> %s: %d\n", HOST_IP, PORT);
        }
        else {
            printf("Connection Failed\n");
            close(socket_fd);
            return 0;
        }
    }
    else { // timeout
        printf("Connecting TImeout\n");
        close(socket_fd);
        return 0;
    }

    recv_length = recv(socket_fd, &buffer, BUF_SIZE, 0);
    printf("From host : %s\n", buffer);

    for (int i = 0; i < 5; i++) {
        printf("message to send >>");
        scanf("%s",buffer);
        send(socket_fd, buffer, strlen(buffer) + 1, 0);

        recv_length = recv(socket_fd, &buffer, BUF_SIZE, 0);
        printf("From host : %s\n", buffer);
    }

    close(socket_fd);
    return 0;
}
