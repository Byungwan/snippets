#include <Python.h>
#include <structmember.h>

#ifdef WORDS_BIGENDIAN
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)d) | (((uint32_t)c) << 8)               \
     | (((uint32_t)b) << 16) | (((uint32_t)a) << 24))
#else
#   define FOURCC(a, b, c, d)                           \
    (((uint32_t)a) | (((uint32_t)b) << 8)               \
     | (((uint32_t)c) << 16) | (((uint32_t)d) << 24))
#endif

#define BOX_TYPE_FTYP FOURCC('f', 't', 'y', 'p')
#define BOX_TYPE_MOOV FOURCC('m', 'o', 'o', 'v')
#define   BOX_TYPE_MVHD FOURCC('m', 'v', 'h', 'd')
#define   BOX_TYPE_MVEX FOURCC('m', 'v', 'e', 'x')
#define     BOX_TYPE_MEHD FOURCC('m', 'e', 'h', 'd')
#define   BOX_TYPE_TRAK FOURCC('t', 'r', 'a', 'k')
#define     BOX_TYPE_MDIA FOURCC('m', 'd', 'i', 'a')
#define       BOX_TYPE_MDHD FOURCC('m', 'd', 'h', 'd')
#define BOX_TYPE_SIDX FOURCC('s', 'i', 'd', 'x')
#define BOX_TYPE_MOOF FOURCC('m', 'o', 'o', 'f')
#define BOX_TYPE_UUID FOURCC('u', 'u', 'i', 'd')
#define BOX_TYPE_FREE FOURCC('f', 'r', 'e', 'e')

typedef uint8_t bool_t;
typedef uint8_t uint3_t;

typedef uint32_t uint24_t;
typedef uint32_t uint28_t;
typedef uint32_t uint31_t;

#define BMFF_BASE_BOX                           \
    off_t offs;                                 \
    uint64_t sz;                                \
    uint32_t type;

#define BMFF_FULL_BOX                           \
    BMFF_BASE_BOX                               \
    uint8_t  ver;                               \
    uint24_t flgs;

typedef struct base_box_s {
    BMFF_BASE_BOX
} base_box_t;

typedef struct full_box_s {
    BMFF_FULL_BOX
} full_box_t;

/* XXX only support version 1 */
#define SIDX_SIZE_MIN 40
#define SIDX_REF_SIZE 12
#define MAX_SIDX_REF  1000000

typedef struct sidx_ref_s {
    bool_t   ref_type;
    uint31_t ref_sz;
    uint32_t sseg_dur;
    bool_t   strt_sap;
    uint3_t  sap_type;
    uint28_t sap_delta_tm;
} sidx_ref_t;

typedef struct sidx_box_s {
    BMFF_FULL_BOX
    uint32_t ref_id;
    uint32_t tm_scal;
    uint64_t erly_pres_tm;
    uint64_t frst_offs;
    uint16_t rsv;
    uint16_t ref_cnt;
    uint16_t max_ref_cnt;
    sidx_ref_t *refs;
} sidx_box_t;

typedef struct init_seg_s {
    off_t beg_offs;
    off_t end_offs;
    full_box_t mvhd;
    off_t mvhd_dur_offs;
    full_box_t mehd;
    off_t mehd_dur_offs;
    full_box_t mdhd;
    off_t mdhd_dur_offs;
} init_seg_t;

static int
read_u8(uint8_t *np, FILE *fp)
{
    if (fread(np, sizeof(*np), 1, fp) != 1)
        return -1;
    return 1;
}

static int
write_u8(uint8_t n, FILE *fp)
{
    if (fwrite(&n, sizeof(n), 1, fp) != 1)
        return -1;
    return 1;
}

static int
read_u16(uint16_t *np, FILE *fp)
{
    uint16_t b16;
    if (fread(&b16, sizeof(b16), 1, fp) != 1)
        return -1;
    *np = be16toh(b16);
    return 2;
}

static int
write_u16(uint16_t n, FILE *fp)
{
    uint16_t b16 = htobe16(n);
    if (fwrite(&b16, sizeof(b16), 1, fp) != 1)
        return -1;
    return 2;
}

static int
read_u24(uint32_t *np, FILE *fp)
{
    uint32_t b32;
    if (fread(&b32, sizeof(b32)-1, 1, fp) != 1)
        return -1;
    *np = be32toh((b32 >> 8) & 0x00FFFFFF);
    return 3;
}

