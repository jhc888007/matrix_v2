#include <Python.h> //有的是#include<Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>  
#include <vector>
#include <algorithm>
#include <iostream>
#include "framewriter.h"
#include "framereader.h"
#include "serieswriter.h"
#include "seriesreader.h"

using namespace std;

typedef struct{
    PyObject_HEAD
    FrameWriterManager *manager;
}frame_writer;
 
static PyObject *frame_writer_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    frame_writer *m;
    m = (frame_writer *)type->tp_alloc(type, 0);
    m->manager = new FrameWriterManager();
    return (PyObject *)m;
}

static void frame_writer_delete(frame_writer *self) {
    delete self->manager;
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int frame_writer_traverse(frame_writer *self, visitproc visit, void *arg) {
    return 0;
}

static int frame_writer_clear(frame_writer *self) {
    return 0;
}

static int frame_writer_init(frame_writer *self, PyObject *args) {
    char *string1;
    char *string2;
    char *string3;
    int max;
    if (!PyArg_ParseTuple(args, "sssi", &string1, &string2, &string3, &max)) {
        return -1;
    }
    self->manager->Init(string1, string2, string3, max);
    return 0;
}

static PyObject *frame_writer_append(frame_writer *self, PyObject *args) {
    long id;
    int len;
    char *value;
    if (!PyArg_ParseTuple(args, "lsi", &id, &value, &len)) {
        return Py_BuildValue("i", -1);
    }
    self->manager->Write(id, value, len);
    return Py_BuildValue("i", 0);
}

static PyMethodDef frame_writer_methods[] = {
    {"append", (PyCFunction)frame_writer_append, METH_VARARGS,"append to frame file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject frame_writer_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder6.frame_writer",    /* tp_name */
    sizeof(frame_writer),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)frame_writer_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "frame_writer builder v1.0",     /* tp_doc */
    (traverseproc)frame_writer_traverse,/* tp_traverse */
    (inquiry)frame_writer_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    frame_writer_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)frame_writer_init,     /* tp_init */
    0,                         /* tp_alloc */
    frame_writer_new,                /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

typedef struct{
    PyObject_HEAD
    FrameReaderManager *manager;
}frame_reader;
 
static PyObject *frame_reader_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    frame_reader *m;
    m = (frame_reader *)type->tp_alloc(type, 0);
    m->manager = new FrameReaderManager();
    return (PyObject *)m;
}

static void frame_reader_delete(frame_reader *self) {
    delete self->manager;
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int frame_reader_traverse(frame_reader *self, visitproc visit, void *arg) {
    return 0;
}

static int frame_reader_clear(frame_reader *self) {
    return 0;
}

static int frame_reader_init(frame_reader *self, PyObject *args) {
    char *string1, *string2;
    int return_size_max = 1024;
    if (!PyArg_ParseTuple(args, "ss|i", &string1, &string2, &return_size_max)) {
        return -1;
    }
    self->manager->Init(string1, string2, return_size_max);
    return 0;
}

static PyObject *frame_reader_get(frame_reader *self, PyObject *args) {
    long uid;
    int len = 1;
    if (!PyArg_ParseTuple(args, "l|i", &uid, &len)) {
        return PyList_New(0);
    }
    if (uid < 0 || len <= 0) {
        return PyList_New(0);
    }
    return self->manager->Read((uint32_t)uid, (uint32_t)len);
}

static PyObject *frame_reader_dict(frame_reader *self, PyObject *args) {
    long uid;
    int len = 1;
    if (!PyArg_ParseTuple(args, "l|i", &uid, &len)) {
        return PyList_New(0);
    }
    if (uid < 0 || len <= 0) {
        return PyList_New(0);
    }
    return self->manager->ReadVec((uint32_t)uid, (uint32_t)len);
}

static PyObject *frame_reader_list(frame_reader *self, PyObject *args) {
    long uid;
    int len = 1;
    if (!PyArg_ParseTuple(args, "l|i", &uid, &len)) {
        return PyList_New(0);
    }
    if (uid < 0 || len <= 0) {
        return PyList_New(0);
    }
    return self->manager->ReadSimple((uint32_t)uid, (uint32_t)len);
}

static PyObject *frame_reader_header(frame_reader *self, PyObject *args) {
    PyObject *list = PyList_New(2);

    uint64_t index_size, data_size;
    self->manager->GetSize(index_size, data_size);

    PyObject *p_index_size = Py_BuildValue("l", index_size);
    PyList_SET_ITEM(list, 0, p_index_size);
    PyObject *p_data_size = Py_BuildValue("l", data_size);
    PyList_SET_ITEM(list, 1, p_data_size);

    return list;
}

static PyMethodDef frame_reader_methods[] = {
    {"get", (PyCFunction)frame_reader_get, METH_VARARGS,"read idx and data, return id and score"},
    {"header", (PyCFunction)frame_reader_header, METH_VARARGS,"return idx size and data size"},
    {"lst", (PyCFunction)frame_reader_list, METH_VARARGS,"read idx and data, return id"},
    {"dic", (PyCFunction)frame_reader_dict, METH_VARARGS,"read idx and data, return id and score"},
    {NULL}  /* Sentinel */
};

static PyTypeObject frame_reader_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder6.frame_reader",    /* tp_name */
    sizeof(frame_reader),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)frame_reader_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "frame_reader builder v1.0",     /* tp_doc */
    (traverseproc)frame_reader_traverse,/* tp_traverse */
    (inquiry)frame_reader_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    frame_reader_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)frame_reader_init,     /* tp_init */
    0,                         /* tp_alloc */
    frame_reader_new,                /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

typedef struct{
    PyObject_HEAD
    SeriesWriterManager *manager;
}series_writer;
 
