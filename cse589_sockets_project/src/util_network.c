#include  <sys/socket.h>  /* basic socket definitions               */
#include  <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include  <netdb.h>       /* name and address conversion            */
#include  <arpa/inet.h>   /* inet_pton/ntop                         */

#include "../include/util_network.h"
#include "../include/util_commands.h"

/*
 **************** GLOBLE VARIABLES *******************
 * */
static int is_peerToken_determined = 0;
// my own message for broadcasting
static broadcast_bag my_broc_msg;
// leader infomation
static broadcast_bag leader;

/*
 **************** CONNECTION TABLE *******************
 * */
static connection_container connection_list[MAX_CONNECTIONS];
/*
 **************** INITIAL TOKEN BAG ******************
 * */
static token_container init_token_container_list[MAX_CONNECTIONS];
/*
 **************** MESSAGE BAG ************************
 * */
static message_container msg_container_list[MAX_CITIZEN_NUM];
/*
 **************** PEER TOKEN BAG *********************
 * */
static broadcast_bag broc_bag[MAX_CITIZEN_NUM];

/*
 * init_broc_bag(): initialize broadcast bag
 * */
void init_broc_bag() {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		broc_bag[i].ip = '\0';
		broc_bag[i].udp_port = '\0';
		broc_bag[i].peer_token[0] = '\0';
		broc_bag[i].r_ip[0] = '\0';
		broc_bag[i].r_port[0] = '\0';
		broc_bag[i].isUsed = 0;
	}
}
/*
 * init_single_broc_bag(): initialize broadcast bag
 * */
void init_leader() {
	leader.ip = '\0';
	leader.udp_port = '\0';
	leader.peer_token[0] = '\0';
	leader.r_ip[0] = '\0';
	leader.r_port[0] = '\0';
	leader.isUsed = 0;
}
/*
 * init_header(): initialize the header
 * */
void init_header(message_header *mh) {
	mh->id[0] = '\0';
	mh->type = '\0';
	mh->payload_length = 0;
}
/*
 * check_received_token() : check to determined peer token
 * */
int check_peer_token() {
	int i, a, counter = 0;
	a = count_current_connections();
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (init_token_container_list[i].isUsed == 1)
			counter++;
	}
	if (a == counter)
		return 1;
	else
		return 0;
}
int get_self_token(char token[]) {
	if (strlen(my_broc_msg.peer_token) != 0) {
		strncpy(token, my_broc_msg.peer_token, TOKEN_LENTH);
		return 1;
	} else
		return 0;
}

void init_token_container() {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		init_token_container_list[i].isUsed = 0;
		init_token_container_list[i].token_list[0] = '\0';
	}
}

void init_message_container() {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		msg_container_list[i].isUsed = 0;
		msg_container_list[i].id[0] = '\0';
	}
}
/*
 * add_msg(): add new msg to message bag
 * */
void add_msg(char *id) {
	int i, flag = 0;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (msg_container_list[i].isUsed == 0 && flag == 0) {
			memcpy(msg_container_list[i].id, id, ID_LENGTH);
			if (0) {
				printf("add_msg(): message id: %s, add it into msg_bag\n", msg_container_list[i].id);
			}
			flag = 1;
		}
	}
}
/*
 * add_broc_msg(): add new broadcast message to the broc_bag
 * */
void add_broc_msg(char peer_token[], uint16_t udp_port, uint32_t ip, char r_port[]) {
	int i;
	int flag = 0;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (broc_bag[i].isUsed == 0 && flag == 0) {
			// add ip in big-endian
			broc_bag[i].ip = ip;
			// add ip in little-endian
			char s_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(broc_bag[i].ip), s_ip, INET_ADDRSTRLEN);
			strncpy(broc_bag[i].r_ip, s_ip, strlen(s_ip));
			// add udp port in big-endian
			broc_bag[i].udp_port = udp_port;
			// add udp port in little-endian
			strncpy(broc_bag[i].peer_token, peer_token, TOKEN_LENTH);
			strncpy(broc_bag[i].r_port, r_port, strlen(r_port));
			// add signal
			broc_bag[i].isUsed = 1;
			flag = 1;
		}
	}
	// check if broadcast bag have n citizen's peer token
	if (numof_peer_token() == max_citizen_number) {
		find_leader();
		printf("\tleader(%s) is from %s:%d\n", leader.peer_token, leader.r_ip, ntohs(leader.udp_port));
		send_salute_message();
	}
}
/*
 * show_broc_bag(): display all broadcast message
 * */