static int
write_u24(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    b32 <<= 8;
    if (fwrite(&b32, sizeof(b32)-1, 1, fp) != 1)
        return -1;
    return 3;
}

static int
read_u32(uint32_t *np, FILE *fp)
{
    uint32_t b32;
    if (fread(&b32, sizeof(b32), 1, fp) != 1)
        return -1;
    *np = be32toh(b32);
    return 4;
}

static int
write_u32(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    if (fwrite(&b32, sizeof(b32), 1, fp) != 1)
        return -1;
    return 4;
}

static int
read_u64(uint64_t *np, FILE *fp)
{
    uint64_t b64;
    if (fread(&b64, sizeof(b64), 1, fp) != 1)
        return -1;
    *np = be64toh(b64);
    return 8;
}

static int
write_u64(uint64_t n, FILE *fp)
{
    uint64_t b64 = htobe64(n);
    if (fwrite(&b64, sizeof(b64), 1, fp) != 1)
        return -1;
    return 8;
}

static int
read_dummy(int32_t sz, FILE *fp)
{
    uint64_t u64 = 0;
    uint8_t u8 = 0;
    uint32_t ret = 0;
    int32_t i;

    for (i = 0; i < sz / 8; i++) {
        if (fread(&u64, sizeof(u64), 1, fp) != 1)
            return -1;
        ret += 8;
    }
    for (i = 0; i < sz % 8; i++) {
        if (fread(&u8, sizeof(u8), 1, fp) != 1)
            return -1;
        ret++;
    }

    return ret;
}

static int
write_zero(uint32_t sz, FILE *fp)
{
    uint64_t u64 = 0;
    uint8_t u8 = 0;
    uint32_t ret = 0;
    uint32_t i;

    for (i = 0; i < sz / 8; i++) {
        if (fwrite(&u64, sizeof(u64), 1, fp) != 1)
            return -1;
        ret += 8;
    }
    for (i = 0; i < sz % 8; i++) {
        if (fwrite(&u8, sizeof(u8), 1, fp) != 1)
            return -1;
        ret++;
    }

    return ret;
}

static int
read_base_box(off_t offs, base_box_t *box, FILE *fp)
{
    uint32_t sz;
    int nb;

    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (read_u32(&sz, fp) == -1) /* size */
        return -1;
    if (fread(&box->type, sizeof(box->type), 1, fp) != 1) /* type */
        return -1;
    nb = 8;

    if (sz == 1) {
        if (read_u64(&box->sz, fp) == -1) /* largesize */
            return -1;
        nb += 8;
    } else if (sz == 0) {
        return -1;              /* box extends to end of file */
    } else {
        box->sz = sz;
    }

    if (box->type == BOX_TYPE_UUID) {
        if (read_dummy(16, fp) != 16)
            return -1;
        nb += 16;
    }

    box->offs = offs;

    return nb;
}

/* XXX largesize */
static int
write_base_box(off_t offs, base_box_t *box, FILE *fp)
{
    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    box->offs = offs;           /* XXX */

    if (write_u32(box->sz, fp) == -1) /* size */
        return -1;
    if (fwrite(&box->type, sizeof(box->type), 1, fp) != 1) /* type */
        return -1;

    return 4;
}

static int
read_full_box(off_t offs, full_box_t *box, FILE *fp)
{
    int nb;

    nb = read_base_box(offs, (base_box_t *)box, fp);
    if (nb == -1)
        return -1;

    if (read_u8(&box->ver, fp) == -1) /* version */
        return -1;
    if (read_u24(&box->flgs, fp) == -1) /* flags */
        return -1;
    nb += 4;

    return nb;
}

static int
write_full_box(off_t offs, full_box_t *box, FILE *fp)
{
    int nb;

    nb = write_base_box(offs, (base_box_t *)box, fp);
    if (nb == -1)
        return -1;

    if (write_u8(box->ver, fp) == -1)   /* version */
        return -1;
    if (write_u24(box->flgs, fp) == -1)  /* flags */
        return -1;
    nb += 4;

    return nb;
}

