#include "ft_ping.h"

volatile sig_atomic_t g_stop = 0;


int main(int argc, char **argv){
    if (argc < 2)
        usage_error("'ft_ping' requires at least one argument");
    t_ping p;
    ping_init(&p);
    parse_options(argc, argv, &p);
    ping_resolve(&p);
    ping_loop(&p);
    ping_stats(&p);
    return EXIT_SUCCESS;
}

void ping_loop(t_ping *p){
    struct timespec last, now, resp_time, intvl = {1, 0};
    fd_set fds;
    ping_send(p);
    clock_gettime(CLOCK_MONOTONIC, &last);
    while (!g_stop){
        FD_ZERO(&fds);
        FD_SET(p->sockfd, &fds);
        clock_gettime(CLOCK_MONOTONIC, &now);
        resp_time = timespec_sub(timespec_add(last, intvl), now);
        if (timespec_sign(resp_time) == -1) 
            resp_time.tv_nsec = resp_time.tv_sec = 0;
        int n = pselect(p->sockfd + 1, &fds, NULL, NULL, &resp_time, NULL);
        if (n < 0){
            if (errno != EINTR)
                perror("pselect");
            continue;
        }
        else if (n == 1)
            ping_receive(p);
        else { // Timeout pselect, on send
            ping_send(p);
            clock_gettime(CLOCK_MONOTONIC, &last);
        }
    }
}

void ping_send(t_ping *p){
    memset(p->sendbuf, 0, PACKET_SIZE);
    struct icmphdr *icmp = (struct icmphdr *)p->sendbuf;

    icmp->type = ICMP_ECHO;
    icmp->un.echo.id = p->pid;
    icmp->un.echo.sequence = p->seq++;
    // timestamp
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    memcpy(p->sendbuf + ICMP_HDRLEN, &ts, sizeof(ts));
    // padding
    for (int i = sizeof(ts); i < PAYLOAD_SIZE; i++)
        p->sendbuf[ICMP_HDRLEN + i] = i;
    // checksum
    icmp->checksum = icmp_checksum(p->sendbuf, PACKET_SIZE);

    if (sendto(p->sockfd, p->sendbuf, PACKET_SIZE, 0, p->addr->ai_addr, p->addr->ai_addrlen) < 0){
        if (p->verbose)
            perror("ping: sendto");
        return;
    }
    p->sent++;
    return;
}

void ping_stats(t_ping *p){
    printf("\n--- %s ping statistics ---\n", p->hostname);
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
           p->sent, p->received,
           p->sent ? (100.0 * (p->sent - p->received) / p->sent) : 0.0);
    if (p->received > 0)
    {
        double avg = p->rtt_sum / p->received;
        double mdev = sqrt((p->rtt_sq_sum / p->received) - (avg * avg));
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               p->rtt_min, avg, p->rtt_max, mdev);
    }
    freeaddrinfo(p->addr);
    close(p->sockfd);
}