void show_broc_bag() {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (broc_bag[i].isUsed == 1) {
			char source_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(broc_bag[i].ip), source_ip, INET_ADDRSTRLEN);
			printf("Broc_bag: i have token: <%s>  ip: <%s> udp_port: <%d>\n", broc_bag[i].peer_token, source_ip, ntohs(broc_bag[i].udp_port));
		}
	}
}
/*
 * show_received_token(): display all received tokens
 * */
void show_received_token() {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (init_token_container_list[i].isUsed == 1)
			printf("I have %s in token_bag\n", init_token_container_list[i].token_list);
	}
}
/*
 * add_init_token(): add token to token_bag, waiting to be sorted
 * */
void add_init_token(char init_token[]) {
	int i;
	int flag = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (flag == 0 && init_token_container_list[i].isUsed == 0) {
			strncpy(init_token_container_list[i].token_list, init_token, TOKEN_LENTH);
			init_token_container_list[i].isUsed = 1;
			flag = 1;
		}
	}
}
/*
 *  token_cmp(): find biggest token as its peertoken
 * */
void cmp_init_token() {
	int z;
	long long int temp = 0;
	long long int max_t = 0;
	for (z = 0; z < MAX_CONNECTIONS; z++) {
		if (init_token_container_list[z].isUsed == 1) {
			temp = strtoll(init_token_container_list[z].token_list, NULL, 0);
			max_t = max(max_t, temp);
		}
	}
	sprintf(my_broc_msg.peer_token, "%lld", max_t);
}
/*
 *  find_leader(): find biggest peer token as our leader
 * */
void find_leader() {
	int z;
	long long int temp1 = 0;
	long long int temp2 = 0;

	for (z = 0; z < MAX_CITIZEN_NUM; z++) {
		if (broc_bag[z].isUsed == 1) {
			temp1 = strtoll(broc_bag[z].peer_token, NULL, 0);
			temp2 = strtoll(leader.peer_token, NULL, 0);
			if (temp1 >= temp2)
				leader = broc_bag[z];
		}
	}
}
/*
 * parse argument line for connect
 * */
int get_connection_arg(char * arg_line, char *arg_array[]) {
	char * p;
	int count = 0;
	p = strtok(arg_line, " ");
	while (p && strcmp(p, " ") != 0) {
		arg_array[count] = p;
		count++;
		p = strtok(NULL, " ");
	}
	return count;
}
/*
 * init_conns_list() : initialize the connection list
 */
void init_conn_list() {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		connection_list[i].connection_status = 0; //disconnected
		connection_list[i].sock_fd = -1;
		(connection_list[i].ip)[0] = '\0';
		(connection_list[i].name)[0] = '\0';
		(connection_list[i].local_port)[0] = '\0';
		(connection_list[i].remote_port)[0] = '\0';
	}
}
/*
 * disp_all_token(): used for display formatted infomations
 * , used by all-token()
 * */
