#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>           /* PRId64 */
#include <endian.h>             /* htobe64 */
#include <limits.h>             /* PATH_MAX */
#include <errno.h>
#include <hiredis/hiredis.h>

#define PROTO_INLINE_MAX_SIZE   (1024*64) /* Max size of inline reads */
#define CR_NL_SIZE              2

#define JSON_MAX 10000

typedef struct __attribute__((__packed__)) ass_schd_s {
    uint64_t start_time;
    uint64_t duration;
    uint32_t flags;
    char     json[JSON_MAX+1];
} ass_schd_t;

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


static void display_schedule(void *data, size_t size)
{
    ass_schd_t as;

    memset(&as, 0, sizeof(as));
    memcpy(&as, data, size);

    as.start_time  = be64toh(as.start_time);
    as.duration    = be64toh(as.duration);
    as.flags       = be32toh(as.flags);

    printf("%"PRId64" %"PRId64" %x ", as.start_time, as.duration, as.flags);
    if (verbose) {
        printf(" %s", as.json);
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



static void display_redis_schedule(const char *hostname, int port,
                                   const char *key)
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
        exit(1);
    }

    reply = redisCommand(c, "ZRANGE %s 0 -1", key);
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (i = 0; i < reply->elements; i++) {
            display_schedule(reply->element[i]->str, reply->element[i]->len);
        }
    }
    freeReplyObject(reply);

    redisFree(c);
}


static void display_file_schedule(const char *filename)
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
                display_schedule(line, bulklen);
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

    while ((opt = getopt(argc, argv, "f:k:h:p:V")) != -1) {
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
        case 'V':
            verbose = 1;
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: schedule KEY [PORT] [HOSTNAME]\n");
            exit(EXIT_FAILURE);
        }
    }

    if (key == NULL && filename == NULL) {
        display_usage();
        exit(EXIT_FAILURE);
    }

    if (key != NULL) {
        display_redis_schedule(hostname, port, key);
    } else if (filename != NULL) {
        display_file_schedule(filename);
    }

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o schedule schedule.c -lhiredis" */
/* End: */