static off_t
read_sidx_box(off_t offs, sidx_box_t *sidx, FILE *fp)
{
    sidx_ref_t *ref;
    uint32_t u32;
    uint16_t dmm16;
    int nb;
    int i;

    nb = read_full_box(offs, (full_box_t *)sidx, fp);
    if (nb == -1)
        return -1;

    if (sidx->type != BOX_TYPE_SIDX)
        return -1;
    if (sidx->ver != 1)           /* XXX only support version 1 */
        return -1;

    if (read_u32(&sidx->ref_id, fp) == -1) /* reference_ID */
        return -1;
    if (read_u32(&sidx->tm_scal, fp) == -1) /* timescale */
        return -1;

    if (read_u64(&sidx->erly_pres_tm, fp) == -1) /* earliest_presentation_time */
        return -1;
    if (read_u64(&sidx->frst_offs, fp) == -1) /* first_offset */
        return -1;
    if (read_u16(&dmm16, fp) == -1) /* reserved */
        return -1;
    if (read_u16(&sidx->ref_cnt, fp) == -1) /* reference_count */
        return -1;

    if (sidx->sz != SIDX_SIZE_MIN + (SIDX_REF_SIZE * sidx->ref_cnt))
        return -1;

    sidx->max_ref_cnt = sidx->ref_cnt + sidx->frst_offs / SIDX_REF_SIZE;
    if (sidx->max_ref_cnt > MAX_SIDX_REF)
        return -1;
    sidx->refs = malloc(sizeof(*sidx->refs) * sidx->max_ref_cnt);
    if (sidx->refs == NULL)
        return -1;

    ref = sidx->refs;
    for (i = 0; i < sidx->ref_cnt; i++) {
        if (read_u32(&u32, fp) == -1)
            goto error;
        ref->ref_type = (u32 >> 31) & 0x00000001; /* reference _type */
        ref->ref_sz = u32 & 0x7FFFFFFF;        /* reference _size */
        if (read_u32(&ref->sseg_dur, fp) == -1) /* subsegment_duration */
            goto error;
        if (read_u32(&u32, fp) == -1) /* XXX skip all about SAP */
            goto error;
        ref->strt_sap = 1;
        ref->sap_type = 0;
        ref->sap_delta_tm = 0;
        ref++;
    }

    return sidx->sz;

error:
    free(sidx->refs);
    return -1;
}

static int
write_sidx_box(off_t offs, sidx_box_t *sidx, FILE *fp)
{
    int nb;

    if (sidx->sz != SIDX_SIZE_MIN + (SIDX_REF_SIZE * sidx->ref_cnt))
        return -1;

    sidx->type = BOX_TYPE_SIDX;
    nb = write_full_box(offs, (full_box_t *)sidx, fp);
    if (nb == -1)
        return -1;

    if (write_u32(sidx->ref_id, fp) == -1)  /* reference_ID */
        return -1;
    if (write_u32(sidx->tm_scal, fp) == -1)  /* timescale */
        return -1;
    if (write_u64(sidx->erly_pres_tm, fp) == -1) /* earliest_presentation_time */
        return -1;
    if (write_u64(sidx->frst_offs, fp) == -1) /* first_offset */
        return -1;
    if (write_u16(0, fp) == -1)  /* reserved */
        return -1;
    if (write_u16(sidx->ref_cnt, fp) == -1)  /* reference_count */
        return -1;

    return SIDX_SIZE_MIN;
}

static void
cleanup_sidx_box(sidx_box_t *sidx)
{
    if (sidx->refs) {
        free(sidx->refs);
        sidx->refs = NULL;
    }
}

static int
write_free_box(off_t offs, uint32_t sz, FILE *fp)
{
    base_box_t box;
    int nb;

    box.sz = sz;
    box.type = BOX_TYPE_FREE;

    nb = write_base_box(offs, &box, fp);
    if (nb == -1)
        return -1;

    if (write_zero(sz - 8, fp) != sz - 8)
        return -1;

    return sz;
}

static void
init_iseg(init_seg_t *iseg)
{
    memset(iseg, 0, sizeof(*iseg));
    iseg->beg_offs = -1;
    iseg->end_offs = -1;
    iseg->mvhd.offs = -1;
    iseg->mehd.offs = -1;
    iseg->mdhd.offs = -1;
    iseg->mvhd_dur_offs = -1;
    iseg->mehd_dur_offs = -1;
    iseg->mdhd_dur_offs = -1;
}

