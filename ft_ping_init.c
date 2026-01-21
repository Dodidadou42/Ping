#include "ft_ping.h"


void ping_init(t_ping *p)
{
    memset(p, 0, sizeof(*p));
    p->pid = getpid() & 0xffff;
    p->seq = 1;
    p->rtt_min = 1e9;
    clock_gettime(CLOCK_MONOTONIC, &p->start_ts);
    

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
}

void ping_resolve(t_ping *p)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;


    if (getaddrinfo(p->hostname, NULL, &hints, &p->addr) != 0)
    {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET,
        &((struct sockaddr_in *)p->addr->ai_addr)->sin_addr,
        ip, sizeof(ip));

    printf("PING %s (%s): %d data bytes",
           p->hostname, ip, PAYLOAD_SIZE);
    if (p->verbose)
        printf(", id 0x%X = %d", p->pid, p->pid);
    printf("\n");
    
}

void ping_socket(t_ping *p)
{
    p->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (p->sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }
}