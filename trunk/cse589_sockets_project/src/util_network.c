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
// my own message for udp message for broadcasting
static udp_message_struct self_peer_msg;
// leader infomation
static udp_message_struct leader;
static connection_container connection_list[MAX_CONNECTIONS];
static token_container init_token_container_list[MAX_CONNECTIONS];
static message_container msg_container_list[MAX_CITIZEN_NUM];
//recieved udp message list
static udp_message_struct recieved_peer_token_container[MAX_CITIZEN_NUM];

void init_recieve_udp_message_container() {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		recieved_peer_token_container[i].ip = '\0';
		recieved_peer_token_container[i].udp_port = '\0';
		recieved_peer_token_container[i].peer_token[0] = '\0';
		recieved_peer_token_container[i].remote_ip[0] = '\0';
		recieved_peer_token_container[i].remote_port[0] = '\0';
		recieved_peer_token_container[i].isUsed = 0;
	}
}

void init_leader() {
	leader.ip = '\0';
	leader.udp_port = '\0';
	leader.peer_token[0] = '\0';
	leader.remote_ip[0] = '\0';
	leader.remote_port[0] = '\0';
	leader.isUsed = 0;
}

void init_header(message_header *mh) {
	mh->id[0] = '\0';
	mh->type = '\0';
	mh->payload_length = 0;
}

int count_init_token() {
	int i, counter = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (init_token_container_list[i].isUsed == 1)
			counter++;
	}

	return counter;
}
int get_self_token(char token[]) {
	if (strlen(self_peer_msg.peer_token) != 0) {
		strncpy(token, self_peer_msg.peer_token, TOKEN_LENTH);
		return 1;
	} else {
		return 0;
	}
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

void add_msg_to_container(char *id) {
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

void add_peer_token_to_container(char peer_token[], uint16_t udp_port, uint32_t ip, char r_port[]) {
	int i;
	int flag = 0;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (recieved_peer_token_container[i].isUsed == 0 && flag == 0) {
			// add ip in big-endian
			recieved_peer_token_container[i].ip = ip;
			// add ip in little-endian
			char s_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(recieved_peer_token_container[i].ip), s_ip, INET_ADDRSTRLEN);
			strncpy(recieved_peer_token_container[i].remote_ip, s_ip, strlen(s_ip));
			// add udp port in big-endian
			recieved_peer_token_container[i].udp_port = udp_port;
			// add udp port in little-endian
			strncpy(recieved_peer_token_container[i].peer_token, peer_token, TOKEN_LENTH);
			strncpy(recieved_peer_token_container[i].remote_port, r_port, strlen(r_port));
			// add signal
			recieved_peer_token_container[i].isUsed = 1;
			flag = 1;
		}
	}
	// check if broadcast bag have n citizen's peer token
	int curr_peer_token = count_peer_token();
	int curr_init_token = count_init_token();
	if (curr_peer_token == max_citizen_number && curr_init_token == max_citizen_number) {
		find_leader();
		printf("\t leader(%s) is from %s:%d\n", leader.peer_token, leader.remote_ip, ntohs(leader.udp_port));
		send_salute_message();
	}
}

void show_upd_message_container() {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (recieved_peer_token_container[i].isUsed == 1) {
			char source_ip[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(recieved_peer_token_container[i].ip), source_ip, INET_ADDRSTRLEN);
			printf("\t Received token: i have token: <%s>  ip: <%s> udp_port: <%d>\n", recieved_peer_token_container[i].peer_token, source_ip, ntohs(recieved_peer_token_container[i].udp_port));
		}
	}
}

void show_received_token() {
	int i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (init_token_container_list[i].isUsed == 1)
			printf("I have %s in token_bag\n", init_token_container_list[i].token_list);
	}
}

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

void get_peer_token() {
	int z;
	long long int temp = 0;
	long long int max_t = 0;
	for (z = 0; z < MAX_CONNECTIONS; z++) {
		if (init_token_container_list[z].isUsed == 1) {
			temp = strtoll(init_token_container_list[z].token_list, NULL, 0);
			max_t = max(max_t, temp);
		}
	}
	sprintf(self_peer_msg.peer_token, "%lld", max_t);
}

void find_leader() {
	int z;
	long long int temp1 = 0;
	long long int temp2 = 0;

	for (z = 0; z < MAX_CITIZEN_NUM; z++) {
		if (recieved_peer_token_container[z].isUsed == 1) {
			temp1 = strtoll(recieved_peer_token_container[z].peer_token, NULL, 0);
			temp2 = strtoll(leader.peer_token, NULL, 0);
			if (temp1 >= temp2)
				leader = recieved_peer_token_container[z];
		}
	}
}

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

