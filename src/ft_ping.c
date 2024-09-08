
#include "ft_ping.h"

t_ping_stats ping_stats = {
    .packets_sent = 0,
    .packets_received = 0,
    .packets_lost = 0,
    .min_rtt = INFINITY,  // Use INFINITY to ensure any actual RTT will be lower
    .max_rtt = -INFINITY, // Use -INFINITY to ensure any actual RTT will be higher
    .avg_rtt = 0,
    .mdev = 0
};

struct timeval start_time, end_time;
sent_packet_info sent_packets[MAX_SENT_PACKETS];
t_echo_request echo_request = {0};
int verbose = 0;

/*
* Function to start the ping process
* @param arguments: Pointer to the t_arguments structure
*/
void ft_ping(t_arguments *arguments)
{
	gettimeofday(&start_time, NULL);  // Capture the start time

	verbose = arguments->options.verbose;

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
			log_verbose("Request timed out\n");
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

		if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && (getpid() & 0xFFFF))
		{
			log_message(DEBUG, "ICMP ECHO_REPLY received: seq=%d\n", icmp_hdr->icmp_seq);
			ping_stats.packets_received++;
			print_ping_stats(icmp_hdr, ip_hdr, &r_addr, n);
			return 1;
		}
		else
		{
			log_verbose("Other ICMP packet received\n");
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

	// Store packet send info
    sent_packets[seq % MAX_SENT_PACKETS].seq = seq;
    sent_packets[seq % MAX_SENT_PACKETS].send_time = time_sent;
	sent_packets[seq % MAX_SENT_PACKETS].id = icmphdr->icmp_id;
}

/*
* Function to print the statistics of the ICMP ECHO_REPLY packet
* @param icmphdr: Pointer to the ICMP header
* @param ip_hdr: Pointer to the IP header
* @param r_addr: Pointer to the sockaddr_in structure containing the address of the sender
* @param n_bytes: Number of bytes received
*/
void print_ping_stats(struct icmp *icmphdr, struct iphdr *ip_hdr, struct sockaddr_in *r_addr, int n_bytes) {
    struct timeval time_received, rtt;
    gettimeofday(&time_received, NULL);

    int seq_index = icmphdr->icmp_seq % MAX_SENT_PACKETS;
    struct timeval *time_sent_in_reply = &sent_packets[seq_index].send_time;

    timersub(&time_received, time_sent_in_reply, &rtt); // Calculate RTT by subtracting the time the packet was sent from the time the reply was received. the measured time is in microseconds

	double rtt_msec = (rtt.tv_sec * 1000.0) + (rtt.tv_usec / 1000.0);

	update_ping_stats(rtt_msec);

    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", n_bytes, inet_ntoa(r_addr->sin_addr), icmphdr->icmp_seq, ip_hdr->ttl, rtt_msec);
}

/* 
* Function to update the ping statistics
* @param rtt_msec: Round-trip time in milliseconds
*/
void update_ping_stats(double rtt_msec)
{
	ping_stats.min_rtt = fmin(ping_stats.min_rtt, rtt_msec);
    ping_stats.max_rtt = fmax(ping_stats.max_rtt, rtt_msec);

	// Update average and variance (for mdev calculation)
    double delta = rtt_msec - ping_stats.avg_rtt;
    ping_stats.avg_rtt += delta / ping_stats.packets_received;
    double delta2 = rtt_msec - ping_stats.avg_rtt;
    ping_stats.mdev += delta * delta2;
}

/*
* Function to print the final statistics of the ping process
*/
void print_final_stats() {
    gettimeofday(&end_time, NULL);  // Capture the end time

    long msec = (end_time.tv_sec - start_time.tv_sec) * 1000 + (end_time.tv_usec - start_time.tv_usec) / 1000;

    printf("\n--- %s ping statistics ---\n", echo_request.dns_host != NULL ? echo_request.dns_host : echo_request.ip_host);
    printf("%d packets transmitted, %d received, %d%% packet loss, time %ldms\n", 
        ping_stats.packets_sent, ping_stats.packets_received,
        100 * (ping_stats.packets_sent - ping_stats.packets_received) / ping_stats.packets_sent,
        msec);
	
	if (ping_stats.packets_received != 0)
	{
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n", 
			ping_stats.min_rtt, ping_stats.avg_rtt, ping_stats.max_rtt, sqrt(ping_stats.mdev / (ping_stats.packets_received - 1)));
	}
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

	ping_stats.packets_sent++;

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

/*
* Function to log a message with a specific log level
* @param level: Log level
* @param message: Message to log
*/
void log_verbose(const char *message, ...)
{
	if (verbose)
	{
		va_list args;
		va_start(args, message);
		vprintf(message, args);
		va_end(args);
	}
}

/*
* Signal handler for SIGINT (Ctrl+C) to print the final statistics and exit
*/
void sigint_handler(int signum)
{
	(void) signum;
	print_final_stats();
	exit(0);
}