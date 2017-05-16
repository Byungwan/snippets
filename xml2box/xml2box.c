#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
#include <endian.h>
#include <errno.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#ifdef WORDS_BIGENDIAN
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)d) | (((uint32_t)c) << 8)               \
     | (((uint32_t)b) << 16) | (((uint32_t)a) << 24))
#else
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)a) | (((uint32_t)b) << 8)               \
     | (((uint32_t)c) << 16) | (((uint32_t)d) << 24))
#endif

#define MAKE_XMLCHAR(s) (const xmlChar *)s

#define XML_NODE_ISOMF    MAKE_XMLCHAR("IsoMediaFile")

#define XML_NODE_BOX      MAKE_XMLCHAR("BoxInfo")
#define XML_ATTR_BOX_SZ   MAKE_XMLCHAR("Size")
#define XML_ATTR_BOX_TYPE MAKE_XMLCHAR("Type")

#define XML_NODE_FBOX     MAKE_XMLCHAR("FullBoxInfo")
#define XML_ATTR_BOX_VER  MAKE_XMLCHAR("Version")
#define XML_ATTR_BOX_FLGS MAKE_XMLCHAR("Flags")

#define XML_NODE_SIDX               MAKE_XMLCHAR("SegmentIndexBox")
#define XML_ATTR_SIDX_REF_ID        MAKE_XMLCHAR("reference_ID")
#define XML_ATTR_SIDX_TM_SCAL       MAKE_XMLCHAR("timescale")
#define XML_ATTR_SIDX_ERLY_PRES_TM  MAKE_XMLCHAR("earliest_presentation_time")
#define XML_ATTR_SIDX_FRST_OFFS     MAKE_XMLCHAR("first_offset")

#define XML_NODE_SIDX_REF           MAKE_XMLCHAR("Reference")
#define XML_ATTR_SIDX_REF_TYPE      MAKE_XMLCHAR("type")
#define XML_ATTR_SIDX_REF_SZ        MAKE_XMLCHAR("size")
#define XML_ATTR_SIDX_SSEG_DUR      MAKE_XMLCHAR("duration")
#define XML_ATTR_SIDX_STRT_SAP      MAKE_XMLCHAR("startsWithSAP")
#define XML_ATTR_SIDX_SAP_TYPE      MAKE_XMLCHAR("SAP_type")
#define XML_ATTR_SIDX_SAP_DELTA_TM  MAKE_XMLCHAR("SAPDeltaTime")

#define BOX_TYPE_SIDX FOURCC('s', 'i', 'd', 'x')
#define BOX_TYPE_UUID FOURCC('u', 'u', 'i', 'd')

typedef uint8_t bool_t;
typedef uint32_t uint24_t;

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

long xmlGetPropLong(xmlNodePtr node, const xmlChar *name)
{
    const char *str;
    char *endptr;
    long val;

    str = (const char *)xmlGetProp(node, name);
    if (str == NULL) {
        fprintf(stderr, "no attribute was found: %s\n", name);
        return LONG_MAX;
    }

    errno = 0;
    val = strtol(str, &endptr, 10);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("strtol");
        errno = 0;
        return LONG_MAX;
    }

    if (endptr == str) {
        fprintf(stderr, "no digits were found: %s='%s'\n", name, str);
        return LONG_MAX;
    }

    return val;
}

long xmlGetPropULong(xmlNodePtr node, const xmlChar *name)
{
    long val = xmlGetPropLong(node, name);
    if (val < 0) {
        fprintf(stderr, "negative number was found: %s='%ld'\n", name, val);
        return LONG_MAX;
    }
    return val;
}

long xmlGetPropHexLong(xmlNodePtr node, const xmlChar *name)
{
    const char *str;
    char *endptr;
    long val;

    str = (const char *)xmlGetProp(node, name);
    if (str == NULL) {
        fprintf(stderr, "no attribute was found: %s\n", name);
        return LONG_MAX;
    }

    errno = 0;
    val = strtol(str, &endptr, 16);

    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("strtol");
        errno = 0;
        return LONG_MAX;
    }

    if (endptr == str) {
        fprintf(stderr, "no digits were found: %s='%s'\n", name, str);
        return LONG_MAX;
    }

    return val;
}

long xmlGetPropHexULong(xmlNodePtr node, const xmlChar *name)
{
    long val = xmlGetPropHexLong(node, name);
    if (val < 0) {
        fprintf(stderr, "negative number was found: %s='%ld'\n", name, val);
        return LONG_MAX;
    }
    return val;
}

