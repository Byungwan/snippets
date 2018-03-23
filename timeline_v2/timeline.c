#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>           /* PRId64 */
#include <endian.h>             /* htobe64, be64toh */
#include <limits.h>             /* PATH_MAX */
#include <time.h>
#include <errno.h>
#include <hiredis/hiredis.h>

#define PROTO_INLINE_MAX_SIZE   (1024*64) /* Max size of inline reads */
#define CR_NL_SIZE              2

#define TIMESCALE               10000000

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
static int human = 0;
static int utc = 0;
static int verbose = 0;


static void display_usage()
{
    fprintf(stderr,
            "Usage: timeline [OPTION]\n\n"
            " From REDIS\n"
            "  -h <hostname>      Server hostname (default: 127.0.0.1)\n"
            "  -p <port>          Server port (default: 6379)\n"
            "  -k <key>           Key\n"
            "  -s <time>          start time (inclusive)\n"
            "  -e <time>          end time (exclusive)\n"
            " From FILE\n"
            "  -f <file>          Input RDB dump file, `-' means STDIN\n"
            " Generic options\n"
            "  -H                 human readable format\n"
            "  -U                 UTC\n"
            "  -V                 more verbose\n\n");
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

    if (human) {
        time_t start_t = st.start_time / TIMESCALE;
        struct tm start_tm;
        if (utc)
            gmtime_r(&start_t, &start_tm);
        else
            localtime_r(&start_t, &start_tm);

        printf("%04d-%02d-%02d_%02d:%02d:%02d %u %"PRId64" %x %"PRId64" %u %u %u ",
               start_tm.tm_year + 1900, start_tm.tm_mon + 1, start_tm.tm_mday,
               start_tm.tm_hour, start_tm.tm_min, start_tm.tm_sec,
               st.seq, st.duration, st.flags, st.offset,
               st.ts_size, st.mdat_offset, st.size);
    } else {
        printf("%"PRId64" %u %"PRId64" %x %"PRId64" %u %u %u ",
               st.start_time, st.seq, st.duration, st.flags, st.offset,
               st.ts_size, st.mdat_offset, st.size);
    }

    if (verbose) {
        for (i = 0; i < sizeof(st.key_id); i++) {
            putchar(hexconvtab[st.key_id[i] >> 4]);
            putchar(hexconvtab[st.key_id[i] & 15]);
        }
        printf(" %s", st.path);
    }
    putchar('\n');
}


static long str2ll(const char *str)
{
    char *endptr;
    long long val;

    val = strtoll(str, &endptr, 10);

    /* Check for various possible errors */
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    if (endptr == str) {
        fprintf(stderr, "Error: No digits were found\n");
        exit(EXIT_FAILURE);
    }

    return val;
}


static void display_redis_timeline(const char *hostname, int port,
                                   const char *key, long start, long end)
{
    redisContext *c;
    redisReply *reply;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    unsigned int i;

    c = redisConnectWithTimeout(hostname, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            fprintf(stderr, "Error: %s\n", c->errstr);
            redisFree(c);
        } else {
            fprintf(stderr, "Error: Can't allocate redis context\n");
        }
        exit(EXIT_FAILURE);
    }

    if (start < 0 && end < 0) {
        reply = redisCommand(c, "ZRANGEBYLEX %s - +", key);
    } else if (start < 0) {
        end = be64toh(end);
        reply = redisCommand(c, "ZRANGEBYLEX %s - (%b", key,
                             &end, sizeof(end));
    } else if (end < 0) {
        start = be64toh(start);
        reply = redisCommand(c, "ZRANGEBYLEX %s [%b +", key,
                             &start, sizeof(start));
    } else {
        start = be64toh(start);
        end = be64toh(end);
        reply = redisCommand(c, "ZRANGEBYLEX %s [%b (%b", key,
                             &start, sizeof(start), &end, sizeof(end));
    }
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (i = 0; i < reply->elements; i++) {
            display_timeline(reply->element[i]->str, reply->element[i]->len);
        }
    }
    freeReplyObject(reply);

    redisFree(c);
}


