#ifndef _SERIES_READER_H_
#define _SERIES_READER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include <vector>
#include <Python.h>

using namespace std;

template<typename HeaderT>
class SeriesHeaderReader {
public:
    HeaderT GetHeader(const char *data_file) {
        int data_fp = open(data_file, O_RDONLY);
        HeaderT header;
        if (data_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open series header fail\n");
            return header;
        }
        if (pread(data_fp, (void *)&header, sizeof(HeaderT), 0) < 0) {
            printf("!!!matrixbuilder lib error!!! read series header fail\n");
        }
        close(data_fp);
        return header;
    }
};

template<typename HeaderT, typename IndexT, typename BodyT>
class SeriesBlockReader {
public:
    SeriesBlockReader() {
        _data_fp = -1;
        _buffer = NULL;
    }
    ~SeriesBlockReader() {
        if (_buffer) {
            free(_buffer);
            _buffer = NULL;
        }
    }
    void Open(const char *data_file, uint32_t off_max, uint32_t block_cap) {
        _data_fp = open(data_file, O_RDONLY);
        if (_data_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open series data fail\n");
            return ;
        }
        _off_max = off_max;
        _block_cap = block_cap;
        if (_block_cap > IDX_LIMIT) {
            _idx_flag = true;
            _block_len = sizeof(IndexT)+sizeof(BodyT)*_block_cap;
        } else {
            _idx_flag = false;
            _block_len = sizeof(BodyT)*_block_cap;
        }
        _buffer = (char *)malloc(_block_len);
        _vec.reserve(_block_cap);
    }
    void Close() {
        if (_data_fp >= 0) {
            if (close(_data_fp) < 0) {
                printf("!!!matrixbuilder lib error!!! close series data fail\n");
            }
        }
        if (_buffer) {
            free(_buffer);
            _buffer = NULL;
        }
    }
    vector<BodyT> *GetData(uint32_t off, uint32_t size) {
        _vec.resize(0);
        if (size <= 0 || off < 0 || off > _off_max || _data_fp < 0) {
            return &_vec;
        }
        if (size > _block_cap) {
            size = _block_cap;
        }

        if (_idx_flag) {
            uint32_t readlen = sizeof(IndexT) + sizeof(BodyT) * size;
            uint32_t offlen = sizeof(HeaderT) + off * _block_len;
            if (_buffer == NULL) {
                return &_vec;
            }
            if (pread(_data_fp, (void *)_buffer, readlen, offlen) < 0) {
                return &_vec;
            }
            IndexT *index = (IndexT *)_buffer;
            if (index->size == 0) {
                return &_vec;
            }
            if (index->size < size) {
                size = index->size;
            }
            _vec.resize(size);
            memcpy((void *)&_vec[0], _buffer + sizeof(IndexT), sizeof(BodyT) * size);
        } else {
            uint32_t readlen = sizeof(BodyT) * size;
            uint32_t offlen = sizeof(HeaderT) + off * _block_len;
            _vec.resize(size);
            if (pread(_data_fp, (void *)&_vec[0], readlen, offlen) < 0) {
                _vec.resize(0);
                return &_vec;
            }
            if (sizeof(BodyT) <= 4) {
                for (uint32_t i = 0; i < size; i++) {
                    if ( *(uint32_t *)&_vec[i] == 0 ) {
                        size = i;
                        break;
                    }
                }
            } else {
                for (uint32_t i = 0; i < size; i++) {
                    if ( *(uint64_t *)&_vec[i] == 0 ) {
                        size = i;
                        break;
                    }
                }
            }
            _vec.resize(size);
        }
        return &_vec;
    }
    PyObject *GetPyObject(vector<BodyT> &vec) {
        int count = 0,length = vec.size();
        if (length == 0) {
            return PyList_New(0);
        }
        PyObject *list = PyList_New(length);
        if (list == NULL) {
            return PyList_New(0);
        }
        for (typename vector<BodyT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            PyObject *obj = BodyT::GetPyObject(*iter);
            if (obj != NULL) {
                PyList_SET_ITEM(list, count, obj);
            } else {
                PyList_SET_ITEM(list, count, Py_None);
            }
            count++;
        }
        for (count = 0;count < length;count++) {
            PyObject *item = PyList_GET_ITEM(list, count);
            if (item == NULL) {
                PyList_SET_ITEM(list, count, Py_None);
            }
        }
        return list;
    }
    uint64_t GetSize() {
        return sizeof(HeaderT) + (_off_max+1) * _block_len;
    }
    void Print(vector<BodyT> &vec) {
        for (typename vector<BodyT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            BodyT::Print(*iter);
        }
    }
private:
    uint32_t _off_max;
    uint32_t _block_cap;
    uint32_t _block_len;
    bool _idx_flag;
    int _data_fp;
    char *_buffer;
    vector<BodyT> _vec;
};

