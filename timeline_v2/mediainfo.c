#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <jansson.h>

#define MAX_KEY_LEN         1024

#define MANIFEST_MAX_SIZE   (1024*64) /* Max size of inline reads */

static void display_usage()
{
    fprintf(stderr,
            "Usage: mediainfo [OPTION]\n\n"
            " From REDIS\n"
            "  -h <hostname>      Server hostname (default: 127.0.0.1)\n"
            "  -p <port>          Server port (default: 6379)\n"
            "  -c <channel>       the same as `-k <channel>/mediainfo.json'\n"
            "  -k <key>           Key\n"
            " From FILE\n"
            "  -f <file>          Input RDB dump file, `-' means STDIN\n\n");
}

#define MI_ASETS_KEY "adaptationSets"
#define MI_REPS_KEY  "representations"

static void
display_aset_track_names(json_t *aset)
{
    const char *key;
    json_t *val;

    json_object_foreach(aset, key, val) {
        if (strcmp(key, MI_REPS_KEY) == 0) {
            size_t ntrk;
            size_t i;

            ntrk = json_array_size(val);
            for (i = 0; i < ntrk; i++) {
                json_t *trk;
                trk = json_array_get(val, i);
                if (json_is_string(trk)) {
                    printf("%s\n", json_string_value(trk));
                }
            }
        }
    }
}

static void
display_all_track_names(const char *text)
{
    json_t *root;
    json_error_t error;

    /* parse */
    root = json_loads(text, 0, &error);
    if (root == NULL) {
        fprintf(stderr, "Error: Can't parse mediainfo\n");
        exit(EXIT_FAILURE);
    }

    if (json_is_object(root)) {
        const char *key1;
        json_t *val1;

        json_object_foreach(root, key1, val1) {
            if (strcmp(key1, MI_ASETS_KEY) == 0) {
                if (json_is_object(val1)) {
                    const char *key2;
                    json_t *val2;
                    json_object_foreach(val1, key2, val2) {
                        if (json_is_object(val2)) {
                            display_aset_track_names(val2);
                        }
                    }
                }
            }
        }
    }
}


static void
display_redis_track_names(const char *hostname, int port, const char *key)
{
    redisContext *c;
    redisReply *reply;
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

    reply = redisCommand(c, "HGET %s file", key);
    if (reply->type == REDIS_REPLY_STRING) {
        display_all_track_names(reply->str);
    }
    freeReplyObject(reply);

    redisFree(c);
}


static void
display_file_track_names(const char *filename)
{
    char *text = NULL;
    FILE *fp;
    int fsize = 0;

    /* read from file */
    fp = fopen(filename, "r");
    if(fp == NULL) {
        fprintf(stderr, "Error: Can't read mediainfo from %s\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0, SEEK_END);
    fsize = ftell(fp);
    rewind(fp);
    text = (char*) malloc(sizeof(char) * fsize);
    fread(text, 1, fsize, fp);
    fclose(fp);

    /* display */
    display_all_track_names(text);

    free(text);
}


int
main(int argc, char **argv)
{
    int opt;
    const char *key = NULL;
    char keybuf[MAX_KEY_LEN];
    int port = 6379;
    const char *hostname = "127.0.0.1";
    const char *filename = NULL;

    while ((opt = getopt(argc, argv, "c:f:k:h:p:")) != -1) {
        switch (opt) {
        case 'f':
            filename = optarg;
            break;
        case 'c':
            /* XXX Do not check for exceeding length */
            snprintf(keybuf, MAX_KEY_LEN, "%s/mediainfo.json", optarg);
            key = keybuf;
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
        display_redis_track_names(hostname, port, key);
    } else if (filename != NULL) {
        display_file_track_names(filename);
    }

    return 0;
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o mediainfo mediainfo.c -lhiredis -ljansson" */
/* End: */