void disp_all_token() {
	if (numof_peer_token() == 0) {
		printf("\n\tno peer token received\n");
		fflush(stdout);
		return;
	}
	int a = strlen("IP"); /* IP field length          */
	int b = strlen("remote port"); /* remote port field length   */
	int c = strlen("token"); /* token field length  */

	// find out how long each field needs to be
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (broc_bag[i].isUsed == 1) {
			a = max(a, strlen(broc_bag[i].r_ip));
			b = max(b, strlen(broc_bag[i].r_port));
			c = max(c, strlen(broc_bag[i].peer_token));
		}
	}
	putchar('\n');

	int n = a + b + c + 8;
	printf(" %-*s | %-*s | %-*s\n", a, "IP", b, "remote port", c, "token");
	int j;
	for (j = 0; j < n; j++) {
		putchar('-');
	}

	putchar('\n');

	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (broc_bag[i].isUsed == 1) {
			printf(" %-*s | %-*s | %-*s\n", a, broc_bag[i].r_ip, b, broc_bag[i].r_port, c, broc_bag[i].peer_token);
		}
	}
	fflush(stdout);
}
/*
 * disp_tcp_conn(): used for display formatted infomations
 * , used by show-conn()
 * */
void display_tcp_connection() {
	if (count_current_connections() == 0) {
		printf("\n\t No active TCP connection. \n");
		fflush(stdout);
		return;
	}
	int ip_length = strlen("IP address");
	int hostname_length = strlen("Host name");
	int local_port_length = strlen("Local port");
	int remote_port_length = strlen("Remote port");
	//calculate the length of each connection info
	int i = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1) {
			ip_length = max(ip_length, strlen(connection_list[i].ip));
			hostname_length = max(hostname_length, strlen(connection_list[i].name));
			local_port_length = max(local_port_length, strlen(connection_list[i].local_port));
			remote_port_length = max(remote_port_length, strlen(connection_list[i].remote_port));
		}
	}
	putchar('\n');

	int n = ip_length + hostname_length + local_port_length + remote_port_length + 18;
	//print header
	printf(" %-*s | %-*s | %-*s | %-*s | %-*s\n", 5, "cnnID", ip_length, "IP address", hostname_length, "Host name", local_port_length, "Local port", remote_port_length, "Remote port");
	//print break line
	for (i = 0; i < n; i++) {
		putchar('-');
	}
	putchar('\n');
	//print table content
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1) {
			printf(" %5d | %-*s | %-*s | %-*s | %-*s\n", i, ip_length, connection_list[i].ip, hostname_length, connection_list[i].name, local_port_length, connection_list[i].local_port,
					remote_port_length, connection_list[i].remote_port);
		}
	}
	fflush(stdout);
}
/*
 * get_max_fd(): return maximum sock_fd of all active connections
 *    -1 if no connection is active
 */
int get_max_fd() {
	int i, maxfd = -1;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1) {
			maxfd = max(maxfd, connection_list[i].sock_fd);
		}
	}
	return maxfd;
}
/*
 * set_active_conns(fd_set*): mark all sock_fd of active connections for select()
 */
void set_active_conns(fd_set* read_set) {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1) {
			FD_SET(connection_list[i].sock_fd, read_set);
		}
	}
}
/*
 * numof_peer_token(): returns the number of active TCP connections
 */
int numof_peer_token() {
	int i, counter = 0;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (broc_bag[i].isUsed == 1)
			counter++;
	}
	return counter;
}
/*
 * numof_active_conns(): returns the number of active TCP connections
 */
int count_current_connections() {
	int i, counter = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1)
			counter++;
	}
	return counter;
}
/*
 * get_conn_info(): givin cid, return ip, tcp_port
 * */
void get_connection_info(int conn_id, char ip[], char port[]) {
	strncpy(ip, connection_list[conn_id].ip, MAXLINE);
	strncpy(port, connection_list[conn_id].remote_port, MAXLINE);
}
/*
 * get_conn_fd(int conn_id): return connection fd of connection whose id is conn_id
 *   -1 if conn_id is inactive
 */
int get_conn_fd(int conn_id) {
	int conn = -1;
	if (connection_list[conn_id].connection_status == 1) {
		conn = connection_list[conn_id].sock_fd;
	}
	return conn;
}
/*
 * remove_conn(int conn_id): remove the connection whose ID is conn_id
 *   return -1 if the connection is not active
 *   return the connection's socket_fd if it is
 */
