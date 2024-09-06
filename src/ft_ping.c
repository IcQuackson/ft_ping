
#include "ft_ping.h"

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
    result = ~sum; // Get the one's complement of the sum
    return result;
}

void ft_ping(t_arguments *arguments)
{
	char dns_host[NI_MAXHOST];
	char ip_host[INET_ADDRSTRLEN];

	get_ip_and_host(arguments, ip_host, dns_host);

	int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); // Create a raw socket to send ICMP packets
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Create address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET; // IPv4
	inet_pton(AF_INET, ip_host, &addr.sin_addr); // Convert IPv4 address from text to binary form

	// Create ICMP packet
	struct icmp icmphdr;
	memset(&icmphdr, 0, sizeof(icmphdr));
	icmphdr.icmp_type = ICMP_ECHO;
	icmphdr.icmp_code = 0;
	icmphdr.icmp_id = htons(getpid() & 0xFFFF); // 0xFFFF is a mask to get only the last 16 bits
	icmphdr.icmp_seq = 0;
	icmphdr.icmp_cksum = checksum(&icmphdr, sizeof(icmphdr));

	char recv_buff[1024];
	struct sockaddr_in r_addr;
	socklen_t addrlen;

	fd_set readfds;
	struct timeval tv;
	int valid_reply_received = 0;

	// Print the ICMP packet information in format: PING <DNS NAME> (<host>) <payload size>(<total size>) bytes of data.
	printf("PING %s (%s) %ld(%ld) bytes of data.\n", dns_host, ip_host, sizeof(icmphdr.icmp_data), sizeof(icmphdr));

	while (1)
	{
		send_icmp_request(sockfd, &addr, &icmphdr);

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
			else if (retval)
			{
                addrlen = sizeof(r_addr);
				int n = recvfrom(sockfd, recv_buff, sizeof(recv_buff) - 1, 0, (struct sockaddr *)&r_addr, &addrlen);
				if (n > 0)
				{
					recv_buff[n] = 0;
					log_message(DEBUG, "Received %d bytes", n);
					struct iphdr *ip_hdr = (struct iphdr *) recv_buff; // Get the IP header
					struct icmp *icmp_hdr = (struct icmp *) (recv_buff + (ip_hdr->ihl << 2)); // Get the ICMP header by skipping the IP header

					log_message(DEBUG, "ICMP type: %d", icmp_hdr->icmp_type);
					
					// Check if the received packet is an ICMP ECHO_REPLY packet and if it is the reply to the sent packet
					if (icmp_hdr->icmp_type == ICMP_ECHOREPLY &&
                        icmp_hdr->icmp_id == htons(getpid() & 0xFFFF) &&
                        icmp_hdr->icmp_seq == icmphdr.icmp_seq)
					{
						log_message(DEBUG, "ICMP ECHO_REPLY received: seq=%d\n", icmp_hdr->icmp_seq);

                        print_ping_stats(icmp_hdr, ip_hdr, &r_addr, n);
						
						valid_reply_received = 1;
					}
				}
				else
				{
					perror("recvfrom");
				}

			}
			else
			{
				printf("Request timed out.\n");
				valid_reply_received = 1;
			}
		}
		valid_reply_received = 0;
		sleep(1);
	}


    close(sockfd);
}

void print_ping_stats(struct icmp *icmphdr, struct iphdr *ip_hdr, struct sockaddr_in *r_addr, int n_bytes)

{
	struct timeval time_received, rtt;
    gettimeofday(&time_received, NULL);

    struct timeval *time_sent_in_reply = (struct timeval *) icmphdr->icmp_data;

    timersub(&time_received, time_sent_in_reply, &rtt);
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", n_bytes, inet_ntoa(r_addr->sin_addr), icmphdr->icmp_seq, ip_hdr->ttl, rtt.tv_sec * 1000.0 + rtt.tv_usec / 1000.0);
}


void send_icmp_request(int sockfd, struct sockaddr_in *addr, struct icmp *icmphdr)
{
    struct timeval time_sent;
	log_message(INFO, "Sending ICMP ECHO_REQUEST");
    icmphdr->icmp_seq++;
    icmphdr->icmp_cksum = 0;
    gettimeofday(&time_sent, NULL);
    memcpy(icmphdr->icmp_data, &time_sent, sizeof(time_sent));
    icmphdr->icmp_cksum = checksum(icmphdr, sizeof(struct icmp));

    if (sendto(sockfd, icmphdr, sizeof(*icmphdr), 0, (struct sockaddr *) addr, sizeof(*addr)) <= 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    log_message(DEBUG, "ICMP ECHO_REQUEST sent to %s: icmp_seq%d", inet_ntoa(addr->sin_addr), icmphdr->icmp_seq);
}

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

int is_valid_ipv4(char *hostname)
{
	struct in_addr ipv4addr;

	return inet_pton(AF_INET, hostname, &ipv4addr) == 1;
}

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

void get_timestamp(char *timestamp)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
}