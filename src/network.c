#include "ft_ping.h"

int resolve(char *input, char ip_host[INET_ADDRSTRLEN], char dns_host[NI_MAXHOST])
{
	struct in_addr ipv4;
	struct sockaddr_in sa;
	struct addrinfo hints = {0}, *res;

	// Check if input is an IPv4 address
	if (inet_pton(AF_INET, input, &ipv4) == 1)
	{
		sa.sin_family = AF_INET;
		sa.sin_addr = ipv4;

		strncpy(ip_host, input, INET_ADDRSTRLEN);

		// Perform reverse lookup
		if (getnameinfo((struct sockaddr *)&sa, sizeof sa, dns_host,
						NI_MAXHOST, NULL, 0, NI_NAMEREQD) == 0)
		{
			return 0;
		}
		else
		{
			strcpy(dns_host, "[no PTR record]");
			return 0;
		}
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(input, NULL, &hints, &res) != 0)
	{
		return -1;
	}

	struct sockaddr_in *ipv4_res = (struct sockaddr_in *)res->ai_addr;

	if (inet_ntop(AF_INET, &ipv4_res->sin_addr, ip_host, INET_ADDRSTRLEN) == NULL)
	{
		freeaddrinfo(res);
		return -1;
	}

	if (getnameinfo((struct sockaddr *)ipv4_res, sizeof(*ipv4_res),
					dns_host, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
	{
		strcpy(dns_host, "[no PTR record]");
	}

	freeaddrinfo(res);
	return 0;
}

int create_socket()
{
	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // Create a raw socket to send ICMP packets
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