int remove_conn(int conn_id) {
	int i, fd = -1;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if ((i == conn_id) && (connection_list[i].connection_status == 1)) {
			connection_list[i].connection_status = 0;
			fd = connection_list[i].sock_fd;
			break;
		}
	}
	return fd;
}
/*
 * is_new_msg(uint8_t *id): check if the id is already in the routing table
 */
int is_new_msg(char *id) {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (memcmp(id, msg_container_list[i].id, ID_LENGTH) == 0) {
			puts("I already had this message, drop it");
			return FALSE;
		}
	}
	return TRUE;
}
/*
 * add_conn(int sock_fd): add a new connection to the connection list
 *   return 0 on success, -1 on failure (list full)
 */
void add_connection(int sock_fd) {
	char ip[MAXLINE], hostname[MAXLINE], local_port[MAXLINE], remote_port[MAXLINE];
	/* get ip, hostname, local port and remote port from sock_fd */
	getsockinfo(sock_fd, ip, hostname, local_port, remote_port);

	/* print out connection info*/
	printf(" new connection established to %s on %s", ip, local_port);

	/* update the connection list */
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 0) {
			connection_list[i].connection_status = 1;
			connection_list[i].sock_fd = sock_fd;
			strncpy(connection_list[i].ip, ip, MAXLINE - 1);
			strncpy(connection_list[i].name, hostname, MAXLINE - 1);
			strncpy(connection_list[i].local_port, local_port, MAXLINE - 1);
			strncpy(connection_list[i].remote_port, remote_port, MAXLINE - 1);
			break;
		}
	}
}
/*
 *  retrieve the conment tcp request's ip, host name, local port, and remote port
 */
void getsockinfo(int sock_fd, char* ip, char* name, char* l_port, char* r_port) {
	struct sockaddr_in sa;
	socklen_t sa_len = sizeof(sa);
	getsockname(sock_fd, (SA*) &sa, &sa_len);
	snprintf(l_port, MAXLINE, "%u", ntohs(sa.sin_port));
	sa_len = sizeof(sa);
	getpeername(sock_fd, (SA *) &sa, &sa_len);
	inet_ntop(AF_INET, &sa.sin_addr, ip, MAXLINE);
	getnameinfo((SA *) &sa, sa_len, name, MAXLINE, r_port, MAXLINE, NI_NUMERICSERV);

}
/*
 * create_tcp_socket() : creates a listening socket at port
 * return: the socket descriptor or -1
 * -1 stands for errors
 */
int create_tcp_socket(char* port) {
	int rv, sockfd = -1;/*new received connection, socket file descriptor*/
	int yes = 1;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and bind to the first we can
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			continue; /* err, try next one */
		} else if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
			throw_exception(FATAL_ERROR, "can't setsockopt to be SO_REUSEADDR");
		} else if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break; /* success */
		else {
			close(sockfd); /* error, try next one */
		}
	} while ((res = res->ai_next) != NULL);

	if (res == NULL) { /* error from the last socket() or bind()*/
		fprintf(stderr, "cann't establish tcp socket to port %s\n", port);
		freeaddrinfo(res);
		return -1;
	}
	listen(sockfd, LISTENQ);
	freeaddrinfo(res);
	return (sockfd);
}
/*
 * bound_udp_socket(): returns a bound udp socket to "port"
 * return: sockfd or -1 on error
 */
int create_udp_socket(char* port) {
	int rv, udp_fd = -1;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	//assign the address of my local host to the socket structures
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		throw_exception(ERROR, "bound_udp error for port %s: %s", port, gai_strerror(rv));
		return -1;
	}

	// loop through all the results and bind to the first we can
	do {
		udp_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (udp_fd < 0) { //error, go to the next
			continue;
		} else if (bind(udp_fd, res->ai_addr, res->ai_addrlen) == 0)
			break; /* success */
		else
			close(udp_fd); /* error */
	} while ((res = res->ai_next) != NULL);

	//check the error from the last socket() or bind()
	if (res == NULL) {
		fprintf(stderr, "cannot bind the udp socket to port %s\n", port);
		freeaddrinfo(res);
		return -1;
	}
	freeaddrinfo(res);
	return (udp_fd);
}
/*
 * tcp_connect(): connect to "host: port" being either the port number or
 *   the actual service's name
 * 	Return: the socket descriptor on success or -1 on error
 */