static int
update_iseg_dur(init_seg_t *iseg, uint64_t dur, FILE *fp)
{
#define WRITE_U64(d, f) if (write_u64(d, f) == -1) return -1
#define WRITE_U32(d, f) if (write_u32(d, f) == -1) return -1

    if (iseg->mvhd_dur_offs != -1) {
        fseeko(fp, iseg->mvhd_dur_offs, SEEK_SET);
        if (iseg->mvhd.ver == 1) {
            WRITE_U64(dur, fp);
        } else {
            WRITE_U32(dur, fp);
        }
    }

    if (iseg->mehd_dur_offs != -1) {
        fseeko(fp, iseg->mehd_dur_offs, SEEK_SET);
        if (iseg->mehd.ver == 1) {
            WRITE_U64(dur, fp);
        } else {
            WRITE_U32(dur, fp);
        }
    }

    if (iseg->mdhd_dur_offs != -1) {
        fseeko(fp, iseg->mdhd_dur_offs, SEEK_SET);
        if (iseg->mdhd.ver == 1) {
            WRITE_U64(dur, fp);
        } else {
            WRITE_U32(dur, fp);
        }
    }

#undef WRITE_U64
#undef WRITE_U32

    return 0;
}

static int
read_meta_box(off_t offs, init_seg_t *iseg, FILE *fp)
{
    base_box_t box, sub_box;
    ssize_t nb;
    off_t end_offs, cur_offs;

    nb = read_base_box(offs, &box, fp);
    if (nb == -1)
        return -1;

    end_offs = box.offs + box.sz;
    cur_offs = offs + nb;
    while (cur_offs < end_offs) {
        nb = read_base_box(cur_offs, &sub_box, fp);
        if (nb == -1)
            return -1;

        switch (box.type) {
        case BOX_TYPE_MOOV:
            switch (sub_box.type) {
            case BOX_TYPE_MVHD:
                nb = read_full_box(cur_offs, &iseg->mvhd, fp);
                if (nb == -1)
                    return -1;
                iseg->mvhd_dur_offs = (iseg->mvhd.offs + nb
                                       + ((iseg->mvhd.ver == 1) ? 20 : 12));
                break;
            case BOX_TYPE_MVEX:
                nb = read_meta_box(cur_offs, iseg, fp);
                if (nb == -1)
                    return -1;
                break;
            case BOX_TYPE_TRAK:
                nb = read_meta_box(cur_offs, iseg, fp);
                if (nb == -1)
                    return -1;
                break;
            }
            break;

        case BOX_TYPE_MVEX:
            switch (sub_box.type) {
            case BOX_TYPE_MEHD:
                nb = read_full_box(cur_offs, &iseg->mehd, fp);
                if (nb == -1)
                    return -1;
                iseg->mehd_dur_offs = iseg->mehd.offs + nb;
                break;
            }
            break;

        case BOX_TYPE_TRAK:
            switch (sub_box.type) {
            case BOX_TYPE_MDIA:
                nb = read_meta_box(cur_offs, iseg, fp);
                if (nb == -1)
                    return -1;
                break;
            }
            break;

        case BOX_TYPE_MDIA:
            switch (sub_box.type) {
            case BOX_TYPE_MDHD:
                nb = read_full_box(cur_offs, &iseg->mdhd, fp);
                if (nb == -1)
                    return -1;
                iseg->mdhd_dur_offs = (iseg->mdhd.offs + nb
                                       + ((iseg->mdhd.ver == 1) ? 20 : 12));
                break;
            }
            break;
        }
        cur_offs += sub_box.sz;
    }

    return box.sz;
}

static int
read_init_seg(FILE *fp, init_seg_t *iseg)
{
    base_box_t ftyp;
    ssize_t nb, tot = 0;

    rewind(fp);

    nb = read_base_box(0, &ftyp, fp); /* ftyp */
    if (nb == -1)
        return -1;
    if (ftyp.type != BOX_TYPE_FTYP)
        return -1;
    tot += nb;

    /* TODO: check MOOV */
    iseg->beg_offs = ftyp.offs + ftyp.sz;
    nb = read_meta_box(ftyp.offs + ftyp.sz, iseg, fp); /* moov */
    if (nb == -1)
        return -1;
    iseg->end_offs = iseg->beg_offs + nb;

    return iseg->end_offs;
}

static int
write_init_seg(const uint8_t *buf, int sz, FILE *fp)
{
    rewind(fp);
    if (fwrite(buf, 1, sz, fp) != sz) {
        return -1;
    }
    return sz;
}

