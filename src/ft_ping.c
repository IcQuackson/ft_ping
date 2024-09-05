
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
	inet_pton(AF_INET, arguments->host, &addr.sin_addr); // Convert IPv4 address from text to binary form

	// Create ICMP packet
	struct icmp icmphdr;
	memset(&icmphdr, 0, sizeof(icmphdr));
	icmphdr.icmp_type = ICMP_ECHO;
	icmphdr.icmp_code = 0;
	icmphdr.icmp_id = htons(getpid() & 0xFFFF); // 0xFFFF is a mask to get only the last 16 bits
	icmphdr.icmp_seq = 0;
	icmphdr.icmp_cksum = checksum(&icmphdr, sizeof(icmphdr));
	// Add timestamp to the ICMP packet and pad the rest of the packet with zeros
	memset(icmphdr.icmp_data, 0, sizeof(icmphdr.icmp_data));
	gettimeofday((struct timeval *)icmphdr.icmp_data, NULL);

	char recv_buff[1024];
	struct sockaddr_in r_addr;
	socklen_t addrlen;

	fd_set readfds;
	struct timeval tv;
	int valid_reply_received = 0;

	// Print the ICMP packet information in format: PING <DNS NAME> (<host>) <payload size>(<total size>) bytes of data.
	printf("PING %s (%s) %ld(%ld) bytes of data.\n", arguments->host, arguments->host, sizeof(icmphdr.icmp_data), sizeof(icmphdr));

	while (1)
	{
		log_message(INFO, "Sending ICMP ECHO_REQUEST");
		icmphdr.icmp_seq++;
		icmphdr.icmp_cksum = 0;
		icmphdr.icmp_cksum = checksum(&icmphdr, sizeof(icmphdr));

		if (sendto(sockfd, &icmphdr, sizeof(icmphdr), 0, (struct sockaddr *) &addr, sizeof(addr)) <= 0)
		{
			perror("sendto");
			continue;
		}
		printf("ICMP ECHO_REQUEST sent to %s: icmp_seq%d\n", arguments->host, icmphdr.icmp_seq);

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
				log_message(DEBUG, "Data is available now.");
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
						printf("ICMP ECHO_REPLY received: seq=%d\n", icmp_hdr->icmp_seq);
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
