#ifndef FT_PING_H
#define FT_PING_H


#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "arg_parser.h"
#include "logger.h"

#define TIMEOUT 1 // Timeout interval in seconds for receiving an ICMP reply
#define PAYLOAD_SIZE 56
#define NI_MAXHOST 1025	   // Maximum length of a hostname
#define INET_ADDRSTRLEN 16 // Maximum length of an IPv4 address
#define MAX_SENT_PACKETS 65536
#define ICMP_HEADER_LEN 28

typedef struct s_ping_stats
{
	int packets_sent;
	int packets_received;
	int packets_lost;
	double min_rtt;
	double max_rtt;
	double avg_rtt;
	double mdev;
} t_ping_stats;

typedef struct s_echo_request
{
	char user_input[NI_MAXHOST];
	char dns_host[NI_MAXHOST];
	char ip_host[INET_ADDRSTRLEN];
	struct sockaddr_in *addr;
} t_echo_request;

typedef struct
{
	int seq;
	int id;
	struct timeval send_time;
} sent_packet_info;

typedef struct s_ping_ctx
{
	t_arguments *arguments;
	t_ping_stats stats;
	sent_packet_info sent_packets[MAX_SENT_PACKETS];
	struct timeval start_time;
	struct timeval end_time;
	t_echo_request echo_request;
	int verbose;
	int sockfd;
	int payload_size;
} t_ping_ctx;

void ft_ping(t_arguments *arguments);
int receive_reply(t_ping_ctx *ctx);
void wait_and_receive_reply(t_ping_ctx *ctx);
void log_verbose(const char *message, ...);
void sigint_handler(int signum);

// Socket/Network
int create_socket();
void set_socket_ttl(t_ping_ctx *ctx, int ttl);
void create_address(struct sockaddr_in *addr, char *ip_host);
int resolve(char *input, char ip_host[INET_ADDRSTRLEN], char dns_host[NI_MAXHOST]);

// Packet
unsigned short checksum(void *b, int len);
struct icmp *create_icmp_packet(t_ping_ctx *ctx, int seq);
void send_icmp_request(t_ping_ctx *ctx);

// Stats
void update_ping_stats(t_ping_ctx *ctx, double rtt_msec);
void print_ping_stats(t_ping_ctx *ctx, struct icmp *icmphdr, struct sockaddr_in *r_addr, int n_bytes);
void print_final_stats(t_ping_ctx *ctx);



#endif // FT_PING_H