static int64_t
write_seg(init_seg_t *iseg, sidx_box_t *sidx, int idx, long dur,
          const uint8_t *buf, int sz, FILE *fp)
{
    sidx_ref_t *ref = sidx->refs + idx;
    uint64_t tot_dur;
    off_t ref_offs, seg_offs;
    int nb;
    int i;
    uint32_t u32;

    if (idx > sidx->max_ref_cnt)
        return -1;
    if (idx > sidx->ref_cnt)
        return -1;

    ref->ref_type = 0;
    ref->ref_sz = sz;
    ref->sseg_dur = dur;
    ref->strt_sap = 1;
    ref->sap_type = 0;
    ref->sap_delta_tm = 0;

    ref_offs = sidx->offs + SIDX_SIZE_MIN + (SIDX_REF_SIZE * idx);
    if (fseeko(fp, ref_offs, SEEK_SET) != 0)
        return -1;

    /* write sidx ref */
    u32 = (ref->ref_type << 31) + ref->ref_sz;
    if (write_u32(u32, fp) == -1)/* reference _type and reference _size */
        return -1;
    if (write_u32(ref->sseg_dur, fp) == -1)/* subsegment_duration */
        return -1;
    u32 = ((ref->strt_sap << 31) + (ref->sap_type << 28) + ref->sap_delta_tm);
    if (write_u32(u32, fp) == -1) /* SAP */
        return -1;

    /* update sidx */
    sidx->ref_cnt = idx + 1;
    sidx->sz = SIDX_SIZE_MIN + (SIDX_REF_SIZE * sidx->ref_cnt);
    sidx->frst_offs = SIDX_REF_SIZE * (sidx->max_ref_cnt - sidx->ref_cnt);
    nb = write_sidx_box(sidx->offs, sidx, fp);
    if (nb == -1)
        return -1;

    if (sidx->frst_offs > 0) {
        if (write_free_box(sidx->offs + sidx->sz, sidx->frst_offs, fp) < 0)
            return -1;
    }

    tot_dur = 0;
    seg_offs = sidx->offs + sidx->sz + sidx->frst_offs;
    ref = sidx->refs;
    for (i = 0; i < idx; i++) {
        seg_offs += ref->ref_sz;
        tot_dur += ref->sseg_dur;
        ref++;
    }

    /* write segment */
    if (fseeko(fp, seg_offs, SEEK_SET) != 0)
        return -1;
    if (fwrite(buf, 1, sz, fp) != sz)
        return -1;

    /* update init seg */
    tot_dur += dur;
    if (update_iseg_dur(iseg, tot_dur, fp) == -1)
        return -1;

    return seg_offs;
}

typedef struct {
    PyObject_HEAD
    PyObject *pathname;
    FILE *fp;

    init_seg_t iseg;
    sidx_box_t sidx;
} inimedia_MediaFileObject;

static void
init_file_attr(inimedia_MediaFileObject *self)
{
    self->fp = NULL;
    init_iseg(&self->iseg);
}

static PyObject *
MediaFile_new(PyTypeObject *type, PyObject *args, PyObject *keywords)
{
    inimedia_MediaFileObject *self;
    self = (inimedia_MediaFileObject *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->pathname = PyUnicode_FromString("");
        if (self->pathname == NULL) {
            Py_DECREF(self);
            return NULL;
        }
        init_file_attr(self);
    }
    return (PyObject *)self;
}

static void
_MediaFile_close(inimedia_MediaFileObject *self)
{
    if (self->fp)
        fclose(self->fp);
    cleanup_sidx_box(&self->sidx);
    init_file_attr(self);
}

