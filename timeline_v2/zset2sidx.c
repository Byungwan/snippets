#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <limits.h>
#include <endian.h>
#include <errno.h>
#include <hiredis/hiredis.h>

#define REDIS_URI_MAX 2048
#define REDIS_SCHEME "redis://"

#define INISOFT_TIMESCALE 10000000

#ifdef WORDS_BIGENDIAN
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)d) | (((uint32_t)c) << 8)               \
     | (((uint32_t)b) << 16) | (((uint32_t)a) << 24))
#else
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)a) | (((uint32_t)b) << 8)               \
     | (((uint32_t)c) << 16) | (((uint32_t)d) << 24))
#endif

#define BOX_TYPE_SIDX FOURCC('s', 'i', 'd', 'x')
#define BOX_TYPE_UUID FOURCC('u', 'u', 'i', 'd')

typedef uint8_t bool_t;
typedef uint32_t uint24_t;

typedef struct seginfo_s {
    uint64_t tm;
    uint32_t seq;
    uint32_t dur;
    uint32_t flgs;
    uint32_t offs;
    uint32_t ts_sz;
    uint32_t mdat_sz;
    uint32_t seg_sz;
    uint8_t  key_id[16];
    char     data[0];
} seginfo_t;

typedef struct base_box_s {
    uint32_t sz;
    uint32_t type;
    uint64_t sz64;
    uint8_t  ext_type[16];
} base_box_t;

typedef struct full_box_s {
    base_box_t base_box;

    uint8_t  ver;
    uint32_t flgs;             /* 24 bits */
} full_box_t;

typedef struct sidx_ref_s {
    bool_t   ref_type;
    uint32_t ref_sz;            /* 31 bits */
    uint32_t sseg_dur;
    bool_t   strt_sap;
    uint8_t  sap_type;          /* 3 bits */
    uint32_t sap_delta_tm;      /* 28 bits */
} sidx_ref_t;

typedef struct sidx_box_s {
    full_box_t full_box;

    uint32_t ref_id;
    uint32_t tm_scal;
    uint64_t erly_pres_tm;
    uint64_t frst_offs;
    uint16_t rsv;
    uint16_t ref_cnt;

    sidx_ref_t *refs;
} sidx_box_t;

int string2port(const char *str)
{
    char *endptr;
    long val;

    errno = 0;
    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("strtol");
        errno = 0;
        return -1;
    }

    if (endptr == str) {
        fprintf(stderr, "no digits were found: '%s'\n", str);
        return -1;
    }

    return val;

}

int write_u8(uint8_t n, FILE *fp)
{
    if (fwrite(&n, sizeof(n), 1, fp) != 1)
        return -1;
    return 0;
}

int write_u16(uint16_t n, FILE *fp)
{
    uint16_t b16 = htobe16(n);
    if (fwrite(&b16, sizeof(b16), 1, fp) != 1)
        return -1;
    return 0;
}

int write_u24(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    b32 <<= 8;
    if (fwrite(&b32, sizeof(b32) - 1, 1, fp) != 1)
        return -1;
    return 0;
}

int write_u32(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    if (fwrite(&b32, sizeof(b32), 1, fp) != 1)
        return -1;
    return 0;
}

int write_u64(uint64_t n, FILE *fp)
{
    uint64_t b64 = htobe64(n);
    if (fwrite(&b64, sizeof(b64), 1, fp) != 1)
        return -1;
    return 0;
}

uint64_t get_base_box_size(base_box_t *box)
{
    uint64_t sz = 8;
    if (box->type == BOX_TYPE_UUID)
        sz += 16;
    return sz;
}

uint64_t get_full_box_size(full_box_t *box)
{
    return get_base_box_size(&box->base_box) + 4;
}

void set_box_size(base_box_t *box, uint64_t sz)
{

    if (sz > UINT32_MAX) {
        box->sz = 1;
        box->sz64 = sz + 8;
    } else {
        box->sz = sz;
        box->sz64 = 0;
    }
}

void free_sidx_box(sidx_box_t *sidx)
{
    if (sidx->refs)
        free(sidx->refs);
    free(sidx);
}

