#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>           /* PRId64 */
#include <endian.h>             /* htobe64 */
#include <limits.h>             /* PATH_MAX */
#include <hiredis/hiredis.h>

#define JSON_MAX 10000

typedef struct __attribute__((__packed__)) ass_schd_s {
    uint64_t start_time;
    uint64_t duration;
    uint32_t flags;
    char     json[JSON_MAX+1];
} ass_schd_t;

int main(int argc, char **argv) {
    redisContext *c;
    redisReply *reply;
    int opt;
    const char *key = NULL;
    int port = 6379;
    const char *hostname = "127.0.0.1";
    int verbose = 0;
    ass_schd_t as;
    unsigned int i;


    while ((opt = getopt(argc, argv, "k:h:p:V")) != -1) {
        switch (opt) {
        case 'k':
            key = optarg;
            break;
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'V':
            verbose = 1;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: schedule KEY [PORT] [HOSTNAME]\n");
            exit(EXIT_FAILURE);
        }
    }

    if (key == NULL) {
        fprintf(stderr, "Usage: schedule KEY [PORT] [HOSTNAME]\n");
        exit(EXIT_FAILURE);
    }

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(1);
    }

    reply = redisCommand(c, "ZRANGE %s 0 -1", key);
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (i = 0; i < reply->elements; i++) {
            memset(&as, 0, sizeof(as));
            memcpy(&as, reply->element[i]->str, reply->element[i]->len);

            as.start_time  = be64toh(as.start_time);
            as.duration    = be64toh(as.duration);
            as.flags       = be32toh(as.flags);

            printf("(%05u) %"PRId64" %"PRId64" %x ",
                   i,
                   as.start_time, as.duration, as.flags);
            if (verbose) {
                printf(" %s", as.json);
            }
            putchar('\n');
        }
    }
    freeReplyObject(reply);

    redisFree(c);

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o schedule schedule.c -lhiredis" */
/* End: */
