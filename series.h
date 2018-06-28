#ifndef _SERIES_H_
#define _SERIES_H_

#include <sys/types.h>
#include <algorithm>
#include <vector>
#include <Python.h>

#define IDX_LIMIT 3

using namespace std;

struct SeriesHeader{
    uint32_t off_max;
    uint32_t block_cap:20;
    uint32_t type:12;
    SeriesHeader() {
        off_max = 0;
        block_cap = 0;
        type = 0;
    }
};

enum SeriesType{
    SeriesTypeNone,
    SeriesTypeInt,
    SeriesTypeFloat,
    SeriesTypeScore,
    SeriesTypeNum
};

struct SeriesIntIndex{
    uint32_t size;
    SeriesIntIndex() {
        size = 0;
    }
};

struct SeriesIntBody{
    uint32_t rid;
    SeriesIntBody() {
        rid = 0;
    }
    static vector<SeriesIntBody> &ParseString(vector<SeriesIntBody> &vec, char *data, int len) {
        char *start = data;
        char *end;
        SeriesIntBody body;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, '|');
            if (NULL == end) {
                body.rid = atoi(start);
                start = end + 1;
                if (body.rid != 0) {
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            body.rid = atoi(start);
            start = end + 1;
            if (body.rid != 0) {
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(SeriesIntBody &body) {
        cout << "id:" << body.rid << endl;
    }
    static PyObject *GetPyObject(SeriesIntBody &body) {
        PyObject *rid = Py_BuildValue("i", body.rid);
        return rid;
    }
};

struct SeriesFloatIndex{
    uint32_t size;
    SeriesFloatIndex() {
        size = 0;
    }
};

struct SeriesFloatBody{
    float value;
    SeriesFloatBody() {
        value = 0;
    }
    static vector<SeriesFloatBody> &ParseString(vector<SeriesFloatBody> &vec,
        char *data, int len) {
        char *start = data;
        char *end;
        SeriesFloatBody body;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, '|');
            if (NULL == end) {
                body.value = atof(start);
                start = end + 1;
                if (body.value > 0.0000001 || body.value < 0.0000001) {
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            body.value = atof(start);
            start = end + 1;
            if (body.value > 0.0000001 || body.value < 0.0000001) {
                vec.push_back(body);
            }
        }
        return vec;
    }
    static void Print(SeriesFloatBody &body) {
        cout << " score:" << body.value << endl;
    }
    static PyObject *GetPyObject(SeriesFloatBody &body) {
        PyObject *value = Py_BuildValue("i", int(body.value));
        return value;
    }
};

struct SeriesScoreIndex{
    uint64_t size;
    SeriesScoreIndex() {
        size = 0;
    }
};

struct SeriesScoreBody{
    uint32_t rid;
    float value;
    SeriesScoreBody() {
        rid = 0;
        value = 0;
    }
    static bool series_comp_func(const SeriesScoreBody &m1, const SeriesScoreBody &m2) {
        return m1.value > m2.value;
    }
    static vector<SeriesScoreBody> &ParseString(vector<SeriesScoreBody> &vec, 
        char *data, int len) {
        char *start = data;
        char *end;
        SeriesScoreBody body;
        vec.resize(0);
        while (start < data + len) {
            end = strchr(start, ':');
            if (NULL == end) {
                break;
            }
            *end = '\0';
            body.rid = atoi(start);
            start = end + 1;
            end = strchr(start, '|');
            if (NULL == end) {
                body.value = atof(start);
                start = end + 1;
                if (body.rid != 0) {
                    vec.push_back(body);
                }
                break;
            }
            *end = '\0';
            body.value = atof(start);
            start = end + 1;
            if (body.rid != 0) {
                vec.push_back(body);
            }
        }
        if (vec.size() == 0) {
            return vec;
        }
        sort(vec.begin(), vec.end(), SeriesScoreBody::series_comp_func);
        return vec;
    }
    static void Print(SeriesScoreBody &body) {
        cout << "id:" << body.rid << " score:" << body.value << endl;
    }
    static PyObject *GetPyObject(SeriesScoreBody &body) {
        PyObject *dict = PyDict_New();
        PyObject *rid = Py_BuildValue("i", body.rid);
        PyMapping_SetItemString(dict, "id", rid);
        PyObject *value = Py_BuildValue("i", int(body.value));
        PyMapping_SetItemString(dict, "score", value);
        Py_DECREF(rid);
        Py_DECREF(value);
        return dict;
    }
};


#endif