int tcp_connect(char *ip, char *port) {
	int rv, sockfd = -1;
	struct addrinfo hints, *res;

	/*
	 * fill out this structure to hint getaddrinfo what to search for
	 * if SOCK_STREAM is not specified, for example, two or more
	 * struct addrinfo could be returned since the service may be available
	 * for both UDP and TCP.
	 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET; /* IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* but TCP only */

	if ((rv = getaddrinfo(ip, port, &hints, &res)) != 0) {
		throw_exception(ERROR, "tcp_connect() error : %s", gai_strerror(rv));
		return -1;
	}

	while (res != NULL) {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
			printf("\n\tnew connection established\n");
			break; /* success */
		} else {
			printf("\nerror occurs in tcp connection\n");
			close(sockfd);
			res = res->ai_next;
		}
	}

	if (res == NULL)
	{ /* errno set from final connect() */
		fprintf(stderr, "tcp_connect error for %s, %s", ip, port);
		return -1;
	}
	freeaddrinfo(res);
	return (sockfd);
}
/*
 * set_randm_id(): generate and set random 7 digit ID
 * */
void generate_message_id(char *id) {
	int a[100] = { 0 };
	int i, m;
	char temp[256] = { '\0' };
	char cache[9] = { 0 };
	static int counter = 1;

	srand((unsigned) (time(NULL) + counter));
	counter++;
	for (i = 1; i <= 99; i++) {
		while (a[m = rand() % 100 + 1])
			;
		a[m] = i;
	}
	for (i = 1; i <= 99; ++i) {
		char n[32] = { 0 };
		snprintf(n, 2, "%d", a[i]);
		strcat(temp, n);
	}
	strncpy(cache, temp, 8);
	int j;
	for (j = 0; j < 7; j++)
		id[j] = (char) (cache[j]);
	id[7] = (char) '0';
	//	printf("random id is %s\n", id);
}
/*
 * send_private_message(): send a tcp message to all connected peers.
 * write the message one field at a time.
 */
void send_private_message(char* message, int sock_fd) {
	//message body
	char package[TOKEN_LENTH + 1] = {'\0'};memcpy(package, message, TOKEN_LENTH);

	/* construct header */
	message_header m_header;
	m_header.type = PRIVATE;
	generate_message_id(m_header.id);
	//convert the local variable to network
	m_header.payload_length = htons(strlen(package) + 1);

	//@TODO test
	if ( 1 ) {
		printf("type is %c\n", m_header.type);
		printf("id is %s\n", m_header.id);
		printf("length is %u\n", ntohs(m_header.payload_length));
	}
	if (sizeof(m_header) != 12) {
		printf("message length is %d", sizeof(m_header));
		throw_exception(FATAL_ERROR, "sizeof(m_header) != 12\n");
	}
	//send the message to all connections
	printf("PRIVATE message established, calling send_message()\n");
	send_message(sock_fd, &m_header, package);

}

/*
 * send_message(): send header and string
 */
int send_message(int sock_fd, message_header *m_header, char* msg) {
	char* message;
	int len;
	len = ID_LENGTH + 4 + strlen(msg);
	message = (char*) malloc(len);

	memcpy(message, m_header->id, ID_LENGTH);
	memcpy(message + ID_LENGTH, &(m_header->type), 1);
	memcpy(message + ID_LENGTH + 1, &(m_header->payload_length), 2);
	memcpy(message + ID_LENGTH + 3, msg, strlen(msg) + 1);

	if (writen(sock_fd, message, len) < len)
		throw_exception(ERROR, "Writen message error");

	free(message);
	return 0; /* success */
}

/*
 * send_broadcast_message(): send peer token to all its neighbors
 * */
