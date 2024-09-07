#ifndef FT_PING_H
#define FT_PING_H

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include <time.h>
#include "arg_parser.h"
#include "logger.h"

#define TIMEOUT 1  // Timeout interval in seconds for receiving an ICMP reply
#define PAYLOAD_SIZE 56

typedef struct s_ping_stats {
	int packets_sent;
	int packets_received;
	int packets_lost;
	int min_rtt;
	int max_rtt;
	int avg_rtt;
} t_ping_stats;

typedef struct s_echo_request {
	struct icmp *icmphdr;
	char data[PAYLOAD_SIZE];
} t_echo_request;

void ft_ping(t_arguments *arguments);
void get_ip_and_host(t_arguments *arguments, char ip_host[16], char dns_host[1025]);
unsigned short checksum(void *b, int len);
int is_valid_ipv4(char *hostname);
void convert_hostname_to_ip(const char *hostname, char *ip);
void send_icmp_request(int sockfd, struct sockaddr_in *addr, struct icmp *icmphdr);
void print_ping_stats(struct icmp *icmphdr, struct iphdr *ip_hdr, struct sockaddr_in *r_addr, int n_bytes);
void create_icmp_packet(struct icmp *icmphdr);
void create_address(struct sockaddr_in *addr, char *ip_host);
int create_socket();
int receive_reply(int sockfd);
void wait_and_receive_reply(int sockfd);


#endif