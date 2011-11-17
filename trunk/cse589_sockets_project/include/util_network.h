/*
 * author : Yin Yan
 */

#ifndef _UTIL_NETWORK_H
#define _UTIL_NETWORK_H

#include "util_header.h"
#include "exceptions.h"

#define LISTENQ	1024     /* for listen() how many request can be hold in Queue*/
#define SA	struct sockaddr

/* message type */
#define PRIVATE 0x00
#define BROADCAST 0x01
#define SALUTE 0x10

/* message length */
#define ID_LENGTH 8
#define TOKEN_LENTH 10
#define SALUTE_LENGTH 26

/*
 * **************** Local Variables **********************
 *
 */
char current_host_name[MAXLINE];
char local_ip[INET_ADDRSTRLEN];

/*
 * **************** Network Variables **********************
 *
 */
#define MAX_CONNECTIONS 3
#define MAX_CITIZEN_NUM 20	// max number of citizens

int listen_fd;	//socket file descriptor for listening connection
int tcp_fd;
int udp_fd;

char udp_port[MAXLINE];
char tcp_port[MAXLINE];

int32_t network_ip;
int16_t network_udp_port;
int16_t network_tcp_port;

/****************** Connection container ********************/
typedef struct {
	int sock_fd; //the socket file descriptor
	int connection_status; //connection status
	char ip[MAXLINE]; //IP address
	char name[MAXLINE]; //host name
	char local_port[MAXLINE]; //local port
	char remote_port[MAXLINE]; //remote port
} connection_container;

/****************** Msg Header Table ************************/
typedef struct {
	char id[ID_LENGTH + 1];
	char type;
	uint16_t payload_length;
} message_header;

typedef struct {
	char peer_token[TOKEN_LENTH + 1];
	uint32_t ip;
	uint16_t udp_port;
	char r_ip[MAXLINE];
	char r_port[MAXLINE];
	int isUsed;
} broadcast_bag;

/********************** token bag *************************/
typedef struct {
	char token_list[TOKEN_LENTH + 1];
	int isUsed;
} token_container;
/*
 * ********************* message bag *********************
 * */
typedef struct {
	uint8_t id[ID_LENGTH + 1];
	int isUsed;
} message_container;

/*
 ************************* utility functions ****************************
 */
// fd table related
int get_max_fd();
int get_conn_fd(int conn_id);

// socket related
void getsockinfo(int sock_fd, char* ip, char* name, char* l_port, char* r_port);
int create_tcp_socket(char* port);
int create_udp_socket(char* port);
int tcp_connect(char *ip, char *port);

// conn_list related
void get_connection_info(int conn_id, char ip[], char port[]);
void init_conn_list();
void add_connection(int sock_fd);
int count_current_connections();
int remove_conn(int conn_id);
void display_tcp_connection();

// message container
void init_message_container();
void add_msg(char *id);

//token container related
void init_token_container();
void add_init_token(char init_token[]);
int count_init_token();
void cmp_init_token();
void show_received_token();

// broadcast table related
void init_broc_bag();
void init_leader();
void find_leader();
void add_broc_msg(char peer_token[], uint16_t udp_port, uint32_t ip,
		char r_port[]);
int numof_peer_token();
void disp_all_token();
int get_self_token(char token[]);

// message related
void generate_message_id(char *id);
int is_new_msg(char *id);
void init_header(message_header *mh);
int send_message(int sock_fd, message_header *mh, char* msg);
void send_private_message(char* message, int sock_fd);
void send_broadcast_message();
void send_salute_message();
void process_received_message(message_header *mh, char msg[], int cid);
void process_private_msg(char* msg, int i);
void process_broadcast_msg(message_header *mh, char* msg, int cid);
void process_salute_msg(char buffer[]);

// misc
int get_connection_arg(char * arg_line, char *arg_array[]);


/*
 *************************** Givin Functions *****************************
 * */
ssize_t readn(int, void*, size_t);
ssize_t writen(int, const void*, size_t);
ssize_t readline(int, void*, size_t);
Sigfunc* simpler_sigaction(int, Sigfunc*);
void sig_child_handler(int);

#endif /* _UTIL_NETWORK_H */