void send_broadcast_message() {
	// double check for network variables
	my_broc_msg.ip = network_ip;
	my_broc_msg.udp_port = network_udp_port;

	if (0) {
		// check my ip address
		char source_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &network_ip, source_ip, INET_ADDRSTRLEN);
		printf("my ip is %s\n", source_ip);
		// check my udp port
		printf("udp_port is %d now\n", ntohs(network_udp_port));
		// check my peer_token
		printf("my peer token is %s\n", my_broc_msg.peer_token);
	}

	/* add my own broadcast message into broc_bag */
	add_broc_msg(my_broc_msg.peer_token, my_broc_msg.udp_port, my_broc_msg.ip, tcp_port);

	/* construct message body */
	char package[TOKEN_LENTH + 7] =
	{	'\0'};memcpy
	(package, my_broc_msg.peer_token, TOKEN_LENTH);
	memcpy(package + TOKEN_LENTH, &my_broc_msg.ip, 4);
	memcpy(package + TOKEN_LENTH + 4, &my_broc_msg.udp_port, 2);

	if (0) { // test package
		printf("after construction, %d bits long broc_package is %s\n", strlen(package), package);
		char in_token[11] = { '\0' };
		uint16_t in_port = 0;
		uint32_t in_ip = 0;

		memcpy(in_token, package, 10);
		memcpy(&in_ip, package + 10, 4);
		memcpy(&in_port, package + 14, 2);

		char source_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &in_ip, source_ip, INET_ADDRSTRLEN);
		uint16_t h_port;
		h_port = ntohs(in_port);
		puts(in_token);
		puts(source_ip);
		printf("%u\n", h_port);
	} // end test

	/* begin broadcasting */
	int i, sock_fd = -1;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		sock_fd = get_conn_fd(i);
		if (sock_fd != -1) {
			/* construct message header */
			message_header mh;
			init_header(&mh);
			generate_message_id(mh.id);
			mh.type = BROADCAST;
			mh.payload_length = htons(strlen(package) + 1);

			if (sizeof(mh) != 12) {
				printf("mh length is %d\n", sizeof(mh));
				throw_exception(ERROR, "sizeof(mh) != 12\n");
			}

			//printf("BROADCAST message established, calling send_message()\n");
			send_message(sock_fd, &mh, package);

			/* add msg_id to msg_bag */
			if (is_new_msg(mh.id))
				add_msg(mh.id);
		}
	}

}
/*
 * send_salute(): send salute to the leader
 */
void send_salute_message() {
	struct sockaddr_in leader_addr;
	leader_addr.sin_family = AI_CANONNAME;
	leader_addr.sin_port = leader.udp_port;
	leader_addr.sin_addr.s_addr = leader.ip;

	char message[MAXLINE] = { '\0' };
	char salute[SALUTE_LENGTH] = "ALL HALL THE MIGHTY LEADER";

	/* construct message body */
	memcpy(message, my_broc_msg.peer_token, TOKEN_LENTH);
	memcpy(message + TOKEN_LENTH, salute, SALUTE_LENGTH);

	/* construct variables for sendto() */
	size_t msg_len = strlen(message) + 1;

	if (0) {
		printf("sendto():udp_port:%d\nmsg_len:%d\n", ntohs(leader.udp_port), msg_len);
		printf("leader addr:%u\nleader_port:%u\n", leader_addr.sin_addr.s_addr, ntohs(leader_addr.sin_port));
	}

	/* now we can send to udp */
	if (sendto(udp_fd, message, msg_len, 0, (SA *) &leader_addr, sizeof(leader_addr)) < 0) {
		throw_exception(ERROR, "\terror sending salute packet to %s, port:%d", leader.r_ip, ntohs(leader.udp_port));
	} else {
		printf("\tSALLUTE message was sent to host %s, port %d\n", leader.r_ip, ntohs(leader.udp_port));
	}
}
/*
 * process_received_private()
 * */