class SeriesReaderManager {
public:
    SeriesReaderManager() {
        _header_reader = new SeriesHeaderReader<SeriesHeader>();

        _int_reader = new SeriesBlockReader<SeriesHeader, SeriesIntIndex, SeriesIntBody>;
        _float_reader = new SeriesBlockReader<SeriesHeader, SeriesFloatIndex, SeriesFloatBody>;
        _score_reader = new SeriesBlockReader<SeriesHeader, SeriesScoreIndex, SeriesScoreBody>;

        _type = SeriesTypeNone;
    }
    ~SeriesReaderManager() {
        _int_reader->Close();
        _float_reader->Close();
        _score_reader->Close();

        delete _header_reader;
        delete _int_reader;
        delete _float_reader;
        delete _score_reader;
    }
    void Init(char *string1) {
        SeriesHeader header = _header_reader->GetHeader(string1);
        if (header.off_max == 0 || header.type == (uint32_t)SeriesTypeNone) {
            return ;
        }
        if (header.type == (uint32_t)SeriesTypeInt) {
            _type = SeriesTypeInt;
            _int_reader->Open(string1, header.off_max, header.block_cap);
        } else if (header.type == (uint32_t)SeriesTypeFloat) {
            _type = SeriesTypeFloat;
            _float_reader->Open(string1, header.off_max, header.block_cap);
        } else if (header.type == (uint32_t)SeriesTypeScore) {
            _type = SeriesTypeScore;
            _score_reader->Open(string1, header.off_max, header.block_cap);
        }
        return ;
    }
    PyObject *Read(int uid, int len) {
        if (_type == SeriesTypeInt) {
            vector<SeriesIntBody> *vec = _int_reader->GetData(uid, len);
            return _int_reader->GetPyObject(*vec);
        } else if (_type == SeriesTypeFloat) {
            vector<SeriesFloatBody> *vec = _float_reader->GetData(uid, len);
            return _float_reader->GetPyObject(*vec);
        } else if (_type == SeriesTypeScore) {
            vector<SeriesScoreBody> *vec = _score_reader->GetData(uid, len);
            return _score_reader->GetPyObject(*vec);
        }
        return PyList_New(0);
    }
    uint64_t Size() {
        if (_type == SeriesTypeInt) {
            return _int_reader->GetSize();
        } else if (_type == SeriesTypeFloat) {
            return _float_reader->GetSize();
        } else if (_type == SeriesTypeScore) {
            return _score_reader->GetSize();
        }
        return 0;
    }
private:
    SeriesHeaderReader<SeriesHeader> *_header_reader;
    SeriesBlockReader<SeriesHeader, SeriesIntIndex, SeriesIntBody> *_int_reader;
    SeriesBlockReader<SeriesHeader, SeriesFloatIndex, SeriesFloatBody> *_float_reader;
    SeriesBlockReader<SeriesHeader, SeriesScoreIndex, SeriesScoreBody> *_score_reader;
    SeriesType _type;
};

#endif
