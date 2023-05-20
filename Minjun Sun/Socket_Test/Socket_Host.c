#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 9635
#define BUF_SIZE 1024
int main(void) {
    int socket_host, socket_client;
    struct sockaddr_in host_addr, client_addr;
    socklen_t size;
    int sz_client_addr, recv_length;
    char buffer[BUF_SIZE];

    fd_set fdset;
    struct timeval tv;

	socket_host = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_host < 0) {
		printf("failed to open host socket\n");
		return -1;
	 } else {
		 printf("socket opened\n");
	 }

    // set host address
	memset(&host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(socket_host, (struct sockaddr*)&host_addr, sizeof(host_addr)) < 0) {
		printf("failed binding\n");
		return -1;
	} else {
		printf("bind success\n");
	}

	if(listen(socket_host, 5) < 0) {
		printf("filed listening");
		return -1;
	} 
	
	sz_client_addr = sizeof(client_addr);

	while(1) {
		printf("waiting for Connect...\n");
		socket_client = accept(socket_host, (struct sockaddr*)&client_addr, &sz_client_addr);
		if(socket_client < 0) {
			printf("failed accepting");
			close(socket_client);
			continue;
		} else {
			printf("server connected >> %s :%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		}

		close(socket_client);
	}


    close(socket_host);
    return 0;
}