static PyObject *series_writer_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    series_writer *m;
    m = (series_writer *)type->tp_alloc(type, 0);
    m->manager = new SeriesWriterManager();
    return (PyObject *)m;
}

static void series_writer_delete(series_writer *self) {
    delete self->manager;
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int series_writer_traverse(series_writer *self, visitproc visit, void *arg) {
    return 0;
}

static int series_writer_clear(series_writer *self) {
    return 0;
}

static int series_writer_init(series_writer *self, PyObject *args) {
    char *string1;
    char *string2;
    int max_id;
    int block_cap = 1;
    if (!PyArg_ParseTuple(args, "ssi|i", &string1, &string2, &max_id, &block_cap)) {
        return -1;
    }
    self->manager->Init(string1, string2, block_cap, max_id);
    return 0;
}

static PyObject *series_writer_append(series_writer *self, PyObject *args) {
    int id;
    int len;
    char *value;
    if (!PyArg_ParseTuple(args, "isi", &id, &value, &len)) {
        return Py_BuildValue("i", -1);
    }
    self->manager->Write(id, value, len);
    return Py_BuildValue("i", 0);
}

static PyMethodDef series_writer_methods[] = {
    {"append", (PyCFunction)series_writer_append, METH_VARARGS,"append to series file"},
    {NULL}  /* Sentinel */
};

static PyTypeObject series_writer_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder6.series_writer",    /* tp_name */
    sizeof(series_writer),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)series_writer_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "series_writer builder v1.0",     /* tp_doc */
    (traverseproc)series_writer_traverse,/* tp_traverse */
    (inquiry)series_writer_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    series_writer_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)series_writer_init,     /* tp_init */
    0,                         /* tp_alloc */
    series_writer_new,                /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

typedef struct{
    PyObject_HEAD
    SeriesReaderManager *manager;
}series_reader;
 
static PyObject *series_reader_new(PyTypeObject *type, PyObject *self, PyObject *args) {
    series_reader *m;
    m = (series_reader *)type->tp_alloc(type, 0);
    m->manager = new SeriesReaderManager();
    return (PyObject *)m;
}

static void series_reader_delete(series_reader *self) {
    delete self->manager;
    PyObject_GC_UnTrack(self);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static int series_reader_traverse(series_reader *self, visitproc visit, void *arg) {
    return 0;
}

static int series_reader_clear(series_reader *self) {
    return 0;
}

static int series_reader_init(series_reader *self, PyObject *args) {
    char *string1;
    if (!PyArg_ParseTuple(args, "s", &string1)) {
        return -1;
    }
    self->manager->Init(string1);
    return 0;
}

static PyObject *series_reader_get(series_reader *self, PyObject *args) {
    int uid, len = 1;
    if (!PyArg_ParseTuple(args, "i|i", &uid, &len)) {
        return PyList_New(0);
    }
    if (uid < 0 || len <= 0) {
        return PyList_New(0);
    }
    return self->manager->Read(uid, len);
}

static PyObject *series_reader_header(series_reader *self, PyObject *args) {
    uint64_t size = self->manager->Size();
    return Py_BuildValue("l", size);
}

static PyMethodDef series_reader_methods[] = {
    {"get", (PyCFunction)series_reader_get, METH_VARARGS,"read idx and data, return id and score"},
    {"header", (PyCFunction)series_reader_header, METH_VARARGS,"return idx size and data size"},
    {NULL}  /* Sentinel */
};

static PyTypeObject series_reader_obj = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "matrixbuilder6.series_reader",    /* tp_name */
    sizeof(series_reader),     /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)series_reader_delete, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,    /* tp_flags */
    "series_reader builder v1.0",     /* tp_doc */
    (traverseproc)series_reader_traverse,/* tp_traverse */
    (inquiry)series_reader_clear,     /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    series_reader_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)series_reader_init,     /* tp_init */
    0,                         /* tp_alloc */
    series_reader_new,                /* tp_new */
    PyObject_GC_Del,                    /* tp_free */
};

static PyMethodDef module_methods[] = {
    {NULL}
};

#if PY_MAJOR_VERSION >= 3
static int matrixbuilder6_traverse(PyObject *m, visitproc visit, void *arg) {
    return 0;
}

static int matrixbuilder6_clear(PyObject *m) {
    return 0;
}

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "matrixbuilder6",
    NULL,
    0,
    module_methods,
    NULL,
    matrixbuilder6_traverse,
    matrixbuilder6_clear,
    NULL
};

PyMODINIT_FUNC PyInit_matrixbuilder6(void) {

#else

PyMODINIT_FUNC initmatrixbuilder6(void) {

#endif
    PyObject *m = NULL;
    
    if (PyType_Ready(&frame_writer_obj) < 0)
        goto END;
    if (PyType_Ready(&frame_reader_obj) < 0)
        goto END;
    if (PyType_Ready(&series_writer_obj) < 0)
        goto END;
    if (PyType_Ready(&series_reader_obj) < 0)
        goto END;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3("matrixbuilder6", module_methods, "matrix builder module v1.0");
#endif

    if (m == NULL)
        goto END;

    Py_INCREF(&frame_writer_obj);
    PyModule_AddObject(m, "frame_writer", (PyObject *)&frame_writer_obj);
    Py_INCREF(&frame_reader_obj);
    PyModule_AddObject(m, "frame_reader", (PyObject *)&frame_reader_obj);
    Py_INCREF(&series_writer_obj);
    PyModule_AddObject(m, "series_writer", (PyObject *)&series_writer_obj);
    Py_INCREF(&series_reader_obj);
    PyModule_AddObject(m, "series_reader", (PyObject *)&series_reader_obj);

END:
#if PY_MAJOR_VERSION >= 3
    return m;
#else
    return;
#endif
}
