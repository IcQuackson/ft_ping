# ft_ping

Ping is a tool used for network testing.

The ping tool primarily uses two types of ICMP datagrams: ICMP ECHO_REQUEST and ICMP ECHO_REPLY. These are used to check the reachability of a network device and measure the time it takes for data to travel to and from the target device. 

ICMP (Internet Control Message Protocol) is used primarily for sending message errors and operational information.

Virtually all modern routers are equipped to understand and process ICMP ECHO_REQUEST datagrams.

The ICMP ECHO_REQUEST is identified by the following details:
- Type: 8
- Code: 0
- Description: 
    - Message send by originating host to the target host.
- Fields:
    - ID
    - Sequence Number: Increments with each request and it's used to check the order of replies and identify lost packets.
    - Data Payload: Bytes of padding to make packet useful for measurement.

The ICMP ECHO_REPLY is identified by the following details:
- Type: 0
- Code: 0
- Description:
    - This is the message sent by the target back to the originating host in response to an ECHO_REQUEST.
    - This indicates the target is reachable and provides a measurement for network latency.
- Fields:
    - ID and Sequence Number: These match those of the corresponding ECHO_REQUEST to help sender identify which is which.
    - Data Payload: Same sent by ECHO_REQUEST

Usage:
    ping [option...] host

The subject of this project requires the implementation of the -v and -? options.

using -? produces output similar to this:
"
Usage: ping [OPTION...] HOST ...
Send ICMP ECHO_REQUEST packets to network hosts.

Options:
  -?, --help             Give this help list
      --usage            Give a short usage message
  -V, --version          Print program version

Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options.

Report bugs to <bug-inetutils@gnu.org>.
"
-v stands for verbose mode. When used, this flag instructs ping to provide more details about each ICMP ECHO_REQUEST and ECHO_REPLY.
In ping's verbose mode, additional details about the ICMP packets, such as their sequence number, TTL (Time to Live), and other parameters might be displayed.

For more information about usage of ping from inetutils-2.0 check out https://www.gnu.org/software/inetutils/manual/inetutils.pdf page 10.