static void
MediaFile_dealloc(inimedia_MediaFileObject *self)
{
    Py_XDECREF(self->pathname);
    _MediaFile_close(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
_MediaFile_open(inimedia_MediaFileObject *self)
{
    PyObject *bytes;
    char *pathname;
    Py_ssize_t len;
    int nb;

    if (self->pathname == NULL)
        return -1;

    if (self->fp)
        return 0;

    bytes = PyUnicode_AsEncodedString(self->pathname, "utf-8",
                                      "surrogateescape");
    if (bytes == NULL)
        return -1;

    PyBytes_AsStringAndSize(bytes, &pathname, &len);
    if (pathname == NULL || strlen(pathname) == 0) {
        goto error;
    }

    self->fp = fopen(pathname, "r+b");
    Py_DECREF(bytes);
    if (self->fp) {
        nb = read_init_seg(self->fp, &self->iseg);
        if (nb != -1)
            read_sidx_box(self->iseg.end_offs, &self->sidx, self->fp); /* sidx */
    }


    return 0;

error:
    Py_DECREF(bytes);
    return -1;
}

static int
_MediaFile_open_trunk(inimedia_MediaFileObject *self)
{
    PyObject *bytes;
    char *pathname;
    Py_ssize_t len;

    if (self->pathname == NULL)
        return -1;

    bytes = PyUnicode_AsEncodedString(self->pathname, "utf-8",
                                      "surrogateescape");
    if (bytes == NULL)
        return -1;

    PyBytes_AsStringAndSize(bytes, &pathname, &len);
    if (pathname == NULL || strlen(pathname) == 0) {
        goto error;
    }

    _MediaFile_close(self);
    self->fp = fopen(pathname, "w+b");
    Py_DECREF(bytes);
    if (self->fp == NULL)
        return -1;

    return 0;

error:
    Py_DECREF(bytes);
    return -1;
}

static int
MediaFile_init(inimedia_MediaFileObject *self, PyObject *args,
               PyObject *keywords)
{
    PyObject *pathname = NULL, *tmp = NULL;
    /* static char *keywordList[] = {"pathname", NULL}; */
    /* if (! PyArg_ParseTupleAndKeywords(args, keywords, */
    /*                                   "!U", keywordList, &pathname)) */
    /*     return -1; */
    if (! PyArg_ParseTuple(args, "U", &pathname))
        return -1;

    if (pathname) {
        tmp = self->pathname;
        Py_INCREF(pathname);
        self->pathname = pathname;
        Py_XDECREF(tmp);
        _MediaFile_close(self);
    }
    return 0;
}

static PyMemberDef MediaFile_members[] = {
    {"pathname", T_OBJECT_EX, offsetof(inimedia_MediaFileObject, pathname),
     0, "pathname of mediafile"},
    {NULL}
};

static PyObject *
MediaFile_pathname(inimedia_MediaFileObject *self)
{
    if (self->pathname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "pathname");
        return NULL;
    }
    return Py_BuildValue("S", self->pathname);
}

static PyObject *
MediaFile_open(inimedia_MediaFileObject *self)
{
    if (self->pathname == NULL) {
        PyErr_SetString(PyExc_AttributeError, "pathname");
        return NULL;
    }
    if (_MediaFile_open(self) == -1) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }
    return Py_BuildValue("S", self->pathname);
}

static PyObject *
MediaFile_close(inimedia_MediaFileObject *self)
{
    _MediaFile_close(self);
    Py_RETURN_NONE;
}

static PyObject *
MediaFile_has_iseg(inimedia_MediaFileObject *self)
{
    if (_MediaFile_open(self) == -1) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }
    return Py_BuildValue("i", self->sidx.offs > 0);
}

static PyObject *
MediaFile_write_iseg(inimedia_MediaFileObject *self, PyObject *args)
{
    const uint8_t *buf;
    Py_ssize_t len;
    int nb;
    sidx_box_t sidx;
    int max_seg_cnt;

    if (! PyArg_ParseTuple(args, "y#ilL", &buf, &len,
                           &max_seg_cnt, &sidx.tm_scal, &sidx.erly_pres_tm))
        return NULL;

    if (_MediaFile_open_trunk(self) == -1) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }

    nb = write_init_seg(buf, len, self->fp);
    if (nb == -1) {
        PyErr_SetString(PyExc_IOError, "init segment write error");
        return NULL;
    }

    nb = read_init_seg(self->fp, &self->iseg);
    if (nb == -1) {
        PyErr_SetString(PyExc_IOError, "init segment write error");
        return NULL;
    }

    sidx.sz = SIDX_SIZE_MIN;
    sidx.type = BOX_TYPE_SIDX;
    sidx.ver = 1;               /* XXX only support version 1 */
    sidx.flgs = 0;
    sidx.ref_id = 1;
    sidx.frst_offs = SIDX_REF_SIZE * max_seg_cnt;
    sidx.ref_cnt = 0;
    sidx.max_ref_cnt = max_seg_cnt;
    sidx.refs = malloc(sizeof(*sidx.refs) * sidx.max_ref_cnt);
    if (sidx.refs == NULL) {
        PyErr_SetString(PyExc_SystemError, "sidx references alloc error");
        return NULL;
    }

    nb = write_sidx_box(self->iseg.end_offs, &sidx, self->fp);
    if (nb == -1) {
        PyErr_SetString(PyExc_IOError, "sidx box write error");
        goto error;
    }
    self->sidx = sidx;

    if (sidx.frst_offs > 0) {
        if (write_free_box(sidx.offs + sidx.sz, sidx.frst_offs, self->fp) == -1) {
            PyErr_SetString(PyExc_IOError, "free box write error");
            goto error;
        }
    }

    return Py_BuildValue("i", 1);

