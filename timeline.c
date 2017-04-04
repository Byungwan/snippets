#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>           /* PRId64 */
#include <endian.h>             /* htobe64 */
#include <limits.h>             /* PATH_MAX */
#include <hiredis/hiredis.h>

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

static int verbose = 0;

static void display_usage()
{
    fprintf(stderr,
            "Usage: timeline [OPTION]\n\n"
            " From REDIS\n"
            "  -h <hostname>      Server hostname (default: 127.0.0.1)\n"
            "  -p <port>          Server port (default: 6379)\n"
            "  -k <key>           Key\n"
            " From FILE\n"
            "  -f <file>          Input RDB dump file, `-' means STDIN\n\n");
}

static void display_timeline(void *data, size_t size)
{
    seg_time_t st;
    unsigned int i;

    memset(&st, 0, sizeof(st));
    memcpy(&st, data, size);

    st.start_time  = be64toh(st.start_time);
    st.seq         = be32toh(st.seq);
    st.duration    = be64toh(st.duration);
    st.flags       = be32toh(st.flags);
    st.offset      = be64toh(st.offset);
    st.ts_size     = be32toh(st.ts_size);
    st.mdat_offset = be32toh(st.mdat_offset);
    st.size        = be32toh(st.size);

    printf("%"PRId64" %u %"PRId64" %x %"PRId64" %u %u %u ",
           st.start_time, st.seq, st.duration, st.flags, st.offset, st.ts_size, st.mdat_offset, st.size);
    if (verbose) {
        for (i = 0; i < sizeof(st.key_id); i++) {
            putchar(hexconvtab[st.key_id[i] >> 4]);
            putchar(hexconvtab[st.key_id[i] & 15]);
        }
        printf(" %s", st.path);
    }
    putchar('\n');
}

static void redis_timeline(const char *hostname, int port, const char *key)
{
    redisContext *c;
    redisReply *reply;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    unsigned int i;

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(EXIT_FAILURE);
    }

    reply = redisCommand(c, "ZRANGE %s 0 -1", key);
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (i = 0; i < reply->elements; i++) {
            display_timeline(reply->element[i]->str, reply->element[i]->len);
        }
    }
    freeReplyObject(reply);

    redisFree(c);
}

int main(int argc, char **argv) {
    int opt;
    const char *key = NULL;
    int port = 6379;
    const char *hostname = "127.0.0.1";

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
            display_usage();
            exit(EXIT_FAILURE);
        }
    }

    if (key == NULL) {
        display_usage();
        exit(EXIT_FAILURE);
    }

    if (key != NULL) {
        redis_timeline(hostname, port, key);
    }

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o timeline timeline.c -lhiredis" */
/* End: */
