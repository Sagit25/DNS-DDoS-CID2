#define _GNU_SOURCE

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define sys_puzzle_print_policy 296
#define sys_puzzle_add_policy 297
#define sys_puzzle_edit_policy 298
#define sys_puzzle_detail_policy 299
#define sys_puzzle_update_policy 300
#define sys_puzzle_print_cache 301
#define sys_puzzle_remake_seed 302
#define sys_puzzle_get_type 303
#define sys_puzzle_set_type 304
#define sys_puzzle_print_dns 305
#define sys_puzzle_set_dns 306

int main(int argc, char *argv[]) {

	int temp;
	if(argc <= 1) {
		printf( "Command List : \n");
		printf( " | print_policy\n");
		printf( " | print_policy [ip]\n");
		printf( " | add_policy [ip] [length] [threshold]\n");
		printf( " | edit_policy [ip] [seed] [length] [threshold]\n");
		printf( " | update_policy [config file]\n");
		printf( " | print_cache\n");
		printf( " | remake_seed [ip]\n");
		printf( " | puzzle_type\n");
		printf( " | puzzle_type [NONE|LOCAL|DNS]\n");
		printf( " | dns_info\n");
		printf( " | dns_info [ip] [port]\n");
		return 0;
	}
	if(strcmp("print_policy", argv[1]) == 0) {
		switch(argc) {
		case 2:
			syscall(sys_puzzle_print_policy);
			goto done;
		case 3:
			syscall(sys_puzzle_detail_policy, inet_addr(argv[2]));
			goto done;
		default:
			goto print_error;
		}
	}
	if(strcmp("add_policy", argv[1]) == 0) {
		if(argc != 5)
			goto print_error;
		syscall(sys_puzzle_add_policy, inet_addr(argv[2]), atoi(argv[3]), atoi(argv[4]));
		goto done;
	}
	if(strcmp("edit_policy", argv[1]) == 0) {
		if(argc != 6)
			goto print_error;
		syscall(sys_puzzle_add_policy, inet_addr(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
		goto done;
	}
	if(strcmp("update_policy", argv[1]) == 0) {
		/*
		if(argc != 5)
			goto print_error;
		syscall(sys_puzzle_add_policy, inet_addr(argv[2]), atoi(argv[3]), atoi(argv[4]));
		*/
		printf( "not implemented yet\n");
		goto done;
	}
	if(strcmp("print_cache", argv[1]) == 0) {
		if(argc != 2)
			goto print_error;
		syscall(sys_puzzle_print_cache);
		goto done;
	}
	if(strcmp("remake_seed", argv[1]) == 0) {
		if(argc != 3)
			goto print_error;
		printf("new seed for %s >> %ld\n", argv[2], syscall(sys_puzzle_remake_seed, inet_addr(argv[2])));
		goto done;
	}
	if(strcmp("puzzle_type", argv[1]) == 0) {
		switch(argc) {
		case 2:
			switch(syscall(sys_puzzle_get_type)) {
			case 1:
				printf( "puzzle type >> none\n");
				goto done;
			case 2:
				printf( "puzzle type >> local\n");
				goto done;
			case 3:
				printf( "puzzle type >> dns\n");
				goto done;
			default:
				printf( "puzzle type >> unknown\n");
				goto done;
			}
		case 3:
			if(strcmp("NONE", argv[2]) == 0)
				temp = 1;
			else if (strcmp("LOCAL", argv[2]) == 0)
				temp = 2;
			else if (strcmp("DNS", argv[2]) == 0)
				temp = 3;
			else
				goto print_error;
			syscall(sys_puzzle_set_type, temp);
			goto done;
		default:
			goto print_error;
		}
	}
	if(strcmp("dns_info", argv[1]) == 0) {
		switch(argc) {
		case 2:
			syscall(sys_puzzle_print_dns);
			goto done;
		case 4:
			syscall(sys_puzzle_set_dns, inet_addr(argv[2]), atoi(argv[3]));
			syscall(sys_puzzle_print_dns);
			goto done;
		default:
			goto print_error;
		}
	}

print_error:
	printf( "call without arguments for manual\n");
done:
	return 0;
}

