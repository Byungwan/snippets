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

static int
read_u8(uint8_t *np, FILE *fp)
{
    if (fread(np, sizeof(*np), 1, fp) != 1)
        return -1;
    return 0;
}

static int
write_u8(uint8_t n, FILE *fp)
{
    if (fwrite(&n, sizeof(n), 1, fp) != 1)
        return -1;
    return 0;
}

static int
read_u16(uint16_t *np, FILE *fp)
{
    uint16_t b16;
    if (fread(&b16, sizeof(b16), 1, fp) != 1)
        return -1;
    *np = be16toh(b16);
    return 0;
}

static int
write_u16(uint16_t n, FILE *fp)
{
    uint16_t b16 = htobe16(n);
    if (fwrite(&b16, sizeof(b16), 1, fp) != 1)
        return -1;
    return 0;
}

static int
read_u24(uint32_t *np, FILE *fp)
{
    uint32_t b32;
    if (fread(&b32, sizeof(b32)-1, 1, fp) != 1)
        return -1;
    *np = be32toh((b32 >> 8) & 0x00FFFFFF);
    return 0;
}

static int
write_u24(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    b32 <<= 8;
    if (fwrite(&b32, sizeof(b32)-1, 1, fp) != 1)
        return -1;
    return 0;
}

static int
read_u32(uint32_t *np, FILE *fp)
{
    uint32_t b32;
    if (fread(&b32, sizeof(b32), 1, fp) != 1)
        return -1;
    *np = be32toh(b32);
    return 0;
}

static int
write_u32(uint32_t n, FILE *fp)
{
    uint32_t b32 = htobe32(n);
    if (fwrite(&b32, sizeof(b32), 1, fp) != 1)
        return -1;
    return 0;
}

static int
read_u64(uint64_t *np, FILE *fp)
{
    uint64_t b64;
    if (fread(&b64, sizeof(b64), 1, fp) != 1)
        return -1;
    *np = be64toh(b64);
    return 0;
}

static int
write_u64(uint64_t n, FILE *fp)
{
    uint64_t b64 = htobe64(n);
    if (fwrite(&b64, sizeof(b64), 1, fp) != 1)
        return -1;
    return 0;
}

static int
read_dummy(int32_t len, FILE *fp)
{
    uint64_t u64 = 0;
    uint8_t u8 = 0;
    int32_t i;

    for (i = 0; i < len / 8; i++) {
        if (fread(&u64, sizeof(u64), 1, fp) != 1)
            return -1;
    }
    for (i = 0; i < len % 8; i++) {
        if (fread(&u8, sizeof(u8), 1, fp) != 1)
            return -1;
    }

    return 0;
}

static int
write_zero(int32_t len, FILE *fp)
{
    uint64_t u64 = 0;
    uint8_t u8 = 0;
    int32_t i;

    for (i = 0; i < len / 8; i++) {
        if (fwrite(&u64, sizeof(u64), 1, fp) != 1)
            return -1;
    }
    for (i = 0; i < len % 8; i++) {
        if (fwrite(&u8, sizeof(u8), 1, fp) != 1)
            return -1;
    }

    return 0;
}

static int
read_base_box(base_box_t *box, FILE *fp)
{
    uint32_t sz;

    if (read_u32(&sz, fp) != 0) /* size */
        return -1;
    if (fread(&box->type, sizeof(box->type), 1, fp) != 1) /* type */
        return -1;

    if (sz == 1) {
        if (read_u64(&box->sz, fp)) /* largesize */
            return -1;
    } else if (sz == 0) {
        return -1;              /* box extends to end of file */
    } else {
        box->sz = sz;
    }

    if (box->type == BOX_TYPE_UUID) {
        if (read_dummy(16, fp) != 0)
            return -1;
    }

    return 0;
}

static int
read_full_box(full_box_t *box, FILE *fp)
{
    if (read_base_box((base_box_t *)box, fp) != 0)
        return -1;

    if (read_u8(&box->ver, fp) != 0) /* version */
        return -1;
    if (read_u24(&box->flgs, fp) != 0) /* flags */
        return -1;

    return 0;
}

static off_t
read_init_seg(FILE *fp)
{
    base_box_t box;
    off_t offs = 0;

    rewind(fp);

    if (read_base_box(&box, fp) != 0)
        return -1;
    if (box.type != BOX_TYPE_FTYP)
        return -1;
    offs += box.sz;

    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (read_base_box(&box, fp) != 0)
        return -1;
    if (box.type != BOX_TYPE_MOOV)
        return -1;
    offs += box.sz;

    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (fseeko(fp, 0, SEEK_END) != 0)
        return -1;
    if (ftello(fp) < offs)
        return -1;

    return offs;
}