sidx_box_t *read_sidx_box(redisContext *redis, const char *keyname)
{
    sidx_box_t *sidx = NULL;
    sidx_ref_t *ref = NULL;
    seginfo_t seg, seg_be;
    redisReply *reply;
    uint64_t sz;
    unsigned int i;

    reply = redisCommand(redis, "ZRANGE %s 0 -1", keyname);
    if (reply->type != REDIS_REPLY_ARRAY) {
        fprintf(stderr, "failed to sidx malloc error\n");
        return NULL;
    }

    sidx = calloc(1, sizeof(sidx_box_t));
    if (sidx == NULL) {
        fprintf(stderr, "sidx malloc error\n");
        exit(EXIT_FAILURE);
    }

    sidx->ref_cnt = reply->elements;
    sidx->refs = calloc(sidx->ref_cnt, sizeof(sidx_ref_t));
    if (sidx->refs == NULL) {
        fprintf(stderr, "sidx_refs malloc error\n");
        exit(EXIT_FAILURE);
    }

    sidx->full_box.base_box.type = BOX_TYPE_SIDX;
    sz = get_full_box_size(&sidx->full_box);
    sz += 8 + 8 + 4 + (12 * sidx->ref_cnt);
    if (sidx->full_box.ver != 0) {
        sz += 8;
    }
    set_box_size(&sidx->full_box.base_box, sz);

    /* XXX hard coding */
    sidx->full_box.ver = 1;
    sidx->full_box.flgs = 0;
    sidx->ref_id = 1;
    sidx->tm_scal = INISOFT_TIMESCALE;
    sidx->erly_pres_tm = 0;
    sidx->frst_offs = 0;

    for (i = 0; i < reply->elements; i++) {
        memcpy(&seg_be, reply->element[i]->str, sizeof(seg_be));
        seg.dur = be32toh(seg_be.dur);
        seg.seg_sz = be32toh(seg_be.seg_sz);

        ref->ref_type = 0;
        ref->ref_sz = seg.offs;
        ref->sseg_dur = seg.dur;
        ref->strt_sap = 1;  /* XXX hard coding */
        ref->sap_type = 0;
        ref->sap_delta_tm = 0;
    }
    freeReplyObject(reply);

    return sidx;
}

int write_base_box(base_box_t *box, FILE *fp)
{
    int rc;

    rc = write_u32(box->sz, fp);
    if (rc != 0) {
        fprintf(stderr, "box size write error\n");
        return rc;
    }

    rc = fwrite(&box->type, sizeof(box->type), 1, fp);
    if (rc != 1) {
        fprintf(stderr, "box type write error\n");
        return -1;
    }

    if (box->sz == 1) {
        rc = write_u64(box->sz64, fp);
        if (rc != 0) {
            fprintf(stderr, "box largesize write error\n");
            return rc;
        }
    } else if (box->sz == 0) {
        /* TODO: not supported yet */
        return -1;
    }

    return 0;
}

int write_full_box(full_box_t *box, FILE *fp)
{
    int rc;

    rc = write_base_box(&box->base_box, fp);
    if (rc != 0) {
        return rc;
    }

    rc = write_u8(box->ver, fp);
    if (rc != 0) {
        fprintf(stderr, "box version write error\n");
        return rc;
    }

    /* XXX endian? */
    rc = write_u24(box->flgs, fp);
    if (rc != 0) {
        fprintf(stderr, "box flags write error\n");
        return rc;
    }

    return 0;
}

