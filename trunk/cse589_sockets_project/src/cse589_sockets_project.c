/*
 ============================================================================
 Name        : cse589_sockets_project.c
 Author      : Gassa
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "../include/exceptions.h"
#include "../include/util_header.h"
#include "../include/util_network.h"
#include "../include/util_commands.h"

/*
 * initiate arguments, process id and connection table;
 * fill out SA, create tcp & udp socket
 */
void init(int argc, char** argv);

/*
 * print the prompt
 */
static void prompt() {
	printf("\n~citizens> ");
	fflush(STANDAR_OUTPUT);
}

int main(int argc, char** argv) {
	check_command(argc, argv);
	init(argc, argv);
	return 1;
}

void init(int argc, char** argv) {
	//atio convert char into integer
	max_citizen_number = atoi(argv[1]);
	strncpy(tcp_port, argv[2], MAXLINE - 1);
	strncpy(udp_port, argv[3], MAXLINE - 1);
	/*different computers use different byte orderings internally for their multibyte integers
	 *convert our byte orderings to network short before sending them out
	 */
	network_udp_port = htons(atoi(udp_port));
	//print greeting message
	printf(GREETING_MSG);
	//returns the name of the computer that the program is running on
	gethostname(current_host_name, MAXLINE);

	struct addrinfo hints, *servinfo;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(current_host_name, NULL, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	// get local ip and network ip
	struct sockaddr_in sa;
	memcpy(&sa, servinfo->ai_addr, sizeof(sa));
	printf("%d", sa.sin_addr.s_addr);
	//return local human-readable IP
	inet_ntop(AF_INET, &sa.sin_addr, local_ip, INET_ADDRSTRLEN);
	memcpy(&network_ip, &sa.sin_addr, 4);
	//free up the memory, otherwise it will cause memory leak
	freeaddrinfo(servinfo);

	// create tcp & udp port
//	listen_fd = create_tcp_socket(tcp_port);
//	udp_fd = create_udp_socket(udp_port);
//	// initialize connection table
//	init_conn_list();
//	// initialize token bag
//	init_token_bag();
//	// initialize message bag
//	init_msg_bag();
//	// initialize broadcast bag
//	init_broc_bag();
//	// initialize leader
//	init_leader();

	return;
}
