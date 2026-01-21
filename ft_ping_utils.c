#include "ft_ping.h"

void sig_handler(int sig)
{
    (void)sig;
    g_stop = 1;
}

double timespec_diff_ms(struct timespec *start, struct timespec *end) {
    return (double)(end->tv_sec - start->tv_sec) * 1000.0 +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000.0;
}


unsigned short icmp_checksum(void *buf, int len)
{
    unsigned int sum = 0;
    unsigned short *ptr = buf;

    while (len > 1)
    {
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1)
        sum += *(unsigned char *)ptr;

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

struct timespec timespec_sub(struct timespec a, struct timespec b) { // A - B
    struct timespec res;
    res.tv_sec = a.tv_sec - b.tv_sec;
    res.tv_nsec = a.tv_nsec - b.tv_nsec;
    if (res.tv_nsec < 0) {
        res.tv_sec--;
        res.tv_nsec += 1000000000L;
    }
    return res;
}

struct timespec timespec_add(struct timespec a, struct timespec b) {
    struct timespec res;
    res.tv_sec = a.tv_sec + b.tv_sec;
    res.tv_nsec = a.tv_nsec + b.tv_nsec;
    if (res.tv_nsec >= 1000000000L) {
        res.tv_sec++;
        res.tv_nsec -= 1000000000L;
    }
    return res;
}

int timespec_sign(struct timespec ts) {
    if (ts.tv_sec < 0) return -1;
    if (ts.tv_sec == 0 && ts.tv_nsec <= 0) return -1;
    return 1;
}