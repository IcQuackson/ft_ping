#include "ft_ping.h"

void print_ping_stats(t_ping_ctx *ctx, struct icmp *icmphdr, struct sockaddr_in *r_addr, int n_bytes)
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
		   ctx->arguments->options.ttl,
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