long long xmlGetPropLongLong(xmlNodePtr node, const xmlChar *name)
{
    const char *str;
    char *endptr;
    long long val;

    str = (const char *)xmlGetProp(node, name);
    if (str == NULL) {
        fprintf(stderr, "no attribute was found: %s\n", name);
        return LLONG_MAX;
    }

    errno = 0;
    val = strtoll(str, &endptr, 10);

    if ((errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN))
        || (errno != 0 && val == 0)) {
        perror("strtol");
        errno = 0;
        return LLONG_MAX;
    }

    if (endptr == str) {
        fprintf(stderr, "no digits were found: %s='%s'\n", name, str);
        return LLONG_MAX;
    }

    return val;
}

long long xmlGetPropULongLong(xmlNodePtr node, const xmlChar *name)
{
    long long val = xmlGetPropLongLong(node, name);
    if (val < 0) {
        fprintf(stderr, "negative number was found: %s='%"PRId64"'\n",
                name, val);
        return LONG_MAX;
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

int read_full_box(xmlNodePtr xml_node, full_box_t *box)
{
    box->ver = xmlGetPropULong(xml_node, XML_ATTR_BOX_VER);
    if (box->ver > 1) {
        fprintf(stderr, "box %s error: %d\n", XML_ATTR_BOX_VER, box->ver);
        return -1;
    }

    /* XXX endian? */
    box->flgs = xmlGetPropHexULong(xml_node, XML_ATTR_BOX_FLGS);
    if (box->flgs == LONG_MAX) {
        fprintf(stderr, "box %s error: %d\n", XML_ATTR_BOX_FLGS, box->flgs);
        return -1;
    }

    return 0;
}

void free_sidx_box(sidx_box_t *)
{
    if (sidx->refs)
        free(sidx->refs);
    free(sidx);
}

sidx_box_t *read_sidx_box(xmlNodePtr xml_node)
{
    xmlNodePtr curr_node = NULL;
    sidx_box_t *sidx = NULL;
    sidx_ref_t *ref = NULL;
    uint64_t sz;

    sidx = calloc(1, sizeof(sidx_box_t));
    if (sidx == NULL) {
        fprintf(stderr, "sidx malloc error\n");
        exit(EXIT_FAILURE);
    }

    sidx->ref_id = xmlGetPropULong(xml_node, XML_ATTR_SIDX_REF_ID);
    if (sidx->ref_id == LONG_MAX) {
        fprintf(stderr, "sidx %s error: %d\n", XML_ATTR_SIDX_REF_ID,
                sidx->ref_id);
        goto error;
    }

    sidx->tm_scal = xmlGetPropULong(xml_node, XML_ATTR_SIDX_TM_SCAL);
    if (sidx->tm_scal == LONG_MAX) {
        fprintf(stderr, "sidx %s error: %d\n", XML_ATTR_SIDX_TM_SCAL,
                sidx->tm_scal);
        goto error;
    }

    sidx->erly_pres_tm = xmlGetPropULongLong(xml_node,
                                             XML_ATTR_SIDX_ERLY_PRES_TM);
    if (sidx->erly_pres_tm == LLONG_MAX) {
        fprintf(stderr, "sidx %s error: %"PRId64"\n",
                XML_ATTR_SIDX_ERLY_PRES_TM, sidx->erly_pres_tm);
        goto error;
    }

    sidx->frst_offs = xmlGetPropULongLong(xml_node, XML_ATTR_SIDX_FRST_OFFS);
    if (sidx->frst_offs == LLONG_MAX) {
        fprintf(stderr, "sidx %s error: %"PRId64"\n",
                XML_ATTR_SIDX_FRST_OFFS, sidx->frst_offs);
        goto error;
    }

    /* count reference items */
    for (curr_node = xml_node->children; curr_node;
         curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(curr_node->name, XML_NODE_SIDX_REF) == 0) {
                sidx->ref_cnt++;
                if (sidx->ref_cnt > UINT16_MAX) {
                    fprintf(stderr, "too many sidx references\n");
                    goto error;
                }
            }
        }
    }

    sidx->refs = calloc(sidx->ref_cnt, sizeof(sidx_ref_t));
    if (sidx->refs == NULL) {
        fprintf(stderr, "sidx_refs malloc error\n");
        exit(EXIT_FAILURE);
    }

    ref = sidx->refs;
    for (curr_node = xml_node->children; curr_node;
         curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(curr_node->name, XML_NODE_FBOX) == 0) {
                if (read_full_box(curr_node, &sidx->full_box) != 0) {
                    fprintf(stderr, "sidx full_box error\n");
                    goto error;
                }
            } else if (xmlStrcmp(curr_node->name, XML_NODE_SIDX_REF) == 0) {
                ref->ref_type = xmlGetPropULong(curr_node,
                                                XML_ATTR_SIDX_REF_TYPE);
                if (ref->ref_type > 1) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_REF_TYPE, ref->ref_type);
                    goto error;
                }
                ref->ref_sz = xmlGetPropULong(curr_node, XML_ATTR_SIDX_REF_SZ);
                if (ref->ref_sz == LONG_MAX) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_REF_SZ, ref->ref_sz);
                    goto error;
                }
                ref->sseg_dur = xmlGetPropULong(curr_node,
                                                XML_ATTR_SIDX_SSEG_DUR);
                if (ref->sseg_dur == LONG_MAX) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_SSEG_DUR, ref->sseg_dur);
                    goto error;
                }
                ref->strt_sap = xmlGetPropULong(curr_node,
                                                XML_ATTR_SIDX_STRT_SAP);
                if (ref->strt_sap > 1) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_STRT_SAP, ref->strt_sap);
                    goto error;
                }
                ref->sap_type = xmlGetPropULong(curr_node,
                                                XML_ATTR_SIDX_SAP_TYPE);
                if (ref->sap_type > 0x7) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_SAP_TYPE, ref->sap_type);
                    goto error;
                }
                ref->sap_delta_tm = xmlGetPropULong(curr_node,
                                                    XML_ATTR_SIDX_SAP_DELTA_TM);
                if (ref->sap_delta_tm > 0xFFFFFFF) {
                    fprintf(stderr, "sidx ref %s error: %d\n",
                            XML_ATTR_SIDX_SAP_DELTA_TM, ref->sap_delta_tm);
                    goto error;
                }
                ref++;
            }
        }
    }

    sidx->full_box.base_box.type = BOX_TYPE_SIDX;
    sz = get_full_box_size(&sidx->full_box);
    sz += 8 + 8 + 4 + (12 * sidx->ref_cnt);
    if (sidx->full_box.ver != 0) {
        sz += 8;
    }
    set_box_size(&sidx->full_box.base_box, sz);

    return sidx;

