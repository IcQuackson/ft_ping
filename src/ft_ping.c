#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include "ft_ping.h"

static t_ping_ctx *global_ctx = NULL;

void ft_ping(t_arguments *arguments)
{
	t_ping_ctx ctx = {0};
	global_ctx = &ctx;

	signal(SIGINT, sigint_handler);

	gettimeofday(&ctx.start_time, NULL);
	ctx.verbose = arguments->options.verbose;

	strncpy(ctx.echo_request.user_input, arguments->host, NI_MAXHOST - 1);
	ctx.echo_request.user_input[NI_MAXHOST - 1] = '\0';
	resolve(arguments->host, ctx.echo_request.ip_host, ctx.echo_request.dns_host);

	log_message(DEBUG, "DNS Host: %s", ctx.echo_request.dns_host);
	log_message(DEBUG, "IP Host: %s", ctx.echo_request.ip_host);

	ctx.sockfd = create_socket();
	struct sockaddr_in addr;
	create_address(&addr, ctx.echo_request.ip_host);
	ctx.echo_request.addr = &addr;

	while (1)
	{
		send_icmp_request(&ctx);
		wait_and_receive_reply(&ctx);
		sleep(1);
	}

	close(ctx.sockfd);
}


/*
 * Function to calculate the checksum of the ICMP packet
 * @param b: Pointer to the ICMP packet
 * @param len: Length of the ICMP packet
 * @return Checksum of the ICMP packet
 */
unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum = 0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2)
		sum += *buf++; // Add the 16-bit value to sum
	if (len == 1)
		sum += *(unsigned char *)buf;	// If the buffer has an odd length, add the last byte to sum
	sum = (sum >> 16) + (sum & 0xFFFF); // Add the carry to the least significant 16 bits
	sum += (sum >> 16);					// Add the carry to the least significant 16 bits
	result = ~sum;
	return result;
}


void wait_and_receive_reply(t_ping_ctx *ctx)
{
	fd_set readfds;
	struct timeval tv;
	int valid_reply_received = 0;

	while (!valid_reply_received)
	{
		FD_ZERO(&readfds);
		FD_SET(ctx->sockfd, &readfds);
		tv.tv_sec = TIMEOUT;
		tv.tv_usec = 0;

		int retval = select(ctx->sockfd + 1, &readfds, NULL, NULL, &tv);
		log_message(DEBUG, "Select returned %d", retval);
		if (retval == -1)
		{
			perror("select");
			break;
		}
		else if (retval)
		{
			valid_reply_received = receive_reply(ctx);
			log_message(DEBUG, "Valid reply received: %d", valid_reply_received);
		}
		else
		{
			log_verbose("Request timed out\n");
			valid_reply_received = 1;
		}
	}
}


int receive_reply(t_ping_ctx *ctx)
{
	char recv_buff[1024];
	memset(recv_buff, 0, sizeof(recv_buff));

	struct sockaddr_in r_addr;
	socklen_t addrlen = sizeof(r_addr);
	int n = recvfrom(ctx->sockfd, recv_buff, sizeof(recv_buff) - 1, 0, (struct sockaddr *)&r_addr, &addrlen);
	if (n > 0)
	{
		recv_buff[n] = 0;
		log_message(DEBUG, "Received %d bytes", n);
		struct iphdr *ip_hdr = (struct iphdr *)recv_buff;						 // Get the IP header
		struct icmp *icmp_hdr = (struct icmp *)(recv_buff + (ip_hdr->ihl << 2)); // Get the ICMP header by skipping the IP header

		log_message(DEBUG, "ICMP type: %d", icmp_hdr->icmp_type);

		if (icmp_hdr->icmp_type == ICMP_ECHOREPLY && (getpid() & 0xFFFF))
		{
			log_message(DEBUG, "ICMP ECHO_REPLY received: seq=%d\n", icmp_hdr->icmp_seq);
			ctx->stats.packets_received++;
			print_ping_stats(ctx, icmp_hdr, ip_hdr, &r_addr, n);
			return 1;
		}
	}
	else
	{
		perror("recvfrom");
	}
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


void create_icmp_packet(t_ping_ctx *ctx, struct icmp *icmphdr, int seq)
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

	ctx->sent_packets[seq % MAX_SENT_PACKETS].seq = seq;
	ctx->sent_packets[seq % MAX_SENT_PACKETS].send_time = time_sent;
	ctx->sent_packets[seq % MAX_SENT_PACKETS].id = icmphdr->icmp_id;
}


