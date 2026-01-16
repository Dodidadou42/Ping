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

    double              rtt_min, rtt_max, rtt_sum, rtt_mdev_sum;

    struct timespec     start_ts, next_send;

    int                 verbose;
}   t_ping;

extern volatile sig_atomic_t g_stop;
void    sig_handler(int sig);

void    ping_init(t_ping *p);
void    ping_resolve(t_ping *p);
void    ping_socket(t_ping *p);
void    ping_loop(t_ping *p);
void    ping_stats(t_ping *p);


void            ping_send(t_ping *p);
int             ping_receive(t_ping *p);
unsigned short  icmp_checksum(void *buf, int len);


double  ts_diff_ms(struct timespec *a, struct timespec *b);

#endif