void display_all_token() {
	if (count_peer_token() == 0) {
		printf("\n\t None of the peer token received\n");
		fflush(stdout);
		return;
	}
	int a = strlen("IP"); /* IP field length          */
	int b = strlen("remote port"); /* remote port field length   */
	int c = strlen("token"); /* token field length  */

	// find out how long each field needs to be
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (recieved_peer_token_container[i].isUsed == 1) {
			a = max(a, strlen(recieved_peer_token_container[i].remote_ip));
			b = max(b, strlen(recieved_peer_token_container[i].remote_port));
			c = max(c, strlen(recieved_peer_token_container[i].peer_token));
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
		if (recieved_peer_token_container[i].isUsed == 1) {
			printf(" %-*s | %-*s | %-*s\n", a, recieved_peer_token_container[i].remote_ip, b, recieved_peer_token_container[i].remote_port, c, recieved_peer_token_container[i].peer_token);
		}
	}
	fflush(stdout);
}

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

int get_max_fd() {
	int i, maxfd = -1;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1) {
			maxfd = max(maxfd, connection_list[i].sock_fd);
		}
	}
	return maxfd;
}

int count_peer_token() {
	int i, counter = 0;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (recieved_peer_token_container[i].isUsed == 1)
			counter++;
	}
	return counter;
}

int count_current_connections() {
	int i, counter = 0;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connection_list[i].connection_status == 1)
			counter++;
	}
	return counter;
}

void get_connection_info(int conn_id, char ip[], char port[]) {
	strncpy(ip, connection_list[conn_id].ip, MAXLINE);
	strncpy(port, connection_list[conn_id].remote_port, MAXLINE);
}

int get_conn_fd(int conn_id) {
	int conn = -1;
	if (connection_list[conn_id].connection_status == 1) {
		conn = connection_list[conn_id].sock_fd;
	}
	return conn;
}

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

int is_new_msg(char *id) {
	int i;
	for (i = 0; i < MAX_CITIZEN_NUM; i++) {
		if (memcmp(id, msg_container_list[i].id, ID_LENGTH) == 0) {
			puts("\t this message has been received!!");
			return FALSE;
		}
	}
	return TRUE;
}

void add_connection(int sock_fd) {
	char ip[MAXLINE], hostname[MAXLINE], local_port[MAXLINE], remote_port[MAXLINE];
	//get ip, hostname, local port and remote port from sock_fd
	throw_exception(DEBUG, "remote port");
	throw_exception(DEBUG, remote_port);
	getsockinfo(sock_fd, ip, hostname, local_port, remote_port);

	//print out connection info
	printf("\t new connection established to %s on %s", ip, local_port);

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

int tcp_connect(char *ip, char *port) {
	int rv, sockfd = -1;
	struct addrinfo hints, *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

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
			break;
		} else {
			printf("\nerror occurs in tcp connection\n");
			close(sockfd);
			res = res->ai_next;
		}
	}

	if (res == NULL) { //errno set from final connect()
		fprintf(stderr, "tcp_connect error for %s, %s", ip, port);
		return -1;
	}
	freeaddrinfo(res);
	return (sockfd);
}

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