void print_ping_stats(t_ping_ctx *ctx, struct icmp *icmphdr, struct iphdr *ip_hdr, struct sockaddr_in *r_addr, int n_bytes)
{
	struct timeval time_received, rtt;
	gettimeofday(&time_received, NULL);

	int seq_index = icmphdr->icmp_seq % MAX_SENT_PACKETS;
	struct timeval *time_sent_in_reply = &ctx->sent_packets[seq_index].send_time;

	timersub(&time_received, time_sent_in_reply, &rtt); // RTT = received - sent

	double rtt_msec = (rtt.tv_sec * 1000.0) + (rtt.tv_usec / 1000.0);

	update_ping_stats(ctx, rtt_msec);

	printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
		   n_bytes,
		   inet_ntoa(r_addr->sin_addr),
		   icmphdr->icmp_seq,
		   ip_hdr->ttl,
		   rtt_msec);
}


void update_ping_stats(t_ping_ctx *ctx, double rtt_msec)
{
	ctx->stats.min_rtt = fmin(ctx->stats.min_rtt, rtt_msec);
	ctx->stats.max_rtt = fmax(ctx->stats.max_rtt, rtt_msec);

	double delta = rtt_msec - ctx->stats.avg_rtt;
	ctx->stats.avg_rtt += delta / ctx->stats.packets_received;
	double delta2 = rtt_msec - ctx->stats.avg_rtt;
	ctx->stats.mdev += delta * delta2;
}


void print_final_stats(t_ping_ctx *ctx)
{
	gettimeofday(&ctx->end_time, NULL); // Capture end time

	long msec = (ctx->end_time.tv_sec - ctx->start_time.tv_sec) * 1000 +
				(ctx->end_time.tv_usec - ctx->start_time.tv_usec) / 1000;

	printf("\n--- %s ping statistics ---\n", ctx->echo_request.user_input);

	printf("%d packets transmitted, %d received, %d%% packet loss, time %ldms\n",
		   ctx->stats.packets_sent,
		   ctx->stats.packets_received,
		   100 * (ctx->stats.packets_sent - ctx->stats.packets_received) / ctx->stats.packets_sent,
		   msec);

	if (ctx->stats.packets_received > 1)
	{
		printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
			   ctx->stats.min_rtt,
			   ctx->stats.avg_rtt,
			   ctx->stats.max_rtt,
			   sqrt(ctx->stats.mdev / (ctx->stats.packets_received - 1)));
	}
}


void send_icmp_request(t_ping_ctx *ctx)
{
	static int seq = 1;
	struct icmp icmphdr;
	create_icmp_packet(ctx, &icmphdr, seq);

	if (sendto(ctx->sockfd, &icmphdr, sizeof(icmphdr), 0,
			   (struct sockaddr *)ctx->echo_request.addr,
			   sizeof(struct sockaddr_in)) <= 0)
	{
		perror("sendto");
		exit(EXIT_FAILURE);
	}

	ctx->stats.packets_sent++;

	if (icmphdr.icmp_seq == 1)
	{
		printf("PING %s(%s (%s)) %ld(%ld) bytes of data.\n",
			   ctx->echo_request.user_input,
			   ctx->echo_request.dns_host,
			   ctx->echo_request.ip_host,
			   sizeof(icmphdr.icmp_data),
			   sizeof(icmphdr));
	}

	log_message(DEBUG, "ICMP ECHO_REQUEST sent to %s: icmp_seq=%d",
				inet_ntoa(ctx->echo_request.addr->sin_addr),
				icmphdr.icmp_seq);

	seq++;
}


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


void log_verbose(const char *message, ...)
{
	if (!global_ctx || !global_ctx->verbose)
		return;

	va_list args;
	va_start(args, message);
	vprintf(message, args);
	va_end(args);
}


void sigint_handler(int signum)
{
	(void)signum;
	if (global_ctx)
		print_final_stats(global_ctx);
	exit(0);
}
