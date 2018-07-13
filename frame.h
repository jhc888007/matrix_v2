#ifndef _FRAME_H_
#define _FRAME_H_

#include <sys/types.h>
#include <vector>
#include <stdio.h>

using namespace std;

enum FrameType{
    FrameTypeNone,
    FrameTypeIF,
    FrameTypeTI,
    FrameTypeTL,
    FrameTypeString,
    FrameTypeU32,
    FrameTypeU64,
    FrameTypeNum
};

struct FrameIndexHeader{
    uint64_t off_max:52;
    uint64_t type:12;
    FrameIndexHeader() {
        off_max = 0;
        type = 0;
    }
    FrameIndexHeader(uint64_t off_max_, uint32_t type_) {
        off_max = off_max_;
        type = type_;
    }
};

struct FrameIndexNormalBody{
    uint64_t offset:40;
    uint64_t count:24;
    FrameIndexNormalBody() {
        offset = 0;
        count = 0;
    }
    inline void Clear() {
        offset = 0;
        count = 0;
    }
    void GetData(uint64_t &offset_, uint64_t &count_) {
        offset_ = offset;
        count_ = count;
    }
};

struct FrameDataHeader{
    uint64_t off_max:52;
    uint64_t type:12;
    FrameDataHeader() {
        off_max = 0;
        type = 0;
    }
    FrameDataHeader(uint64_t off_max_, uint32_t type_) {
        off_max = off_max_;
        type = type_;
    }
};

struct FrameDataIFBody{
    uint32_t rid;
    float value;
    FrameDataIFBody() {
        rid = 0;
        value = 0;
    }
    static bool frame_comp_func(const FrameDataIFBody &m1, const FrameDataIFBody &m2) {
        return m1.value > m2.value;
    }
    static vector<FrameDataIFBody> &ParseString(vector<FrameDataIFBody> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        FrameDataIFBody body;
        long rid;
        float value;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, ':');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            rid = atol(start);
            start = end + 1;
            end = strchr(start, '|');
            if (NULL == end) {
                value = atof(start);
                start = end + 1;

                if (rid >= 0) {
                    body.rid = rid;
                    body.value = value;
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            value = atof(start);
            start = end + 1;

            if (rid >= 0) {
                body.rid = rid;
                body.value = value;
                vec.push_back(body);
            }
        }
        if (vec.size() == 0) {
            return vec;
        }
        sort(vec.begin(), vec.end(), FrameDataIFBody::frame_comp_func);
        return vec;
    }
    static void Print(FrameDataIFBody &body) {
        cout << "id:" << body.rid << " score:" << body.value << endl;
    }
    static PyObject *GetPyObject(FrameDataIFBody &body) {
        PyObject *dict = PyDict_New();
        PyObject *rid = Py_BuildValue("i", body.rid);
        PyMapping_SetItemString(dict, "id", rid);
        PyObject *value = Py_BuildValue("i", int(body.value));
        PyMapping_SetItemString(dict, "score", value);
        Py_DECREF(rid);
        Py_DECREF(value);
        return dict;
    }
    static PyObject *GetPyObjectSimple(FrameDataIFBody &body) {
        PyObject *rid = Py_BuildValue("i", body.rid);
        return rid;
    }
    static PyObject *GetPyObject(vector<FrameDataIFBody> &vec) {
        PyObject *dict = PyDict_New();
        for (vector<FrameDataIFBody>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            PyObject *rid = Py_BuildValue("i", iter->rid);
            PyObject *value = Py_BuildValue("i", int(iter->value));
            PyDict_SetItem(dict, rid, value);
            Py_DECREF(rid);
            Py_DECREF(value);
        }
        return dict;
    }
};

