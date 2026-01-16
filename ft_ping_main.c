#include "ft_ping.h"



volatile sig_atomic_t g_stop = 0;

void ping_send(t_ping *p)
{
    memset(p->sendbuf, 0, PACKET_SIZE);

    struct icmphdr *icmp = (struct icmphdr *)p->sendbuf;
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = p->pid;
    icmp->un.echo.sequence = p->seq++;

    //monotonic timestamp in payload
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    memcpy(p->sendbuf + ICMP_HDRLEN, &ts, sizeof(ts));

    // padding 
    for (int i = sizeof(ts); i < PAYLOAD_SIZE; i++)
        p->sendbuf[ICMP_HDRLEN + i] = i;

    icmp->checksum = icmp_checksum(p->sendbuf, PACKET_SIZE);

    if (sendto(p->sockfd, p->sendbuf, PACKET_SIZE, 0,
               p->addr->ai_addr, p->addr->ai_addrlen) < 0)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }
    p->sent++;
}

int ping_receive(t_ping *p)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(p->sockfd, &fds);

    struct timeval tv = {1, 0}; /* 1s timeout */
    int ret = select(p->sockfd + 1, &fds, NULL, NULL, &tv);

    if (ret <= 0)
        return 0;

    socklen_t len = sizeof(p->reply_addr);
    ssize_t r = recvfrom(p->sockfd, p->recvbuf, sizeof(p->recvbuf), 0,
                          (struct sockaddr *)&p->reply_addr, &len);
    if (r <= 0)
        return 0;

    struct iphdr *ip = (struct iphdr *)p->recvbuf;
    struct icmphdr *icmp = (struct icmphdr *)(p->recvbuf + ip->ihl * 4);

    if (icmp->type != ICMP_ECHOREPLY || icmp->un.echo.id != p->pid)
        return 0;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    struct timespec *sent_ts =
        (struct timespec *)(p->recvbuf + ip->ihl * 4 + ICMP_HDRLEN);

    double rtt = ts_diff_ms(sent_ts, &now);

    p->received++;
    p->rtt_sum += rtt;
    if (rtt < p->rtt_min) p->rtt_min = rtt;
    if (rtt > p->rtt_max) p->rtt_max = rtt;

    double avg = p->rtt_sum / p->received;
    p->rtt_mdev_sum += fabs(rtt - avg);

    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
           r - ip->ihl * 4,
           inet_ntoa(p->reply_addr.sin_addr),
           icmp->un.echo.sequence,
           ip->ttl,
           rtt);

    return 1;
}


void ping_loop(t_ping *p)
{
    struct timespec now;
    double sleep_ms;

    while (!g_stop)
    {
        clock_gettime(CLOCK_MONOTONIC, &now);

        if (ts_diff_ms(&p->next_send, &now) >= 0){
            ping_send(p);
            p->next_send.tv_sec += 1;
        }
        ping_receive(p);

        clock_gettime(CLOCK_MONOTONIC, &now);
        sleep_ms = ts_diff_ms(&now, &p->next_send);
        if (sleep_ms > 0){
            struct timespec ts = {
                .tv_sec = (time_t)(sleep_ms / 1000),
                .tv_nsec = (long)((sleep_ms - (ts.tv_sec * 1000)) * 1e6)
            };
            nanosleep(&ts, NULL);
        }
        sleep(1);
    }
}

void ping_stats(t_ping *p)
{
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);

    double total = ts_diff_ms(&p->start_ts, &end);

    printf("\n--- %s ping statistics ---\n", p->hostname);
    printf("%d packets transmitted, %d received, %.1f%% packet loss, time %.0f ms\n",
           p->sent, p->received,
           p->sent ? (100.0 * (p->sent - p->received) / p->sent) : 0.0,
           total);

    if (p->received > 0)
    {
        double avg = p->rtt_sum / p->received;
        double mdev = p->rtt_mdev_sum / p->received;
        printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
               p->rtt_min, avg, p->rtt_max, mdev);
    }

    freeaddrinfo(p->addr);
    close(p->sockfd);
}


void confirm_option(char *option, t_ping *ping){
    if (strcmp(option, "-v") == 0)
        ping->verbose = 1;
    else {
        fprintf(stderr, "Unknown option : %s\n", option);
        exit(EXIT_FAILURE);
    }
}

void parse_options(int argc, char **argv, t_ping *ping){
    if (argc < 2){
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
   
    for (int i = 1; i < argc - 1; i++)
        confirm_option(argv[i], ping);

    ping->hostname = argv[argc - 1];
    

}

int main(int argc, char **argv)
{
    t_ping p;
    ping_init(&p);
    parse_options(argc, argv, &p);
    ping_resolve(&p);
    ping_socket(&p);
    ping_loop(&p);
    ping_stats(&p);
    return EXIT_SUCCESS;
}