static off_t
write_init_seg(const uint8_t *buf, int len, FILE *fp)
{
    rewind(fp);
    if (fwrite(buf, 1, len, fp) != len) {
        return -1;
    }
    return len;
}

static off_t
read_sidx_seg(off_t offs, sidx_box_t *sidx, FILE *fp)
{
    uint32_t u32;
    uint16_t dmm16;
    sidx_ref_t *ref;
    int i;

    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (read_full_box((full_box_t *)sidx, fp) != 0)
        return -1;

    if (sidx->type != BOX_TYPE_SIDX)
        return -1;
    if (sidx->ver != 1)           /* XXX only support version 1 */
        return -1;

    if (read_u32(&sidx->ref_id, fp) != 0) /* reference_ID */
        return -1;
    if (read_u32(&sidx->tm_scal, fp) != 0) /* timescale */
        return -1;

    if (read_u64(&sidx->erly_pres_tm, fp) != 0) /* earliest_presentation_time */
        return -1;
    if (read_u64(&sidx->frst_offs, fp)) /* first_offset */
        return -1;
    if (read_u16(&dmm16, fp) != 0) /* reserved */
        return -1;
    if (read_u16(&sidx->ref_cnt, fp) != 0) /* reference_count */
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
        if (read_u32(&u32, fp) != 0)
            goto error;
        ref->ref_type = (u32 >> 31) & 0x00000001; /* reference _type */
        ref->ref_sz = u32 & 0x7FFFFFFF;        /* reference _size */
        if (read_u32(&ref->sseg_dur, fp) != 0) /* subsegment_duration */
            goto error;
        if (read_u32(&u32, fp) != 0) /* XXX skip all about SAP */
            goto error;
        ref->strt_sap = 1;
        ref->sap_type = 0;
        ref->sap_delta_tm = 0;
        ref++;
    }

    return offs + sidx->sz;

error:
    free(sidx->refs);
    return -1;
}

static off_t
write_sidx_seg(off_t offs, sidx_box_t *sidx, FILE *fp)
{
    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (sidx->sz != SIDX_SIZE_MIN + (SIDX_REF_SIZE * sidx->ref_cnt))
        return -1;

    if (write_u32(sidx->sz, fp) != 0) /* size */
        return -1;
    if (fwrite(&sidx->type, sizeof(sidx->type), 1, fp) != 1) /* type */
        return -1;
    if (write_u8(sidx->ver, fp) != 0)   /* version */
        return -1;
    if (write_u24(sidx->flgs, fp) != 0)  /* flags */
        return -1;
    if (write_u32(sidx->ref_id, fp) != 0)  /* reference_ID */
        return -1;
    if (write_u32(sidx->tm_scal, fp) != 0)  /* timescale */
        return -1;
    if (write_u64(sidx->erly_pres_tm, fp) != 0) /* earliest_presentation_time */
        return -1;
    if (write_u64(sidx->frst_offs, fp)) /* first_offset */
        return -1;
    if (write_u16(0, fp) != 0)  /* reserved */
        return -1;
    if (write_u16(sidx->ref_cnt, fp) != 0)  /* reference_count */
        return -1;

    return offs + sidx->sz;
}

static off_t
write_free_box(off_t offs, uint32_t len, FILE *fp)
{
    uint32_t type;

    if (fseeko(fp, offs, SEEK_SET) != 0)
        return -1;

    if (write_u32(len, fp) != 0) /* size */
        return -1;
    type = BOX_TYPE_FREE;
    if (fwrite(&type, sizeof(type), 1, fp) != 1) /* type */
        return -1;

    if (write_zero(len - 8, fp) != 0)
        return -1;

    return offs + len;
}