struct FrameDataTIBody{
    uint32_t type:4;
    uint32_t rid:28;
    FrameDataTIBody() {
        type = 0;
        rid = 0;
    }
    static vector<FrameDataTIBody> &ParseString(vector<FrameDataTIBody> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        FrameDataTIBody body;
        int type, rid;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, ':');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            type = atoi(start);
            start = end + 1;
            end = strchr(start, '|');
            if (NULL == end) {
                rid = atoi(start);
                start = end + 1;

                if (type >= 0 && rid >= 0) {
                    body.type = type;
                    body.rid = rid;
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            rid = atoi(start);
            start = end + 1;

            if (type >= 0 && rid >= 0) {
                body.type = type;
                body.rid = rid;
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(FrameDataTIBody &body) {
        cout << "type:" << body.type << " id:" << body.rid << endl;
    }
    static PyObject *GetPyObject(FrameDataTIBody &body) {
        PyObject *dict = PyDict_New();
        PyObject *type = Py_BuildValue("i", body.type);
        PyMapping_SetItemString(dict, "type", type);
        PyObject *rid = Py_BuildValue("i", body.rid);
        PyMapping_SetItemString(dict, "id", rid);
        Py_DECREF(rid);
        Py_DECREF(type);
        return dict;
    }
    static PyObject *GetPyObjectSimple(FrameDataTIBody &body) {
        PyObject *rid = Py_BuildValue("i", body.rid);
        return rid;
    }
    static PyObject *GetPyObject(vector<FrameDataTIBody> &vec) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, str);
        return list;
    }
};

struct FrameDataTLBody{
    uint64_t type:4;
    uint64_t rid:60;
    FrameDataTLBody() {
        type = 0;
        rid = 0;
    }
    static vector<FrameDataTLBody> &ParseString(vector<FrameDataTLBody> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        FrameDataTLBody body;
        long type, rid;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, ':');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            type = atol(start);
            start = end + 1;
            end = strchr(start, '|');
            if (NULL == end) {
                rid = atol(start);
                start = end + 1;

                if (type >= 0 && rid >= 0) {
                    body.type = type;
                    body.rid = rid;
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            rid = atol(start);
            start = end + 1;

            if (type >= 0 && rid >= 0) {
                body.type = type;
                body.rid = rid;
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(FrameDataTLBody &body) {
        cout << "type:" << body.type << " id:" << body.rid << endl;
    }
    static PyObject *GetPyObject(FrameDataTLBody &body) {
        PyObject *dict = PyDict_New();
        PyObject *type = Py_BuildValue("l", body.type);
        PyMapping_SetItemString(dict, "type", type);
        PyObject *rid = Py_BuildValue("l", body.rid);
        PyMapping_SetItemString(dict, "id", rid);
        Py_DECREF(rid);
        Py_DECREF(type);
        return dict;
    }
    static PyObject *GetPyObjectSimple(FrameDataTLBody &body) {
        PyObject *rid = Py_BuildValue("l", body.rid);
        return rid;
    }
    static PyObject *GetPyObject(vector<FrameDataTLBody> &vec) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, str);
        return list;
    }
};

struct FrameDataStringBody{
    uint32_t str;
    FrameDataStringBody() {
        str = 0;
    }
    static vector<FrameDataStringBody> &ParseString(vector<FrameDataStringBody> &vec, 
        char *data, int len) {
        vec.resize(0);
        int length = len/4+1;
        vec.resize(length);
        *(data+len) = '\0';
        memcpy(&vec[0], data, len+1);
        return vec;
    }
    static void Print(FrameDataStringBody &body) {
        cout << "Null Call" << endl;
    }
    static PyObject *GetPyObject(FrameDataStringBody &body) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        return str;
    }
    static PyObject *GetPyObjectSimple(FrameDataStringBody &body) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        return str;
    }
    static PyObject *GetPyObject(vector<FrameDataStringBody> &vec) {
        PyObject *str = NULL:
        if vec.size() == 0 {
            str = Py_BuildValue("s", "");
        } else {
            str = Py_BuildValue("s", (char *)&vec[0]);
        }
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, str);
        return list;
    }
};

struct FrameDataU32Body{
    uint32_t rid;
    FrameDataU32Body() {
        rid = 0;
    }
    static vector<FrameDataU32Body> &ParseString(vector<FrameDataU32Body> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        FrameDataU32Body body;
        int rid;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, '|');
            if (NULL == end) {
                rid = atoi(start);
                start = end + 1;

                if (rid >= 0) {
                    body.rid = (uint32_t)rid;
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            rid = atoi(start);
            start = end + 1;

            if (rid >= 0) {
                body.rid = (uint32_t)rid;
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(FrameDataU32Body &body) {
        cout << " id:" << body.rid << endl;
    }
    static PyObject *GetPyObject(FrameDataU32Body &body) {
        PyObject *rid = Py_BuildValue("i", body.rid);
        return rid;
    }
    static PyObject *GetPyObjectSimple(FrameDataU32Body &body) {
        PyObject *rid = Py_BuildValue("i", body.rid);
        return rid;
    }
    static PyObject *GetPyObject(vector<FrameDataU32Body> &vec) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, str);
        return list;
    }
};

struct FrameDataU64Body{
    uint64_t rid;
    FrameDataU64Body() {
        rid = 0;
    }
    static vector<FrameDataU64Body> &ParseString(vector<FrameDataU64Body> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        FrameDataU64Body body;
        long rid;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, '|');
            if (NULL == end) {
                rid = atol(start);
                start = end + 1;

                if (rid >= 0) {
                    body.rid = (uint64_t)rid;
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            rid = atol(start);
            start = end + 1;

            if (rid >= 0) {
                body.rid = (uint64_t)rid;
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(FrameDataU64Body &body) {
        cout << " id:" << body.rid << endl;
    }
    static PyObject *GetPyObject(FrameDataU64Body &body) {
        PyObject *rid = Py_BuildValue("l", body.rid);
        return rid;
    }
    static PyObject *GetPyObjectSimple(FrameDataU64Body &body) {
        PyObject *rid = Py_BuildValue("l", body.rid);
        return rid;
    }
    static PyObject *GetPyObject(vector<FrameDataU64Body> &vec) {
        PyObject *str = Py_BuildValue("s", "Null Call");
        PyObject *list = PyList_New(1);
        PyList_SET_ITEM(list, 0, str);
        return list;
    }
};



#endif

