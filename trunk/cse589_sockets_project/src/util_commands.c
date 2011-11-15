/*
 * util_commands.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */
#include "../include/util_commands.h"

static int ready_switch = 0;

command_table cmd_table[] = { { "info" }, { "ready" }, { "connect" }, { "show-conn", }, { "self-token" }, { "all-tokens" }, { "exit" }, { 0 } };

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

void run_cmd(char line[]) {
	int i = 0;//command index
	int is_cmd, is_connect; // cmd checker
	char* token = NULL;
	char temp[MAXLINE] = { '\0' };
	char conn_argv[MAXLINE] = { '\0' };

	/* parse command */
	token = strtok(line, " \t\n");
	if (token == NULL)
		i = UNKNOW_COMMAND;

	while (cmd_table[i].name != 0) {
		is_cmd = !strcmp(token, cmd_table[i].name);
		//we have to deal with other parameters for connect
		is_connect = !strcmp(cmd_table[CONNECT].name, token);
		if (is_cmd) {
			if( is_connect ){
				token = strtok(NULL, "");
				strcpy(conn_argv, token);
			}
			break;
		}
		i++;
	}
	switch (i) {
	case 0:
		//info_handler();
		break;
	case 1:
		//ready_handler();
		break;
	case 2:
		//connect_handler(conn_argv);
		break;
	case 3:
		//show_conn_handler();
		break;
	case 4:
		//self_token_handler();
		break;
	case 5:
		all_tokens_handler();
		break;
	case 6:
		exit_handler();
		break;
	case 7:
		cmd_not_found();
		break;
	default:
		break;
	}
}

void all_tokens_handler() {
	disp_all_token();
}
void exit_handler() {
	printf("see you!!!");
	exit(1);
}
void cmd_not_found() {
	printf("\n\tcommand not found\n");
}