static off_t
write_seg(off_t offs, sidx_box_t *sidx, int idx, long dur,
          const uint8_t *buf, int len, FILE *fp)
{
    sidx_ref_t *ref = sidx->refs + idx;
    off_t sidx_end, ref_offs, seg_offs;
    int i;
    uint32_t u32;

    if (idx > sidx->max_ref_cnt)
        return -1;
    if (idx > sidx->ref_cnt)
        return -1;

    ref->ref_type = 0;
    ref->ref_sz = len;
    ref->sseg_dur = dur;
    ref->strt_sap = 1;
    ref->sap_type = 0;
    ref->sap_delta_tm = 0;

    ref_offs = offs + SIDX_SIZE_MIN + (SIDX_REF_SIZE * idx);
    if (fseeko(fp, ref_offs, SEEK_SET) != 0)
        return -1;

    /* write sidx ref */
    u32 = (ref->ref_type << 31) + ref->ref_sz;
    if (write_u32(u32, fp) != 0)/* reference _type and reference _size */
        return -1;
    if (write_u32(ref->sseg_dur, fp) != 0)/* subsegment_duration */
        return -1;
    u32 = ((ref->strt_sap << 31) + (ref->sap_type << 28) + ref->sap_delta_tm);
    if (write_u32(u32, fp) != 0) /* SAP */
        return -1;

    /* update sidx */
    sidx->ref_cnt = idx + 1;
    sidx->sz = SIDX_SIZE_MIN + (SIDX_REF_SIZE * sidx->ref_cnt);
    sidx->frst_offs = SIDX_REF_SIZE * (sidx->max_ref_cnt - sidx->ref_cnt);
    sidx_end = write_sidx_seg(offs, sidx, fp);
    if (sidx_end == -1)
        return -1;

    if (sidx->frst_offs > 0) {
        if (write_free_box(sidx_end, sidx->frst_offs, fp) < 0)
            return -1;
    }

    seg_offs = sidx_end + sidx->frst_offs;
    ref = sidx->refs;
    for (i = 0; i < idx; i++) {
        seg_offs += ref->ref_sz;
        ref++;
    }

    /* write segment */
    if (fseeko(fp, seg_offs, SEEK_SET) != 0)
        return -1;
    if (fwrite(buf, 1, len, fp) != len) {
        return -1;
    }

    return sidx_end;
}

typedef struct {
    PyObject_HEAD
    PyObject *pathname;
    FILE *fp;

    off_t sidx_beg;
    off_t sidx_end;
    sidx_box_t sidx;
} inimedia_MediaFileObject;

static void
init_file_attr(inimedia_MediaFileObject *self)
{
    self->fp = NULL;
    self->sidx_beg = -1;
    self->sidx_end = -1;
    memset(&self->sidx, 0, sizeof(self->sidx));
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
    if (self->sidx.refs)
        free(self->sidx.refs);
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
    sidx_box_t sidx;

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
        self->sidx_beg = read_init_seg(self->fp);
        if (self->sidx_beg > 0) {
            self->sidx_end = read_sidx_seg(self->sidx_beg, &sidx, self->fp);
            if (self->sidx_end > 0)
                self->sidx = sidx;
        }
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
    if (_MediaFile_open(self) != 0) {
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
    if (_MediaFile_open(self) != 0) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }
    return Py_BuildValue("i", self->sidx_end > 0);
}

static PyObject *
MediaFile_write_iseg(inimedia_MediaFileObject *self, PyObject *args)
{
    const uint8_t *buf;
    Py_ssize_t len;
    sidx_box_t sidx;
    int max_seg_cnt;

    if (! PyArg_ParseTuple(args, "y#ilL", &buf, &len,
                           &max_seg_cnt, &sidx.tm_scal, &sidx.erly_pres_tm))
        return NULL;

    if (_MediaFile_open_trunk(self) != 0) {
        PyErr_SetString(PyExc_IOError, "file open error");
        return NULL;
    }

    self->sidx_beg = write_init_seg(buf, len, self->fp);
    if (self->sidx_beg < 0) {
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

    self->sidx_end = write_sidx_seg(self->sidx_beg, &sidx, self->fp);
    if (self->sidx_end < 0) {
        PyErr_SetString(PyExc_IOError, "sidx box write error");
        goto error;
    }
    self->sidx = sidx;

    if (sidx.frst_offs > 0) {
        if (write_free_box(self->sidx_end, sidx.frst_offs, self->fp) < 0) {
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

    if (! PyArg_ParseTuple(args, "y#il", &buf, &len, &idx, &dur))
        return NULL;

    if (_MediaFile_open(self) != 0) {
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

    self->sidx_end = write_seg(self->sidx_beg, &self->sidx,
                               idx, dur, buf, len, self->fp);
    if (self->sidx_end < 0) {
        PyErr_SetString(PyExc_IOError, "segment write error");
        return NULL;
    }

    return Py_BuildValue("i", 1);
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
    PyObject_HEAD_INIT(NULL)
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
