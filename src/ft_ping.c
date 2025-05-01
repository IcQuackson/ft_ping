#include "ft_ping.h"

static t_ping_ctx *global_ctx = NULL;


void ft_ping(t_arguments *arguments)
{
	t_ping_ctx ctx = {0};
	global_ctx = &ctx;
	ctx.arguments = arguments;

	signal(SIGINT, sigint_handler);

	gettimeofday(&ctx.start_time, NULL);
	ctx.verbose = arguments->options.verbose;

	strncpy(ctx.echo_request.user_input, arguments->host, NI_MAXHOST - 1);
	ctx.echo_request.user_input[NI_MAXHOST - 1] = '\0';
	resolve(arguments->host, ctx.echo_request.ip_host, ctx.echo_request.dns_host);

	log_message(DEBUG, "DNS Host: %s", ctx.echo_request.dns_host);
	log_message(DEBUG, "IP Host: %s", ctx.echo_request.ip_host);

	ctx.sockfd = create_socket();
	// log ttl
	log_message(INFO, "ttl: %d", arguments->options.ttl);
	set_socket_ttl(&ctx, arguments->options.ttl ? arguments->options.ttl : DEFAULT_TTL);
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
			print_ping_stats(ctx, icmp_hdr, &r_addr, n);
			return 1;
		}
	}
	else
	{
		perror("recvfrom");
	}
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