error:
    free_sidx_box(sidx);
    return NULL;
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

int write_boxes(xmlNodePtr node, FILE *fp)
{
    xmlNodePtr curr_node = NULL;
    int rc;
    for (curr_node = node; curr_node; curr_node = curr_node->next) {
        if (curr_node->type == XML_ELEMENT_NODE) {
            if (xmlStrcmp(curr_node->name, XML_NODE_SIDX) == 0) {
                sidx_box_t *sidx;

                sidx = read_sidx_box(curr_node);
                if (sidx == NULL) {
                    fprintf(stderr, "sidx read error\n");
                    return -1;
                }

                rc = write_sidx_box(sidx, fp);
                free_sidx_box(sidx);
                if (rc != 0) {
                    fprintf(stderr, "sidx write error\n");
                    return rc;
                }
            }
        }
        write_boxes(curr_node->children, fp);
    }

    return 0;
}

int main(int argc, const char *argv[])
{
    const char *xmlfile;
    const char *boxfile;
    xmlDocPtr doc = NULL;
    xmlNodePtr root = NULL;
    FILE *fp = NULL;

    if (argc < 3) {
        fprintf(stderr, "Usage: xml2box [XML-FILE] [BOX-FILE]\n");
        exit(EXIT_FAILURE);
    }

    xmlfile = argv[1];
    boxfile = argv[2];

    LIBXML_TEST_VERSION;

    doc = xmlReadFile(xmlfile, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "xml loading failed: %s\n", xmlfile);
        goto error;
    }
    root = xmlDocGetRootElement(doc);
    if (root == NULL) {
        fprintf(stderr, "invalid xml: %s\n", xmlfile);
        goto error;
    }
    if (xmlStrcmp(root->name, XML_NODE_ISOMF)) {
        fprintf(stderr, "invalid bmff xml: %s\n", xmlfile);
        goto error;
    }

    fp = fopen(boxfile, "w");
    if (fp == NULL) {
        fprintf(stderr, "file open error: %s\n", boxfile);
        goto error;
    }

    if (write_boxes(root->children, fp) != 0) {
        fprintf(stderr, "box write error: %s\n", boxfile);
        goto error;
    }

    fclose(fp);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return EXIT_SUCCESS;

error:
    if (fp)
        fclose(fp);
    if (doc)
        xmlFreeDoc(doc);
    xmlCleanupParser();

    exit(EXIT_FAILURE);
}

/* Local Variables: */
/* compile-command:"gcc -Wall -g $(xml2-config --cflags) -o xml2box xml2box.c $(xml2-config --libs)" */
/* End: */
