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

	char buffer[BUF_SIZE] = { '\0' };
	int bytes_read = -1;
	int maxfd;
	fd_set read_set; /* set of fds to read from */

	prompt();
	//////////////////////////////////////
	while (TRUE) {
		//Clear all entries from the set.
		FD_ZERO(&read_set);
		//add std input into set
		FD_SET(STDIN_FILENO, &read_set);
		//add socket port into set
		FD_SET(listen_fd, &read_set);

		//keep track of the biggest file descriptor
		maxfd = max(udp_fd, STDIN_FILENO); /* UDP or standard input? */
		maxfd = max(maxfd, listen_fd); /* is TCP higher? */
		maxfd = max(maxfd, get_max_fd()); /* is the existed fds higher? */
		maxfd = maxfd + 1; /* give the highest priority to maxfd for select()*/

		/* now we are ready to use select() */
		if (select(maxfd, &read_set, NULL, NULL, NULL) < 0) {
			throw_exception(FATAL_ERROR ,"error running select()");
		} /* now the read_set has been renewed */

		//take connection request
		if (FD_ISSET(listen_fd, &read_set)) {
			//error occurs, accept() returns -1 and sets errno
			tcp_fd = accept(listen_fd, NULL, NULL);
			if( tcp_fd != -1 ){ //accept successfully
				//check wether we have room for this connection
				if (numof_active_conns() < MAX_CONNECTIONS) {
					add_connection(tcp_fd);
					printf("\n new connection established\n");
				} else {
					close(tcp_fd);
					fprintf(stderr, "max number of connection reached");
				}
			}else{
				if (errno == EINTR) {
					/* when a child connection closed,
					 * parent will generate a interupt signal,
					 * so jump out of the loop to select() again */
					continue;
				} else {
					throw_exception(WARNING, "error accepting new connections\n");
					exit(1);
				}
			}
		} /* end of FD_ISSET() checking */

		/* check standard input */
		if (FD_ISSET(STDIN_FILENO, &read_set)) {
			if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
				putchar('\n');
				break; /* Ctrl^D was typed, jump out of the infinite while loop */
			}
			run_cmd(buffer);
		}

		prompt();
	} /* end while (TRUE) */
	exit(0);
}

void init(int argc, char** argv) {
	//atio convert char into integer
	max_citizen_number = atoi(argv[1]);
	strncpy(tcp_port, argv[2], MAXLINE - 1);
	strncpy(udp_port, argv[3], MAXLINE - 1);
	/*different computers use different byte orderings internally for their multibyte integers
	 *convert our byte orderings to network short before sending them out*/
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
	//printf("%d", sa.sin_addr.s_addr);
	//return local human-readable IP
	inet_ntop(AF_INET, &sa.sin_addr, local_ip, INET_ADDRSTRLEN);
	memcpy(&network_ip, &sa.sin_addr, 4);
	//free up the memory, otherwise it will cause memory leak
	freeaddrinfo(servinfo);

	//start tcp and udp listening port
	listen_fd = create_tcp_socket(tcp_port);
//	udp_fd = create_udp_socket(udp_port);
	init_conn_list();
	init_token_container();
//  initialize message bag
	init_message_container();
//	// initialize broadcast bag
//	init_broc_bag();
//	// initialize leader
//	init_leader();
	////////////////////////////////
	return;
}