static void display_file_timeline(const char *filename)
{
    FILE *fp;
    char *line;
    char *newline = NULL;
    long multibulklen, bulkcnt, bulklen = -1;

    line = malloc(PROTO_INLINE_MAX_SIZE + CR_NL_SIZE);
    if (line == NULL) {
        fprintf(stderr, "Error: Can't allocate read buffer\n");
        exit(EXIT_FAILURE);
    }

    if (strcmp(filename, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            perror("Error");
            exit(EXIT_FAILURE);
        }
    }

    /* multibulk length */
    if (fgets(line, PROTO_INLINE_MAX_SIZE, fp) == NULL) {
        fprintf(stderr, "Error: too big mbulk count string\n");
        exit(EXIT_FAILURE);
    }
    newline = strchr(line, '\r');
    if (newline == NULL) {
        fprintf(stderr, "Error: too big mbulk count string\n");
        exit(EXIT_FAILURE);
    }
    *newline = '\0';
    if (line[0] != '*') {
        fprintf(stderr, "Error: expected '*', got '%c'\n", line[0]);
        exit(EXIT_FAILURE);
    }
    multibulklen = str2ll(line+1);
    if (multibulklen > 1024*1024) {
        fprintf(stderr, "Error: invalid multibulk length\n");
        exit(EXIT_FAILURE);
    }

    for (bulkcnt = 0; multibulklen > bulkcnt; bulkcnt++) {
        /* bulk length */
        if (fgets(line, PROTO_INLINE_MAX_SIZE, fp) == NULL) {
            fprintf(stderr, "Error: too big bulk count string\n");
            exit(EXIT_FAILURE);
        }
        newline = strchr(line, '\r');
        if (newline == NULL) {
            fprintf(stderr, "Error: too big bulk count string\n");
            exit(EXIT_FAILURE);
        }
        *newline = '\0';
        if (line[0] != '$') {
            fprintf(stderr, "Error: expected '$', got '%c'\n", line[0]);
            exit(EXIT_FAILURE);
        }
        bulklen = str2ll(line+1);
        /* XXX Use PROTO_INLINE_MAX_SIZE instead of 512*1024*1024 */
        if (bulklen < 0 || bulklen > PROTO_INLINE_MAX_SIZE) {
            fprintf(stderr, "Error: invalid bulk length\n");
            exit(EXIT_FAILURE);
        }

        /* bulk argument */
        if (fread(line, bulklen+CR_NL_SIZE, 1, fp) != 1) {
            fprintf(stderr, "Error: Not enough bulk data\n");
            exit(EXIT_FAILURE);
        }
        line[bulklen] = '\0';
        if (bulkcnt == 0) {
            if (strcmp(line, "ZADD") != 0) {
                fprintf(stderr, "Error: expected 'ZADD', got '%s'\n", line);
                exit(EXIT_FAILURE);
            }
        } else if (bulkcnt == 1) {
            if (verbose) {
                printf("key : '%s'\n", line);
            }
        } else {
            if ((bulkcnt % 2) == 1) {
                display_timeline(line, bulklen);
            } else {
                /* score  */
                if (strcmp(line, "0") != 0) {
                    fprintf(stderr, "Error: expected '0', got '%s'\n", line);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    free(line);
    if (fp != NULL) {
        if (fp != stdin)
            fclose(fp);
    }
}


int main(int argc, char **argv)
{
    int opt;
    const char *key = NULL;
    int port = 6379;
    const char *hostname = "127.0.0.1";
    const char *filename = NULL;
    long start = -1;
    long end = -1;

    while ((opt = getopt(argc, argv, "f:k:h:p:s:e:HUV")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        case 'k':
            key = optarg;
            break;
        case 'h':
            hostname = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 's':
            start = str2ll(optarg);
            break;
        case 'e':
            end = str2ll(optarg);
            break;
        case 'H':
            human = 1;
            break;
        case 'U':
            utc = 1;
            break;
        case 'V':
            verbose = 1;
            break;
        default: /* '?' */
            display_usage();
            exit(EXIT_FAILURE);
        }
    }

    if (key == NULL && filename == NULL) {
        display_usage();
        exit(EXIT_FAILURE);
    }

    if (key != NULL) {
        display_redis_timeline(hostname, port, key, start, end);
    } else if (filename != NULL) {
        display_file_timeline(filename);
    }

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o timeline timeline.c -lhiredis" */
/* End: */
