#include "ft_ping.h"

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

void send_icmp_request(t_ping_ctx *ctx)
{
	static int seq = 1;
	struct icmp icmphdr;
	create_icmp_packet(ctx, &icmphdr, seq);

	if (sendto(ctx->sockfd, &icmphdr, sizeof(icmphdr), 0,
			   (struct sockaddr *)ctx->echo_request.addr,
			   sizeof(struct sockaddr_in)) <= 0)
	{
		perror("ping: connect");
		exit(EXIT_FAILURE);
	}

	ctx->stats.packets_sent++;

	if (icmphdr.icmp_seq == 1)
	{
		printf(!ctx->echo_request.dns_host[0] ? "PING %s%s (%s) %ld(%ld) bytes of data.\n" : "PING %s(%s (%s)) %ld(%ld) bytes of data.\n",
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
