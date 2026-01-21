#include "ft_ping.h"


void parse_options(int argc, char **argv, t_ping *ping)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <host>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++){
        if (argv[i][0] != '-')
            ping->hostname = argv[i];
        else if (argv[i][0] == '-' && argv[i][1] == '-'){
            if (strcmp("--verbose", argv[i]) == 0)
                ping->verbose = 1;
            else if (strcmp("--usage", argv[i]) == 0 || strcmp("--help", argv[i]) == 0)
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

    if (ping->hostname == NULL)
        usage_error("ft_ping: missing host operand");
}

void usage_error(char *msg){
    fprintf(stderr, "%s\n", msg);
    fprintf(stderr, "Try 'ft_ping --help' or 'ft_ping --usage' for more information.\n");
    exit(EXIT_FAILURE);
}

void usage()
{
    fprintf(stderr, "Usage: ft_ping [OPTION...] HOST ...\n");
    fprintf(stderr, "Send ICMP ECHO_REQUEST packets to network hosts.\n\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "  -v, --verbose      verbose output\n");
    fprintf(stderr, "  -?, --help         give this help list\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Report bugs to <ddychus@student.42.fr>.\n");
    exit(EXIT_SUCCESS);
}