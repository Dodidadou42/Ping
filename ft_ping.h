#ifndef FT_PING_H
# define FT_PING_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/time.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>


#define ICMP_HDRLEN 8
#define PAYLOAD_SIZE 56
#define PACKET_SIZE (ICMP_HDRLEN + PAYLOAD_SIZE)
#define RECVBUF_SIZE 1024


typedef struct s_ping
{
    int                 sockfd;
    struct addrinfo     *addr;
    struct sockaddr_in  reply_addr;

    char                *hostname;

    unsigned char       sendbuf[PACKET_SIZE];
    unsigned char       recvbuf[RECVBUF_SIZE];

    pid_t               pid;
    int                 seq;

    int                 sent, received;

    double              rtt_min, rtt_max, rtt_sum, rtt_sq_sum;

    struct timespec     start_ts;

    int                 verbose;
}   t_ping;


extern volatile sig_atomic_t g_stop;


void    sig_handler(int sig);

void    ping_init(t_ping *p);
void    parse_options(int argc, char **argv, t_ping *ping);
void    ping_resolve(t_ping *p);

void    usage();
void    usage_error(char *msg);

void    ping_loop(t_ping *p);
void    ping_send(t_ping *p);
void    ping_stats(t_ping *p);

void    ping_receive(t_ping *p);

void    calculate_and_print_rtt(t_ping *p, struct iphdr *ip, struct icmphdr *icmp, ssize_t *n, struct sockaddr_in *from);
void    handle_invalid_reply(t_ping *p, struct sockaddr_in *from, struct icmphdr *icmp, ssize_t len);
void    handle_dest_unreach(const int code);
void    handle_ttl_exceed_and_redirect(const int type, const int code);

unsigned short  icmp_checksum(void *buf, int len);

double          timespec_diff_ms(struct timespec *a, struct timespec *b);
struct timespec timespec_sub(struct timespec a, struct timespec b);
struct timespec timespec_add(struct timespec a, struct timespec b);
int             timespec_sign(struct timespec ts);


#endif