void process_received_msg(message_header *mh, char msg[], int i) {
	if (mh->type == PRIVATE)
	{
		puts("\n\n\tPRIVATE message received");
		process_private_msg(msg, i);
	} else if (mh->type == BROADCAST)
	{
		puts("\n\n\tBROADCAST message received");
		if (is_new_msg(mh->id)) {
			add_msg(mh->id);
			process_broadcast_msg(mh, msg, i);
		} else {
			throw_exception(NOTE, "duplicate message droped");
			return;
		}
	} else
		throw_exception(NOTE, "wrong type: message ignored");

}
/*
 * process_private_msg(): deal with all private message
 */
void process_private_msg(char* msg, int cid) {
	/* construct private_msg */
	char init_token[TOKEN_LENTH + 1] = {'\0'};
	char ip[MAXLINE] = {'\0'};
	char tcp_port[MAXLINE] = {'\0'};

	strncpy(init_token, msg, TOKEN_LENTH);
// get sender's information
	get_connection_info(cid, ip, tcp_port);
	printf("\tgot init_token: %s from %s:%s\n", init_token, ip, tcp_port);
// add into token bag
	add_init_token(init_token);
// show_received_token();

	if (check_peer_token())
	{ // received enough tokens
		cmp_init_token();
		is_peerToken_determined = 1;
		printf("\tpeer token determined: %s\n", my_broc_msg.peer_token);
		send_broadcast_message();
	}
}
/*
 * process_broadcast_mse(): deal with all broadcast message and forward
 * */
void process_broadcast_msg(message_header *mh, char* msg, int cid) {
	if (0) { // test package
		printf("process_broadcast_msg(): message length %d\n", strlen(msg));
		uint16_t in_port = 0;
		uint32_t in_ip = 0;
		memcpy(&in_ip, msg + 10, 4);
		memcpy(&in_port, msg + 14, 2);
		char source_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &in_ip, source_ip, INET_ADDRSTRLEN);
		uint16_t h_port;
		h_port = ntohs(in_port);
		puts(source_ip);
		printf("%u\n", h_port);
	} // end test

	char in_token[11] = { '\0' };
	memcpy(in_token, msg, TOKEN_LENTH);
	printf("\tgot peer_token: %s\n", in_token);

	char source_token[TOKEN_LENTH + 1] =
	{	'\0'};uint32_t
	source_ip = 0;
	uint16_t source_udp_port = 0;

	memcpy(source_token, msg, TOKEN_LENTH);
	memcpy(&source_ip, msg + TOKEN_LENTH, 4);
	memcpy(&source_udp_port, msg + TOKEN_LENTH + 4, 2);

	char r_port[MAXLINE] = { '\0' };
	char temp_ip[MAXLINE] = { '\0' };

	get_connection_info(cid, temp_ip, r_port);
	add_broc_msg(source_token, source_udp_port, source_ip, r_port);

	if (0) {
		show_broc_bag();
	}

	int i, sock_fd = -1;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (i != cid) {
			sock_fd = get_conn_fd(i);
			if (sock_fd != -1) {
				send_message(sock_fd, mh, msg);
			}
		}
	}
}
/*
 * process_salute(): deal with all the salute udp package
 * */
void process_salute_msg(char buffer[]) {
	char token[TOKEN_LENTH + 1] =
	{	'\0'};char word[27] =
	{	'\0'};
	memcpy(token, buffer, TOKEN_LENTH);
	memcpy(word, buffer + TOKEN_LENTH, SALUTE_LENGTH);
	printf("%s: \"%s\"\n", token, word);
}

/*
 *********************** Givin functions ******************************
 * */
/*
 * ----------------------------------------------------------------------------
 * readn:
 *   read 'n' bytes or upto EOF from descriptor 'fd' into 'vptr'
 *   returns number of bytes read or -1 on error
 *   our program will be blocked waiting for n bytes to be available on fd
 *
 * fd:   the file descriptor (socket) we're reading from
 * vptr: address of memory space to put read data
 * n:    number of bytes to be read
 * ----------------------------------------------------------------------------
 */
