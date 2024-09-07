
#include "ft_ping.h"

void ft_ping(t_arguments *arguments)
{
	t_echo_request echo_request = {0};

	get_ip_and_host(arguments, echo_request.ip_host, echo_request.dns_host);

	log_message(DEBUG, "DNS Host: %s", echo_request.dns_host);
	log_message(DEBUG, "IP Host: %s", echo_request.ip_host);

	int sockfd = create_socket();

	struct sockaddr_in addr;
	create_address(&addr, echo_request.ip_host);
	echo_request.addr = &addr;

	while (1)
	{
		send_icmp_request(sockfd, &echo_request);
		wait_and_receive_reply(sockfd);
		sleep(1);
	}

    close(sockfd);
}

/*
* Function to calculate the checksum of the ICMP packet
* @param b: Pointer to the ICMP packet
* @param len: Length of the ICMP packet
* @return Checksum of the ICMP packet
*/
unsigned short checksum(void *b, int len) {    
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++; // Add the 16-bit value to sum
    if (len == 1)
        sum += *(unsigned char *)buf; // If the buffer has an odd length, add the last byte to sum
    sum = (sum >> 16) + (sum & 0xFFFF); // Add the carry to the least significant 16 bits
    sum += (sum >> 16); // Add the carry to the least significant 16 bits
    result = ~sum;
    return result;
}

/*
* Function to wait for an ICMP ECHO_REPLY packet and receive it
* @param sockfd: Socket file descriptor
*/
void wait_and_receive_reply(int sockfd)
{
	fd_set readfds;
	struct timeval tv;
	int valid_reply_received = 0;

	while (!valid_reply_received) {
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;

		int retval = select(sockfd + 1, &readfds, NULL, NULL, &tv);
		log_message(DEBUG, "Select returned %d", retval);
		if (retval == -1)
		{
			perror("select");
			break;
		}
		else if (retval) // Data is available to read
		{
			valid_reply_received = receive_reply(sockfd);
			log_message(DEBUG, "Valid reply received: %d", valid_reply_received);
		}
		else
		{
			printf("Request timed out.\n");
			valid_reply_received = 1;
		}
	}
}

/*
* Function to receive ICMP ECHO_REPLY packets
* @param sockfd: Socket file descriptor
* @param r_addr: Address of the sender of the ICMP ECHO_REPLY packet
* @param recv_buff: Buffer to store the received packet
* @return 1 if a valid ICMP ECHO_REPLY packet is received, 0 otherwise
*/
int receive_reply(int sockfd)
{
	char recv_buff[1024];
	memset(recv_buff, 0, sizeof(recv_buff));

	struct sockaddr_in r_addr;
	socklen_t addrlen = sizeof(r_addr);
	int n = recvfrom(sockfd, recv_buff, sizeof(recv_buff) - 1, 0, (struct sockaddr *) &r_addr, &addrlen);
	if (n > 0)
	{
		recv_buff[n] = 0;
		log_message(DEBUG, "Received %d bytes", n);
		struct iphdr *ip_hdr = (struct iphdr *) recv_buff; // Get the IP header
		struct icmp *icmp_hdr = (struct icmp *) (recv_buff + (ip_hdr->ihl << 2)); // Get the ICMP header by skipping the IP header

		log_message(DEBUG, "ICMP type: %d", icmp_hdr->icmp_type);

		if (icmp_hdr->icmp_type == ICMP_ECHOREPLY)
		{
			log_message(DEBUG, "ICMP ECHO_REPLY received: seq=%d\n", icmp_hdr->icmp_seq);

			print_ping_stats(icmp_hdr, ip_hdr, &r_addr, n);

			return 1;
		}
	}
	else
	{
		perror("recvfrom");
	}
	return 0;
}

/*
* Function to create a raw socket to send ICMP packets
* @return Socket file descriptor
*/
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

/*
* Function to create a sockaddr_in structure with the IP address of the host
* @param addr: Pointer to the sockaddr_in structure
* @param ip_host: IP address of the host
*/
void create_address(struct sockaddr_in *addr, char *ip_host)
{
	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	inet_pton(AF_INET, ip_host, &addr->sin_addr);
}

