#include "ft_ping.h"

struct icmp *create_icmp_packet(t_ping_ctx *ctx, int seq)
{
	struct icmp *icmp_pkt = NULL;
	struct timeval time_sent;
	int packet_len = 8 + ctx->payload_size;

	icmp_pkt = malloc(packet_len);
	if (!icmp_pkt) {
		exit(EXIT_FAILURE);
	}
	memset(icmp_pkt, 0, packet_len);
	icmp_pkt->icmp_type = ICMP_ECHO;
	icmp_pkt->icmp_code = 0;
	icmp_pkt->icmp_id = (getpid() & 0xFFFF); // 0xFFFF is a mask to get the last 16 bits
	icmp_pkt->icmp_seq = seq;
	gettimeofday(&time_sent, NULL);

	log_message(DEBUG, "time_sent: %d\n", time_sent.tv_sec);
	
	memcpy(icmp_pkt->icmp_data, &time_sent, sizeof(time_sent));
	icmp_pkt->icmp_cksum = checksum(icmp_pkt, packet_len);

	ctx->sent_packets[seq % MAX_SENT_PACKETS].seq = seq;
	ctx->sent_packets[seq % MAX_SENT_PACKETS].send_time = time_sent;
	ctx->sent_packets[seq % MAX_SENT_PACKETS].id = icmp_pkt->icmp_id;
	
	return icmp_pkt;
}

void send_icmp_request(t_ping_ctx *ctx)
{
	static int seq = 1;
	struct icmp *icmp_pkt = create_icmp_packet(ctx, seq);

	if (sendto(ctx->sockfd, icmp_pkt, 8 + ctx->payload_size, 0,
			   (struct sockaddr *)ctx->echo_request.addr,
			   sizeof(struct sockaddr_in)) <= 0)
	{
		perror("ping: connect");
		exit(EXIT_FAILURE);
	}

	ctx->stats.packets_sent++;

	if (icmp_pkt->icmp_seq == 1)
	{
		printf(!ctx->echo_request.dns_host[0] ? "PING %s%s (%s) %d(%d) bytes of data.\n" : "PING %s(%s (%s)) %d(%d) bytes of data.\n",
			   ctx->echo_request.user_input,
			   ctx->echo_request.dns_host,
			   ctx->echo_request.ip_host,
			   ctx->payload_size,
			   IP_HEADER_LEN + ICMP_HEADER_LEN + ctx->payload_size);
	}

	log_message(DEBUG, "ICMP ECHO_REQUEST sent to %s: icmp_seq=%d",
				inet_ntoa(ctx->echo_request.addr->sin_addr),
				icmp_pkt->icmp_seq);

	seq++;
	free(icmp_pkt);
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