error:
    free(sidx.refs);
    return NULL;
}

static PyObject *
MediaFile_write_seg(inimedia_MediaFileObject *self, PyObject *args)
{
    const uint8_t *buf;
    Py_ssize_t len;
    int idx;
    long dur;
    int64_t seg_offs;

    if (! PyArg_ParseTuple(args, "y#il", &buf, &len, &idx, &dur))
        return NULL;

    if (_MediaFile_open(self) == -1) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }

    if (idx > self->sidx.max_ref_cnt) {
        PyErr_Format(PyExc_IndexError, "segment index is out of range",
                     self->sidx.ref_cnt, idx);
        return NULL;
    }

    if (idx > self->sidx.ref_cnt) {
        PyErr_Format(PyExc_RuntimeError, "expected index is %d, but %d",
                     self->sidx.ref_cnt, idx);
        return NULL;
    }

    seg_offs = write_seg(&self->iseg, &self->sidx, idx, dur, buf, len,
                         self->fp);
    if (seg_offs == -1) {
        PyErr_SetString(PyExc_IOError, "segment write error");
        return NULL;
    }

    return Py_BuildValue("L", seg_offs);
}

static PyMethodDef MediaFile_methods[] = {
    {"pathname", (PyCFunction)MediaFile_pathname, METH_NOARGS,
     "Return pathname of mediafile"},
    {"open", (PyCFunction)MediaFile_open, METH_NOARGS,
     "open mediafile"},
    {"close", (PyCFunction)MediaFile_close, METH_NOARGS,
     "close mediafile"},
    {"has_iseg", (PyCFunction)MediaFile_has_iseg, METH_NOARGS,
     "TODO"},
    {"write_iseg", (PyCFunction)MediaFile_write_iseg, METH_VARARGS,
     "TODO"},
    {"write_seg", (PyCFunction)MediaFile_write_seg, METH_VARARGS,
     "TODO"},
    {NULL}
};

static PyTypeObject inimedia_MediaFileType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "inimedia.MediaFile",             /* tp_name */
    sizeof(inimedia_MediaFileObject), /* tp_basicsize */
    0,                                /* tp_itemsize */
    (destructor)MediaFile_dealloc,    /* tp_dealloc */
    0,                                /* tp_print */
    0,                                /* tp_getattr */
    0,                                /* tp_setattr */
    0,                                /* tp_reserved */
    0,                                /* tp_repr */
    0,                                /* tp_as_number */
    0,                                /* tp_as_sequence */
    0,                                /* tp_as_mapping */
    0,                                /* tp_as_hash */
    0,                                /* tp_call */
    0,                                /* tp_str */
    0,                                /* tp_getattro */
    0,                                /* tp_setattro */
    0,                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "inimedia mediafile",             /* tp_doc */
    0,                                /* tp_traverse */
    0,                                /* tp_clear */
    0,                                /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    0,                                /* tp_iter */
    0,                                /* tp_iternext */
    MediaFile_methods,                /* tp_methods */
    MediaFile_members,                /* tp_members */
    0,                                /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    (initproc)MediaFile_init,         /* tp_init */
    0,                                /* tp_alloc */
    MediaFile_new,                    /* tp_new */
};

static PyModuleDef inimediamodule = {
    PyModuleDef_HEAD_INIT,
    "inimedia",
    "module for handling multimedia data",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC
PyInit_inimedia(void)
{
    PyObject *m;
    if (PyType_Ready(&inimedia_MediaFileType) < 0)
        return NULL;
    m = PyModule_Create(&inimediamodule);
    if (m == NULL)
        return NULL;
    Py_INCREF(&inimedia_MediaFileType);
    PyModule_AddObject(m, "MediaFile", (PyObject *)&inimedia_MediaFileType);
    return m;
}

/* Local Variables: */
/* compile-command:"cd ..; python3 setup.py build" */
/* End: */
