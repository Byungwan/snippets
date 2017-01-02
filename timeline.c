#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>           /* PRId64 */
#include <endian.h>             /* htobe64 */
#include <limits.h>             /* PATH_MAX */
#include <hiredis/hiredis.h>

#define INI_TIMESCALE 10000000

typedef struct __attribute__((__packed__)) seg_time_s {
    uint64_t start_time;
    uint32_t seq;
    uint64_t duration;
    uint32_t flags;
    uint64_t offset;
    uint32_t ts_size;
    uint32_t mdat_offset;
    uint32_t size;
    uint8_t  key_id[16];
    char     path[PATH_MAX+1];
} seg_time_t;

static char hexconvtab[] = "0123456789abcdef";

int main(int argc, char **argv) {
    redisContext *c;
    redisReply *reply;
    const char *key;
    int port;
    const char *hostname;
    seg_time_t st;
    unsigned int i, j;

    if (argc < 2) {
        fprintf(stderr, "Usage: timeline KEY [PORT] [HOSTNAME]\n");
        exit(EXIT_FAILURE);
    }

    key = argv[1];
    port = (argc > 2) ? atoi(argv[2]) : 6379;
    hostname = (argc > 3) ? argv[3] : "127.0.0.1";

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
            memset(&st, 0, sizeof(st));
            memcpy(&st, reply->element[i]->str, reply->element[i]->len);

            st.start_time  = be64toh(st.start_time);
            st.seq         = be32toh(st.seq);
            st.duration    = be64toh(st.duration);
            st.flags       = be32toh(st.flags);
            st.offset      = be64toh(st.offset);
            st.ts_size     = be32toh(st.ts_size);
            st.mdat_offset = be32toh(st.mdat_offset);
            st.size        = be32toh(st.size);

            printf("(%05u) %"PRId64" %u %"PRId64" %x %"PRId64" %u %u %u ",
                   i,
                   st.start_time, st.seq, st.duration, st.flags, st.offset, st.ts_size, st.mdat_offset, st.size);
            for (j = 0; j < sizeof(st.key_id); j++) {
                putchar(hexconvtab[st.key_id[j] >> 4]);
                putchar(hexconvtab[st.key_id[j] & 15]);
            }
            printf("\n");
            printf(" %s\n", st.path);
        }
    }
    freeReplyObject(reply);

    redisFree(c);

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o timeline timeline.c -lhiredis" */
/* End: */