ssize_t readn(int fd, void* vptr, size_t n) {
	size_t nleft;
	ssize_t nread;
	char* ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) { /* keep reading upto n bytes     */
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
			{ /* got interrupted by a signal ? */
				nread = 0; /* try calling read() again      */
			} else {
				return (-1);
			}
		} else if (nread == 0) {
			break; /* EOF */
		}
		nleft -= nread;
		ptr += nread;
	}

	return (n - nleft); /* return >= 0 */
}
/*
 * ----------------------------------------------------------------------------
 * writen:
 *   write 'n' bytes from 'vptr' to descriptor 'fd'
 *   returns number of bytes written or -1 on error
 * ----------------------------------------------------------------------------
 */
ssize_t writen(int fd, const void* vptr, size_t n) {
	size_t nleft;
	ssize_t nwritten;
	const char* ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ((nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
			{ /* interrupted by a signal ? */
				nwritten = 0; /* try call write() again    */
			} else {
				return (-1);
			}
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}
/*
 * ----------------------------------------------------------------------------
 * my_read:
 *   internal function, reads upto MAXLINE characters at a time, and then
 *     return them one at a time.
 *   this is much more efficient than 'read'ing one byte at a time, but the 
 *     price is that it's not "reentrant" or "thread-safe", due to the use
 *     of static variables
 * ----------------------------------------------------------------------------
 */
static ssize_t my_read(int fd, char* ptr) {
	static int read_count = 0;
	static char* read_ptr;
	static char read_buf[MAXLINE];
	int got_signal;

	got_signal = 0;
	if (read_count <= 0) {
		again: if ((read_count = read(fd, read_buf, sizeof(read_buf))) < 0) {
			if (errno == EINTR)
			{
				goto again;
				/* Dijkstra hates this, but he's not reading our code */
			} else {
				return (-1);
			}
		} else if (read_count == 0) {
			return (0);
		}
		read_ptr = read_buf;
	}

	read_count--;
	*ptr = *read_ptr++;
	return (1);
}
/*
 * ----------------------------------------------------------------------------
 * readline:
 *   read upto '\n' or maxlen bytes from 'fd' into 'vptr'
 *   returns number of bytes read or -1 on error
 * ----------------------------------------------------------------------------
 */
ssize_t readline(int fd, void *vptr, size_t maxlen) {
	int n, rc;
	char c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++) {
		if ((rc = my_read(fd, &c)) == 1) {
			*ptr++ = c;
			if (c == '\n') {
				break; /* newline is stored, like fgets() */
			}
		} else if (rc == 0) {
			if (n == 1) {
				return (0); /* EOF, no data read       */
			} else {
				break; /* EOF, some data was read */
			}
		} else {
			return (-1); /* error, errno set by read()  */
		}
	}

	*ptr = 0; /* null terminate like fgets() */
	return (n);
}
/*
 * ----------------------------------------------------------------------------
 * simpler_sigaction:
 *   appropriately calls POSIX's sigaction, except for SIGALARM, we try to 
 *     restart any interrupted system calls after any other signals
 *   'signo' is the signal number
 *   'func' is the signal handler
 *   SIG_ERR is returned if the call to sigaction fails
 * ----------------------------------------------------------------------------
 */
Sigfunc*
simpler_sigaction(int signo, Sigfunc *func) {
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (signo == SIGALRM)
	{
#ifdef  SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT; /* SunOS 4.x : only in the case of SIGALARM
		 * we do not want to restart the sys. call */
#endif
	} else {
#ifdef  SA_RESTART
		act.sa_flags |= SA_RESTART; /* SVR4, 44BSD : restart interrupted system
		 * calls */
#endif
	}

	if (sigaction(signo, &act, &oact) < 0) {
		return (SIG_ERR);
	}
	return (oact.sa_handler);
}
/*
 * ----------------------------------------------------------------------------
 * sig_child_handler:
 *   we do not want zombies, so try to wait for all children to finish whenever
 *     a SIGCHLD is received
 * ----------------------------------------------------------------------------
 */
void sig_child_handler(int signo) {
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
		throw_exception(NOTE, "Child process %d terminated\n", pid);
	}
}

