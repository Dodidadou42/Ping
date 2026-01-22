#include "ft_ping.h"


void ping_init(t_ping *p)
{
    memset(p, 0, sizeof(*p));
    p->pid = getpid() & 0xffff;
    p->seq = 1;
    p->rtt_min = 1e9;
    clock_gettime(CLOCK_MONOTONIC, &p->start_ts);

    p->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (p->sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
}

void parse_options(int argc, char **argv, t_ping *ping)
{
    for (int i = 1; i < argc; i++){
        if (argv[i][0] != '-')
            ping->hostname = argv[i];
        else if (argv[i][0] == '-' && argv[i][1] == '-'){
            if (strcmp("--verbose", argv[i]) == 0)
                ping->verbose = 1;
            else if (strcmp("--help", argv[i]) == 0)
                usage();
            else {
                fprintf(stderr, "%s: unrecognized option '%s'", argv[0], argv[i]);
                usage_error("");
            }
        } else {
            for (int j = 1; argv[i][j] != '\0'; j++){
                if (argv[i][j] == 'v')
                    ping->verbose = 1;
                else if (argv[i][j] == '?')
                    usage();
                else {
                    fprintf(stderr, "%s: invalid option -- '%c'", argv[0], argv[i][j]);
                    usage_error("");
                }
            }
        }
    }
}


void ping_resolve(t_ping *p)
{
    if (p->hostname == NULL)
        usage_error("ft_ping: missing host operand");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;

    if (getaddrinfo(p->hostname, NULL, &hints, &p->addr) != 0){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &((struct sockaddr_in *)p->addr->ai_addr)->sin_addr, ip, sizeof(ip));

    printf("PING %s (%s): %d data bytes", p->hostname, ip, PAYLOAD_SIZE);
    if (p->verbose)
        printf(", id 0x%X = %d", p->pid, p->pid);
    printf("\n");
    
}

