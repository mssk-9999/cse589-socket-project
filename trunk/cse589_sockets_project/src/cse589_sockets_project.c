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
	struct sockaddr_storage remote_addr; // connector's address information
	socklen_t sin_size;
	fd_set read_set;

	prompt();
	//////////////////////////////////////
	while (TRUE) {
		//Clear all entries from the set.
		FD_ZERO(&read_set);
		//add std input into set
		FD_SET(STDIN_FILENO, &read_set);
		//add tcp port into set
		FD_SET(listen_fd, &read_set);
		//add contected tcp peer
		int i = 0;
		for (i = 0; i < MAX_CONNECTIONS; i++) {
			int socket_fd = get_conn_fd(i);
			if (socket_fd != -1)
				FD_SET(socket_fd, &read_set);
		}
		//add udp port into set
		FD_SET(udp_fd, &read_set);

		//keep track of the biggest file descriptor
		maxfd = max(udp_fd, STDIN_FILENO);
		maxfd = max(maxfd, listen_fd);
		maxfd = max(maxfd, get_max_fd());
		maxfd = maxfd + 1;

		if (select(maxfd, &read_set, NULL, NULL, NULL) < 0) {
			throw_exception(FATAL_ERROR, "error running select()");
		}

		//take connection request
		if (FD_ISSET(listen_fd, &read_set)) {
			//error occurs, accept() returns -1 and sets errno
			sin_size = sizeof remote_addr;
			tcp_fd = accept(listen_fd, NULL, NULL);
			if (tcp_fd != -1) { //accept successfully
				//check wether we have room for this connection
				if (count_current_connections() < MAX_CONNECTIONS) {
					add_connection(tcp_fd);
					printf("\n new connection established\n");
				} else {
					close(tcp_fd);
					fprintf(stderr, "max number of connection reached");
				}
			} else {
				if (errno == EINTR) {
					continue;
				} else {
					throw_exception(WARNING, "error accepting new connections\n");
					exit(1);
				}
			}
		} /* end of FD_ISSET() checking */

		//check standard input
		if (FD_ISSET(STDIN_FILENO, &read_set)) {
			if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
				putchar('\n');
				break;
			}
			run_cmd(buffer);
		}

		//check incoming messages from tcp peers
		for (i = 0; i < MAX_CONNECTIONS; i++) {
			/* get the ith conn_fd  */
			int connFd;
			connFd = get_conn_fd(i);
			if (connFd != -1) {
				if (FD_ISSET(connFd, &read_set)) {
					//read header
					bytes_read = readn(connFd, buffer, 11);
					//check the connection status
					if (bytes_read == 0) {
						close(connFd);
						remove_conn(i);
						printf("Connection number %d was closed by peer", i);
						continue;
					} else {
						//read body
						message_header m_head;
						init_header(&m_head);
						memcpy(m_head.id, buffer, ID_LENGTH);
						memcpy(&(m_head.type), buffer + ID_LENGTH, 1);
						memcpy(&(m_head.payload_length), buffer + ID_LENGTH + 1, 2);
						/* read the message */
						bytes_read = readn(connFd, buffer, ntohs(m_head.payload_length));
						process_received_message(&m_head, buffer, i);
					}
				}
			}
		} /* end of loop */
		//check for message from udp boradcast
		if (FD_ISSET(udp_fd, &read_set)) {
			struct sockaddr_in citizen_SA;
			socklen_t len;
			len = sizeof(citizen_SA);
			// read the message
			buffer[0] = '\0';
			bytes_read = recvfrom(udp_fd, buffer, BUF_SIZE, 0, (SA *) &citizen_SA, &len);
			printf("read %d bytes UDP packet\n", bytes_read);
			// process and display data just read
			process_salute_msg(buffer);
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
	char ip[MAXLINE] = {'\0'};
	get_public_ip(ip);
	if ((rv = getaddrinfo(ip, NULL, &hints, &servinfo)) != 0) {
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

	//start tcp and udp listening port
	listen_fd = create_tcp_socket(tcp_port);
	udp_fd = create_udp_socket(udp_port);
	init_conn_list();
	init_token_container();
	init_message_container();
	init_recieve_udp_message_container();
	init_leader();
	////////////////////////////////
	return;
}
