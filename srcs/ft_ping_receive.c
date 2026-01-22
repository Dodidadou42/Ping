#include "../includes/ft_ping.h"

void ping_receive(t_ping *p){
    struct msghdr msg;
    struct iovec iov;
    struct sockaddr_in from;
    char control_buf[128];

    memset(&msg, 0, sizeof(msg));
    iov.iov_base = p->recvbuf;
    iov.iov_len = sizeof(p->recvbuf);
    msg.msg_name = &from;
    msg.msg_namelen = sizeof(from);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_buf;
    msg.msg_controllen = sizeof(control_buf);

    ssize_t n = recvmsg(p->sockfd, &msg, 0);
    if (n < 0)
        return;
    struct iphdr *ip = (struct iphdr *)p->recvbuf;
    struct icmphdr *icmp = (struct icmphdr *)(p->recvbuf + (ip->ihl * 4));
    if (icmp->type != ICMP_ECHOREPLY || icmp->un.echo.id != p->pid)
        handle_invalid_reply(p, &from, icmp, n);
    else 
        calculate_and_print_rtt(p, ip, icmp, &n, &from);
}

void calculate_and_print_rtt(t_ping *p, struct iphdr *ip, struct icmphdr *icmp, ssize_t *n, struct sockaddr_in *from){
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
    p->rtt_sq_sum += rtt * rtt;

    printf("%ld bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
           *n - (ip->ihl * 4), inet_ntoa(from->sin_addr),
           icmp->un.echo.sequence, ip->ttl, rtt);

    return;
}

void handle_invalid_reply(t_ping *p, struct sockaddr_in *from, struct icmphdr *icmp, ssize_t len){
    // check packet integrity
    if (len < (ssize_t)(sizeof(struct icmphdr) + sizeof(struct iphdr)))
        return;
    
    // Packet = Ip header + ICMP error hdr + Original IP hdr + original icmp hdr 
    struct iphdr *orig_ip = (struct iphdr *)((char *)icmp + 8);
    struct icmphdr *orig_icmp = (struct icmphdr *)((char *)orig_ip + (orig_ip->ihl * 4));

    if (orig_icmp->un.echo.id != p->pid) // other recipient
        return;
    printf("From %s: ", inet_ntoa(from->sin_addr));
    if (icmp->type == ICMP_TIME_EXCEEDED || icmp->type == ICMP_REDIRECT)
        handle_ttl_exceed_and_redirect(icmp->type, icmp->code);
    else if (icmp->type == ICMP_DEST_UNREACH)
        handle_dest_unreach(icmp->code);
    else if (p->verbose)
        printf("ICMP type=%d, code= %d\n", icmp->type, icmp->code);
}

void handle_dest_unreach(const int code){
    if (code == ICMP_NET_UNREACH)
        printf("Destination Net Unreachable\n");
    else if (code == ICMP_HOST_UNREACH)
        printf("Destination Host Unreachable\n");
    else if (code == ICMP_PROT_UNREACH)
        printf("Destination Protocol Unreachable\n");
    else if (code == ICMP_PORT_UNREACH)
        printf("Destination Port Unreachable\n");
    else if (code == ICMP_FRAG_NEEDED)
        printf("Fragmentation needed and DF set\n");
    else if (code == ICMP_SR_FAILED)
        printf("Source Route Failed\n");
    else if (code == ICMP_NET_UNKNOWN)
        printf("Network Unknown\n");
    else if (code == ICMP_HOST_UNKNOWN)
        printf("Host Unknown\n");
    else if (code == ICMP_HOST_ISOLATED)
        printf("Host Isolated\n");
    else if (code == ICMP_NET_UNR_TOS)
        printf("Destination Network Unreachable At This TOS\n");
    else if (code == ICMP_HOST_UNR_TOS)
        printf("Destination Host Unreachable At This TOS\n");
    else
        printf("Dest Unreachable, Bad Code: %d\n", code);
}

void handle_ttl_exceed_and_redirect(const int type, const int code){
    if (type == ICMP_TIME_EXCEEDED){
        if (code == ICMP_EXC_TTL)
            printf("Time to live exceeded\n");
        else if (code == ICMP_EXC_FRAGTIME)
            printf("Frag reassembly time exceeded\n");
    }
    else if (type == ICMP_REDIRECT){
        if (code == ICMP_REDIR_NET)
            printf("Redirect Network\n");
        else if (code == ICMP_REDIR_HOST)
            printf("Redirect Host\n");
        else if (code == ICMP_REDIR_NETTOS)
            printf("Redirect Type of Service and Network\n");
        else if (code == ICMP_REDIR_HOSTTOS)
            printf("Redirect Type of Service and Host\n");
    }
}





