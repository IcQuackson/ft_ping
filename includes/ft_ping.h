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
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include "arg_parser.h"
#include "logger.h"

#define TIMEOUT 1  // Timeout interval in seconds for receiving an ICMP reply

void ft_ping(t_arguments *arguments);
void get_ip_and_host(t_arguments *arguments, char ip_host[16], char dns_host[1025]);
unsigned short checksum(void *b, int len);
int is_valid_ipv4(char *hostname);
void lookup_dns(const char *hostname);
void convert_hostname_to_ip(const char *hostname, char *ip);

#endif