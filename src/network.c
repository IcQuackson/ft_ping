#include "ft_ping.h"

int resolve(char *input, char ip_host[INET_ADDRSTRLEN], char dns_host[NI_MAXHOST])
{
	struct in_addr ipv4;
	struct addrinfo hints = {0}, *res;

	// Check if input is an IPv4 address
	if (inet_pton(AF_INET, input, &ipv4) == 1)
	{
		strncpy(ip_host, ipv4.s_addr == INADDR_ANY ? "127.0.0.1" : input, INET_ADDRSTRLEN);
		ip_host[INET_ADDRSTRLEN - 1] = '\0';
		return 0;
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Check if input is a hostname
	if (getaddrinfo(input, NULL, &hints, &res) != 0)
	{
		fprintf(stderr, "ping: %s: Name or service not known\n", input);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in *ipv4_res = (struct sockaddr_in *)res->ai_addr;

	// Convert the IP address to a string
	if (inet_ntop(AF_INET, &ipv4_res->sin_addr, ip_host, INET_ADDRSTRLEN) == NULL)
	{
		freeaddrinfo(res);
		return -1;
	}

	getnameinfo((struct sockaddr *)ipv4_res, sizeof(*ipv4_res),
				dns_host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
	freeaddrinfo(res);
	return 0;
}

int create_socket()
{
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sockfd;
}

void create_address(struct sockaddr_in *addr, char *ip_host)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	inet_pton(AF_INET, ip_host, &addr->sin_addr);
}

void set_socket_ttl(t_ping_ctx *ctx, int ttl)
{
	if (setsockopt(ctx->sockfd, IPPROTO_IP, IP_TTL,
				   &ttl, sizeof(ttl)) < 0)
	{
		perror("setsockopt(IP_TTL)");
		exit(EXIT_FAILURE);
	}
}