/*
* Function to create an ICMP ECHO_REQUEST packet
* @param icmphdr: Pointer to the ICMP header
*/
void create_icmp_packet(struct icmp *icmphdr, int seq)
{
    struct timeval time_sent;

	memset(icmphdr, 0, sizeof(struct icmp));
	icmphdr->icmp_type = ICMP_ECHO;
	icmphdr->icmp_code = 0;
	icmphdr->icmp_id = (getpid() & 0xFFFF); // 0xFFFF is a mask to get the last 16 bits
	icmphdr->icmp_seq = seq;
	gettimeofday(&time_sent, NULL);
    memcpy(icmphdr->icmp_data, &time_sent, sizeof(time_sent));
	icmphdr->icmp_cksum = checksum(icmphdr, sizeof(struct icmp));
}

/*
* Function to print the statistics of the ICMP ECHO_REPLY packet
* @param icmphdr: Pointer to the ICMP header
* @param ip_hdr: Pointer to the IP header
* @param r_addr: Pointer to the sockaddr_in structure containing the address of the sender
* @param n_bytes: Number of bytes received
*/
void print_ping_stats(struct icmp *icmphdr, struct iphdr *ip_hdr, struct sockaddr_in *r_addr, int n_bytes)
{
	struct timeval time_received, rtt;
    gettimeofday(&time_received, NULL);

    struct timeval *time_sent_in_reply = (struct timeval *) icmphdr->icmp_data;

    timersub(&time_received, time_sent_in_reply, &rtt);
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", n_bytes, inet_ntoa(r_addr->sin_addr), icmphdr->icmp_seq, ip_hdr->ttl, rtt.tv_sec * 1000.0 + rtt.tv_usec / 1000.0);
}

/*
* Function to send an ICMP ECHO_REQUEST packet
* @param sockfd: Socket file descriptor
* @param addr: Pointer to the sockaddr_in structure containing the address of the host
* @param icmphdr: Pointer to the ICMP header
*/
void send_icmp_request(int sockfd, t_echo_request *echo_request)
{
	log_message(INFO, "Sending ICMP ECHO_REQUEST");

	static int seq = 1;

	struct icmp icmphdr;
	create_icmp_packet(&icmphdr, seq);

    if (sendto(sockfd, &icmphdr, sizeof(icmphdr), 0, (struct sockaddr *) echo_request->addr, sizeof(struct sockaddr_in)) <= 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

	if (icmphdr.icmp_seq == 1)
	{
		printf("PING %s (%s) %ld(%ld) bytes of data.\n", echo_request->dns_host, echo_request->ip_host, sizeof(icmphdr.icmp_data), sizeof(icmphdr));
	}

    log_message(DEBUG, "ICMP ECHO_REQUEST sent to %s: icmp_seq%d", inet_ntoa(echo_request->addr->sin_addr), icmphdr.icmp_seq);
	
	seq++;
}

/*
* Function to get the IP address and hostname of the host
* @param arguments: Pointer to the t_arguments structure
* @param ip_host: Buffer to store the IP address of the host
* @param dns_host: Buffer to store the hostname of the host
*/
void get_ip_and_host(t_arguments *arguments, char ip_host[INET_ADDRSTRLEN], char dns_host[NI_MAXHOST])
{
	if (is_valid_ipv4(arguments->host))
	{
		strncpy(ip_host, arguments->host, INET_ADDRSTRLEN);
		strncpy(dns_host, arguments->host, NI_MAXHOST);
	}
	else
	{
		strncpy(dns_host, arguments->host, NI_MAXHOST);
		convert_hostname_to_ip(dns_host, ip_host);
	}
}

/*
* Function to check if a hostname is a valid IPv4 address
* @param hostname: Hostname to check
* @return 1 if the hostname is a valid IPv4 address, 0 otherwise
*/
int is_valid_ipv4(char *hostname)
{
	struct in_addr ipv4addr;

	return inet_pton(AF_INET, hostname, &ipv4addr) == 1;
}

/*
* Function to convert a hostname to an IP address
* @param hostname: Hostname to convert
* @param ip: Buffer to store the IP address
*/
void convert_hostname_to_ip(const char *hostname, char *ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;

	if ((he = gethostbyname(hostname)) == NULL)
	{
		herror("gethostbyname");
		exit(EXIT_FAILURE);
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for (i = 0; addr_list[i] != NULL; i++)
	{
		strncpy(ip, inet_ntoa(*addr_list[i]), INET_ADDRSTRLEN);
		return;
	}
}
