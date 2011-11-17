/*
 * util_commands.c
 *
 *  Created on: Nov 10, 2011
 *      Author: gassa
 */
#include "../include/util_commands.h"
//ready flag, after we send initial token, it will assign to 1
static int is_ready = 0;

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
	int i = 0; //command index
	int is_cmd, is_connect; // cmd checker
	char* token = NULL;
	//char temp[MAXLINE] = { '\0' };
	char conn_argv[MAXLINE] = { '\0' };

	/* parse command */
	token = strtok(line, " \t\n");
	if (token == NULL
	)
		i = UNKNOW_COMMAND;

	while (cmd_table[i].name != 0) {
		is_cmd = !strcmp(token, cmd_table[i].name);
		//we have to deal with other parameters for connect
		is_connect = !strcmp(cmd_table[CONNECT].name, token);
		if (is_cmd) {
			if (is_connect) {
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
		ready();
		break;
	case 2:
		establish_connection(conn_argv);
		break;
	case 3:
		show_connections();
		break;
	case 4:
		//self_token_handler();
		break;
	case 5:
		//all_tokens_handler();
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
	printf("see you!!!\n");
	exit(1);
}
void cmd_not_found() {
	printf("\n\tcommand not found\n");
}

void establish_connection(char* argv) {
	char *arg_array[32]; // argument list for "connect"
	int j;
	for (j = 0; j < 32; j++)
		arg_array[j] = '\0';

	int count;
	count = count_current_connections();

	int is_correct = 1;
	is_correct = get_connection_arg(argv, arg_array);
	if (is_correct == 2) {
		char ip[MAXLINE] = "";
		char port[MAXLINE] = "";
		strcpy(ip, arg_array[0]);
		strcpy(port, arg_array[1]);
		port[strlen(port) - 1] = '\0';
		if ((strcmp(ip, local_ip) == 0) && (atoi(port) == atoi(tcp_port)))
			throw_exception(ERROR, "\t can't connect local machine");
		else {
			int sock_fd;
			if (count < MAX_CONNECTIONS) {
				sock_fd = tcp_connect(ip, port);
				if (sock_fd != -1)
					add_connection(sock_fd);
			} else {
				throw_exception(ERROR, "\t maximum number of connections reached");
			}
		}
	} else {
		printf("\n\t connect [ip-address] [tcp-port]\n");
	}
}

void show_connections() {
	display_tcp_connection();
}

void ready() {
	if (is_ready != 1) {
		printf("\n\t sorry the step of sending initial token already complete.\n");
		return;
	}
	char n;
	char buffer[BUF_SIZE] = { '\0' };
	n = count_current_connections();
	if (n == 0) {
		is_ready = 0;
		printf("\t There is no any connection.\n");
		return;
	} else {
		int i, sock_fd = -1;
		putchar('\n');
		printf("\t Please, Enter 10-bits init_token (the exceeded part will be trimmed):\n");
		//let user input initial token for each peer and sent the token out through TCP connection
		for (i = 0; i < MAX_CONNECTIONS; i++) {
			sock_fd = get_conn_fd(i);
			if (sock_fd != -1) {
				char init_token[TOKEN_LENTH] = { '\0' };
				int flag = 0;
				while (flag == 0) {
					char ip[MAXLINE] = {'\0'};
					char port[MAXLINE] = {'\0'};
					get_connection_info(i, ip, port);
					printf("\t Enter the initial token for %s", ip[MAXLINE]);
					if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
						putchar('\n');
						exit(1); /* Ctrl^D */
					}

					if (buffer[0] == '0') {
						buffer[0] = '\0';
						printf("\t the input number should not start with zero.\n");
					} else if (strlen(buffer) - 1 < 10) {
						buffer[0] = '\0';
						printf("\t the input number is too short, please make sure it is exactly 10-digits.\n");
					}else {//valid input
						flag = 1;
						strncpy(init_token, buffer, TOKEN_LENTH);
						send_private_message(init_token, sock_fd);
					}
				}
			}
		} //end for loop
	}
	//set flag to 1
	is_ready = 1;
}