int write_sidx_box(sidx_box_t *box, FILE *fp)
{
    int rc;
    int i;

    rc = write_full_box(&box->full_box, fp);
    if (rc != 0) {
        return rc;
    }

    rc = write_u32(box->ref_id, fp);
    if (rc != 0) {
        fprintf(stderr, "sidx reference_ID write error\n");
        return rc;
    }

    rc = write_u32(box->tm_scal, fp);
    if (rc != 0) {
        fprintf(stderr, "sidx timescale write error\n");
        return rc;
    }

    if (box->full_box.ver == 0) {
        rc = write_u32(box->erly_pres_tm, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx earliest_presentation_time write error\n");
            return rc;
        }
        rc = write_u32(box->frst_offs, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx first_offset write error\n");
            return rc;
        }
    } else {
        rc = write_u64(box->erly_pres_tm, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx earliest_presentation_time write error\n");
            return rc;
        }
        rc = write_u64(box->frst_offs, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx first_offset write error\n");
            return rc;
        }
    }
    rc = write_u16(box->rsv, fp);
    if (rc != 0) {
        fprintf(stderr, "sidx reserved write error\n");
        return rc;
    }
    rc = write_u16(box->ref_cnt, fp);
    if (rc != 0) {
        fprintf(stderr, "sidx reference_count write error\n");
        return rc;
    }

    for (i = 0; i < box->ref_cnt; i++) {
        sidx_ref_t *ref = box->refs + i;
        uint32_t u32;

        u32 = (ref->ref_type << 31) + ref->ref_sz;
        rc = write_u32(u32, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx ref(%d) type, size write error\n", i + 1);
            return rc;
        }

        rc = write_u32(ref->sseg_dur, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx ref(%d) duration write error\n", i + 1);
            return rc;
        }

        u32 = ((ref->strt_sap << 31) + (ref->sap_type << 28) + ref->sap_delta_tm);
        rc = write_u32(u32, fp);
        if (rc != 0) {
            fprintf(stderr, "sidx ref(%d) SAP info. write error\n", i + 1);
            return rc;
        }
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    const char *rdsuri;
    const char *boxfile;

    redisContext *redis = NULL;
    char *hostname = NULL;
    char *portstr;
    char *keyname;
    int port = 6379;
    struct timeval timeout = { 1, 500000 }; /* 1.5 seconds */

    FILE *fp = NULL;

    sidx_box_t *sidx = NULL;
    int rc;

    if (argc < 3) {
        fprintf(stderr, "Usage: xml2box [REDIS_URI] [BOX-FILE]\n");
        exit(EXIT_FAILURE);
    }

    rdsuri = argv[1];
    boxfile = argv[2];

    /* parse redis URI */
    if (strncmp(rdsuri, REDIS_SCHEME, sizeof(REDIS_SCHEME)-1)) {
        fprintf(stderr, "invalid redis URI: %s\n", rdsuri);
        goto error;
    }
    hostname = strndup(rdsuri + sizeof(REDIS_SCHEME), REDIS_URI_MAX);
    keyname = strchr(hostname, '/');
    if (keyname == NULL) {
        fprintf(stderr, "no keyname was found: %s\n", rdsuri);
        goto error;
    }
    *keyname = '\0';
    keyname++;
    if (strlen(keyname) == 0) {
        fprintf(stderr, "no keyname was found: %s\n", rdsuri);
        goto error;
    }
    portstr = strchr(hostname, ':');
    if (portstr != NULL) {
        *portstr = '\0';
        portstr++;
        port = string2port(portstr);
    }

    redis = redisConnectWithTimeout(hostname, port, timeout);
    if (redis == NULL || redis->err) {
        if (redis)
            printf("redis connection error: %s\n", redis->errstr);
        else
            printf("redis connection error: can't allocate redis context\n");
        goto error;
    }
    free(hostname);

    fp = fopen(boxfile, "w");
    if (fp == NULL) {
        fprintf(stderr, "file open error: %s\n", boxfile);
        goto error;
    }

    sidx = read_sidx_box(redis, keyname);
    if (sidx == NULL) {
        fprintf(stderr, "sidx read error\n");
        goto error;
    }

    rc = write_sidx_box(sidx, fp);
    free_sidx_box(sidx);
    if (rc != 0) {
        fprintf(stderr, "sidx write error\n");
        goto error;
    }

    fclose(fp);
    redisFree(redis);

    return EXIT_SUCCESS;

error:

    if (sidx)
        free_sidx_box(sidx);
    if (fp)
        fclose(fp);
    if (hostname)
        free(hostname);
    if (redis)
        redisFree(redis);

    exit(EXIT_FAILURE);
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g -o zset2sidx zset2sidx.c -lhiredis" */
/* End: */