void send_private_message(char* message, int sock_fd) {
	//message body
	char package[TOKEN_LENTH + 1] = {'\0'};memcpy(package, message, TOKEN_LENTH);

	//header
	message_header m_header;
	m_header.type = PRIVATE;
	generate_message_id(m_header.id);
	//convert the local variable to network
	m_header.payload_length = htons(strlen(package) + 1);
	if (sizeof(m_header) != 12) {
		printf("message length is %d", sizeof(m_header));
		throw_exception(FATAL_ERROR, "sizeof(m_header) != 12\n");
	}
	//send the message to all connections
	printf("\t PRIVATE message is ready to go, calling send_message()\n");
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

void send_udp_message() {
	// double check for network variables
	self_peer_msg.ip = network_ip;
	self_peer_msg.udp_port = network_udp_port;
	//TODO test
	if (1) {
		// check my ip address
		char source_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &network_ip, source_ip, INET_ADDRSTRLEN);
		printf(" my ip is %s \n", source_ip);
		// check my udp port
		printf(" udp_port is %d now \n", ntohs(network_udp_port));
		// check my peer_token
		printf(" my peer token is %s \n", self_peer_msg.peer_token);
	}

	add_peer_token_to_container(self_peer_msg.peer_token, self_peer_msg.udp_port, self_peer_msg.ip, tcp_port);

	/* construct message body */
	char package[TOKEN_LENTH + 7] = {'\0'};memcpy
	(package, self_peer_msg.peer_token, TOKEN_LENTH);
	memcpy(package + TOKEN_LENTH, &self_peer_msg.ip, 4);
	memcpy(package + TOKEN_LENTH + 4, &self_peer_msg.udp_port, 2);

	//TODO test
	if (1) { // test package
		printf("after construction, %d bits long udp message is %s \n", strlen(package), package);
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
				add_msg_to_container(mh.id);
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
	memcpy(message, self_peer_msg.peer_token, TOKEN_LENTH);
	memcpy(message + TOKEN_LENTH, salute, SALUTE_LENGTH);

	/* construct variables for sendto() */
	size_t msg_len = strlen(message) + 1;
	//TODO

	//TODO test
	if (1) {
		printf("\t sendto():udp_port:%d\nmsg_len:%d\n", ntohs(leader.udp_port), msg_len);
		printf("\t leader addr:%u\nleader_port:%u\n", leader_addr.sin_addr.s_addr, ntohs(leader_addr.sin_port));
	}

	/* now we can send to udp */
	if (sendto(udp_fd, message, msg_len, 0, (SA *) &leader_addr, sizeof(leader_addr)) < 0) {
		throw_exception(ERROR, "\terror sending salute packet to %s, port:%d", leader.remote_ip, ntohs(leader.udp_port));
	} else {
		printf("\tSALLUTE message was sent to host %s, port %d\n", leader.remote_ip, ntohs(leader.udp_port));
	}
}

void process_received_message(message_header *mh, char msg[], int i) {
	if (mh->type == PRIVATE) {
		puts("\n\n\t receive message on TCP port ... \n");
		process_private_msg(msg, i);
	} else if (mh->type == BROADCAST) {
		puts("\n\n\t Receive message on UDP port ... \n");
		//TODO
		puts("id =====>");
		puts(mh->id);
		puts("\n");
		if (is_new_msg(mh->id)) {
			add_msg_to_container(mh->id);
			process_broadcast_msg(mh, msg, i);
		} else {
			throw_exception(NOTE, "\t drop duplicate message");
			return;
		}
	} else {
		throw_exception(NOTE, "\t wrong type: message ignored");
	}
}
/*
 * process_private_msg(): deal with all private message
 */
void process_private_msg(char* msg, int cid) {
	/* construct private_msg */
	char init_token[TOKEN_LENTH + 1] = {'\0'};char ip[MAXLINE] = {'\0'};
	char tcp_port[MAXLINE] = {'\0'};

	strncpy(init_token, msg, TOKEN_LENTH);
	//sender infomation
	get_connection_info(cid, ip, tcp_port);
	printf("\t got init_token: %s from %s:%s\n", init_token, ip, tcp_port);
	add_init_token(init_token);
	show_received_token();

	int num_tokens = count_init_token();
	int num_peers = count_current_connections();

	if ( num_tokens == num_peers ) { // received enough tokens
		puts("\t OK, we have enough tokens, here!!!!!!!!!");
		get_peer_token();
		is_peerToken_determined = 1;
		printf("\t peer token determined: %s\n", self_peer_msg.peer_token);
		send_udp_message();
	}
}
/*
 * process_broadcast_mse(): deal with all broadcast message and forward
 * */
void process_broadcast_msg(message_header *mh, char* msg, int cid) {
	//TODO test
	if (1) { // test package
		printf("\t process_broadcast_msg(): message length %d\n", strlen(msg));
		uint16_t in_port = 0;
		uint32_t in_ip = 0;
		memcpy(&in_ip, msg + 10, 4);
		memcpy(&in_port, msg + 14, 2);
		char source_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &in_ip, source_ip, INET_ADDRSTRLEN);
		uint16_t h_port;
		h_port = ntohs(in_port);
		puts(source_ip);
		printf("\t %u\n", h_port);
	} // end test

	char in_token[11] = { '\0' };
	memcpy(in_token, msg, TOKEN_LENTH);
	printf("\t Received peer_token: %s\n", in_token);

	char source_token[TOKEN_LENTH + 1] = {'\0'};uint32_t
	source_ip = 0;
	uint16_t source_udp_port = 0;

	memcpy(source_token, msg, TOKEN_LENTH);
	memcpy(&source_ip, msg + TOKEN_LENTH, 4);
	memcpy(&source_udp_port, msg + TOKEN_LENTH + 4, 2);

	char remote_port[MAXLINE] = { '\0' };
	char temp_ip[MAXLINE] = { '\0' };

	get_connection_info(cid, temp_ip, remote_port);
	add_peer_token_to_container(source_token, source_udp_port, source_ip, remote_port);
	//TODO test
	if (1) {
		show_upd_message_container();
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
	char token[TOKEN_LENTH + 1] = {'\0'};char word[27] = {'\0'};
	memcpy(token, buffer, TOKEN_LENTH);
	memcpy(word, buffer + TOKEN_LENTH, SALUTE_LENGTH);
	printf("%s: \"%s\"\n", token, word);
}

void display_peer_token() {
	char my_token[TOKEN_LENTH] = { '\0' };
	if (get_self_token(my_token))
		printf("\n\t peer token determined: %s\n", my_token);
	else
		printf("\n\t WAITING ON PEER_TOKEN\n");
}
/*
 *********************** Given functions ******************************
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
	while (nleft > 0) {
		if ((nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR) {
				nread = 0;
			} else {
				return (-1);
			}
		} else if (nread == 0) {
			break; /* EOF */
		}
		nleft -= nread;
		ptr += nread;
	}

	return (n - nleft);
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
			if (errno == EINTR) {
				nwritten = 0;
			} else {
				return (-1);
			}
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (n);
}

