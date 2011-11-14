/*
 * util_commands.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */
#include "../include/util_commands.h"

void check_command(int argc, char** argv) {
	if (argc != 4) {
		printf("citizen <n> <tcp-port> <udp-port>\n");
		exit(1);
	}

	if (atoi(argv[2]) < 1024 && atoi(argv[2]) < 1024) {
		fprintf(stderr, "error: port number should more than 1024\n");
		exit(1);
	}
}
