#include "ft_ping.h"

volatile sig_atomic_t g_stop = 0;

int ping_send(t_ping *p)
{
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

    if (sendto(p->sockfd, p->sendbuf, PACKET_SIZE, 0,
               p->addr->ai_addr, p->addr->ai_addrlen) < 0)
    {
        if (p->verbose)
            perror("ping: sendto");
        return -1;
    }
    p->sent++;
    return 0;
}

void init_recv_msg(t_ping *p, t_recvmsg_data *init)
{
    memset(&init->msg, 0, sizeof(init->msg));
    init->iov.iov_base = p->recvbuf;
    init->iov.iov_len = sizeof(p->recvbuf);
    init->msg.msg_name = &init->from;
    init->msg.msg_namelen = sizeof(init->from);
    init->msg.msg_iov = &init->iov;
    init->msg.msg_iovlen = 1;
    init->msg.msg_control = init->control_buf;
    init->msg.msg_controllen = sizeof(init->control_buf);
}

int handle_icmp_type_error(t_ping *p, t_recvmsg_data *data, struct icmphdr *icmp){
    // Paquet = Ip header + ICMP error hdr + Original IP hdr + original icmp hdr 
    struct iphdr *orig_ip = (struct iphdr *)((char *)icmp + 8);
    struct icmphdr *orig_icmp = (struct icmphdr *)((char *)orig_ip + (orig_ip->ihl * 4));

    if (orig_icmp->un.echo.id != p->pid) // autre destinataire
        return -1;

    printf("From %s: ", inet_ntoa(data->from.sin_addr));

    if (icmp->type == ICMP_TIME_EXCEEDED)
        printf("Time to live exceeded\n");
    else if (icmp->type == ICMP_DEST_UNREACH){
        switch (icmp->code){
            case ICMP_NET_UNREACH:  printf("Destination Net Unreachable\n"); break;
            case ICMP_HOST_UNREACH: printf("Destination Host Unreachable\n"); break;
            case ICMP_PROT_UNREACH: printf("Destination Protocol Unreachable\n"); break;
            case ICMP_PORT_UNREACH: printf("Destination Port Unreachable\n"); break;
            default: printf("Dest Unreachable, Bad Code: %d\n", icmp->code); break;
        }
    } else if (p->verbose){
        printf("ICMP type=%d, code= %d\n", icmp->type, icmp->code);
    }


    return -1;
}

int ping_receive(t_ping *p)
{

    t_recvmsg_data recv_data;
    init_recv_msg(p, &recv_data);

    ssize_t n = recvmsg(p->sockfd, &recv_data.msg, 0);
    if (n < 0)
        return -1;

    struct iphdr *ip = (struct iphdr *)p->recvbuf;
    struct icmphdr *icmp = (struct icmphdr *)(p->recvbuf + (ip->ihl * 4));

    // error icmp type ou pid
    if (icmp->type != ICMP_ECHOREPLY || icmp->un.echo.id != p->pid)
        return handle_icmp_type_error(p, &recv_data, icmp);
    // paquet ok
    return calculate_and_print_rtt(p, ip, icmp, &n, &recv_data.from);
}

int calculate_and_print_rtt(t_ping *p, struct iphdr *ip, struct icmphdr *icmp, ssize_t *n, struct sockaddr_in *from)
{
    struct timespec now, *sent_ts;
    clock_gettime(CLOCK_MONOTONIC, &now);
    sent_ts = (struct timespec *)(p->recvbuf + (ip->ihl * 4) + ICMP_HDRLEN);

    double rtt = timespec_diff_ms(sent_ts, &now);

    p->received++;
    p->rtt_sum += rtt;
    if (rtt < p->rtt_min)
        p->rtt_min = rtt;
    if (rtt > p->rtt_max)
        p->rtt_max = rtt;
    p->rtt_mdev_sum += fabs(rtt - (p->rtt_sum / p->received));

    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
           *n - (ip->ihl * 4), inet_ntoa(from->sin_addr),
           icmp->un.echo.sequence, ip->ttl, rtt);

    return 0;
}

void ping_loop(t_ping *p)
{
    struct timespec last, intvl, now, resp_time;
    fd_set fds;

    intvl.tv_sec = 1;
    intvl.tv_nsec = 0;

    ping_send(p);
    clock_gettime(CLOCK_MONOTONIC, &last);

    while (!g_stop)
    {
        FD_ZERO(&fds);
        FD_SET(p->sockfd, &fds);

        clock_gettime(CLOCK_MONOTONIC, &now);
        resp_time = timespec_sub(timespec_add(last, intvl), now);

        if (timespec_sign(resp_time) == -1) 
            resp_time.tv_nsec = resp_time.tv_sec = 0;

        int n = pselect(p->sockfd + 1, &fds, NULL, NULL, &resp_time, NULL);

        if (n < 0)
        {
            if (errno != EINTR)
                perror("pselect");
            continue;
        }
        else if (n == 1)
        {
            if (ping_receive(p) == 0) // réponse valide
            { // option -c 

            }
        }
        else
        { // Timeout pselect, on send
            ping_send(p);
            clock_gettime(CLOCK_MONOTONIC, &last);
        }
    }
}

void ping_stats(t_ping *p)

{
    printf("\n--- %s ping statistics ---\n", p->hostname);
    printf("%d packets transmitted, %d received, %.1f%% packet loss\n",
           p->sent, p->received,
           p->sent ? (100.0 * (p->sent - p->received) / p->sent) : 0.0);
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

