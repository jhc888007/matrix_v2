#ifndef _FRAME_READER_H_
#define _FRAME_READER_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include <vector>
#include "frame.h"

using namespace std;

template<typename HeaderT>
class FrameHeaderReader {
public:
    HeaderT GetHeader(const char *file) {
        int fp = open(file, O_RDONLY);
        HeaderT header;
        if (fp < 0) {
            printf("!!!matrixbuilder lib error!!! open frame header fail\n");
            return header;
        }
        if (pread(fp, (void *)&header, sizeof(HeaderT), 0) < 0) {
            printf("!!!matrixbuilder lib error!!! read frame header fail\n");
        }
        close(fp);
        return header;
    }
};

template<typename IndexHeaderT, typename IndexT, typename DataHeaderT, typename DataT>
class FrameReader {
public:
    FrameReader() {
        _off_max = 0;
        _size_max = 0;
        _idx_fp = -1;
        _data_fp = -1;
    }
    void Open(const char *idx_file, const char *data_file, uint32_t return_size_max,
        uint64_t off_max, uint64_t size_max) {
        _idx_fp = open(idx_file, O_RDONLY);
        if (_idx_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open frame idx fail\n");
            return ;
        }
        _data_fp = open(data_file, O_RDONLY);
        if (_data_fp < 0) {
            printf("!!!matrixbuilder lib error!!! open frame data fail\n");
            return ;
        }
        _off_max = off_max;
        _size_max = size_max;
        _return_size_max = return_size_max;
        _vec.reserve(_return_size_max);
    }
    void Close() {
        if (_idx_fp > 0) {
            if (close(_idx_fp) < 0) {
                printf("!!!matrixbuilder lib error!!! close frame idx fail\n");
            }
            _idx_fp = -1;
        }
        if (_data_fp > 0) {
            if (close(_data_fp) < 0) {
                printf("!!!matrixbuilder lib error!!! close frame data fail\n");
            }
            _data_fp = -1;
        }
    }
    IndexT GetIndex(uint64_t uid) {
        IndexT idx;
        if (uid > _off_max || _idx_fp < 0) {
            return idx;
        }
        if (pread(_idx_fp, (void *)&idx, sizeof(IndexT), 
            sizeof(IndexHeaderT)+uid*sizeof(IndexT)) < 0) {
            idx.Clear();
        }
        return idx;
    }
    vector<DataT> *GetData(uint64_t uid, uint32_t len) {
        _vec.resize(0);

        if (uid > _off_max || _idx_fp < 0) {
            return &_vec;
        }

        IndexT idx;
        if (pread(_idx_fp, (void *)&idx, sizeof(IndexT), 
            sizeof(IndexHeaderT)+uid*sizeof(IndexT)) < 0) {
            idx.Clear();
        }

        uint64_t offset, size;
        idx.GetData(offset, size);
        if (offset + size > _size_max || size == 0 || _data_fp < 0) {
            return &_vec;
        }
        if (size > len) {
            size = len;
        }
        if (size > _return_size_max) {
            size = _return_size_max;
        }
        _vec.resize(size);
        if (pread(_data_fp, (void *)&_vec[0], size*sizeof(DataT),
            sizeof(DataHeaderT)+offset*sizeof(DataT)) < 0) {
            _vec.resize(0);
        }
        return &_vec;
    }
    PyObject *GetPyObject(vector<DataT> &vec) {
        int count = 0,length = vec.size();
        if (length == 0) {
            return PyList_New(0);
        }
        PyObject *list = PyList_New(length);
        if (list == NULL) {
            return PyList_New(0);
        }
        for (typename vector<DataT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            PyObject *obj = DataT::GetPyObject(*iter);
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
    PyObject *GetPyObjectSimple(vector<DataT> &vec) {
        int count = 0,length = vec.size();
        if (length == 0) {
            return PyList_New(0);
        }
        PyObject *list = PyList_New(length);
        if (list == NULL) {
            return PyList_New(0);
        }
        for (typename vector<DataT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            PyObject *obj = DataT::GetPyObjectSimple(*iter);
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
    PyObject *GetPyObjectVec(vector<DataT> &vec) {
        return DataT::GetPyObject(vec);
    }

    void Print(vector<DataT> &vec) {
        for (typename vector<DataT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            DataT::Print(*iter);
        }
    }
    void GetSize(uint64_t &index_size, uint64_t &data_size) {
        index_size = sizeof(IndexHeaderT) + _off_max * sizeof(IndexT);
        data_size = sizeof(DataHeaderT) + _size_max * sizeof(DataT);
    }
private:
    int _idx_fp, _data_fp;
    uint64_t _size_max;
    uint64_t _off_max;
    uint64_t _return_size_max;
    vector<DataT> _vec;
};

class FrameReaderManager {
public:
    FrameReaderManager() {
        _index_header_reader = new FrameHeaderReader<FrameIndexHeader>();
        _data_header_reader = new FrameHeaderReader<FrameDataHeader>();

        _if_reader = new FrameReader<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataIFBody>;
        _ti_reader = new FrameReader<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataTIBody>;
        _string_reader = new FrameReader<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataStringBody>;
        _u32_reader = new FrameReader<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataU32Body>;

        _type = FrameTypeNone;
    }
    ~FrameReaderManager() {
        _if_reader->Close();
        _ti_reader->Close();
        _string_reader->Close();
        _u32_reader->Close();
        delete _if_reader;
        delete _ti_reader;
        delete _string_reader;
        delete _u32_reader;
        delete _index_header_reader;
        delete _data_header_reader;
    }
    void Init(char *string1, char *string2, int return_size_max) {
        if (return_size_max <= 0) {
            printf("!!!matrixbuilder lib error!!! set frame size fail\n");
            return ;
        }
        FrameIndexHeader index_header = _index_header_reader->GetHeader(string1);
        FrameDataHeader data_header = _data_header_reader->GetHeader(string2);
        if (index_header.off_max == 0 || index_header.type == (uint32_t)FrameTypeNone
            || data_header.off_max == 0 || data_header.type == (uint32_t)FrameTypeNone
            || index_header.type != data_header.type) {
            printf("!!!matrixbuilder lib error!!! read frame files fail\n");
            return ;
        }
        if (index_header.type == (uint32_t)FrameTypeIF) {
            _type = FrameTypeIF;
            _if_reader->Open(string1, string2, (uint32_t)return_size_max,
                index_header.off_max, data_header.off_max);
        } else if (index_header.type == (uint32_t)FrameTypeTI) {
            _type = FrameTypeTI;
            _ti_reader->Open(string1, string2, (uint32_t)return_size_max,
                index_header.off_max, data_header.off_max);
        } else if (index_header.type == (uint32_t)FrameTypeString) {
            _type = FrameTypeString;
            _string_reader->Open(string1, string2, (uint32_t)return_size_max,
                index_header.off_max, data_header.off_max);
        } else if (index_header.type == (uint32_t)FrameTypeU32) {
            _type = FrameTypeU32;
            _u32_reader->Open(string1, string2, (uint32_t)return_size_max,
                index_header.off_max, data_header.off_max);
        } 
        return ;
    }
    PyObject *Read(long uid, int len) {
        if (uid < 0 || len <= 0) {
            return PyList_New(0);
        }
        if (_type == FrameTypeIF) {
            vector<FrameDataIFBody> *vec = _if_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _if_reader->GetPyObject(*vec);
        } else if (_type == FrameTypeTI) {
            vector<FrameDataTIBody> *vec = _ti_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _ti_reader->GetPyObject(*vec);
        } else if (_type == FrameTypeString) {
            vector<FrameDataStringBody> *vec 
                = _string_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _string_reader->GetPyObjectVec(*vec);
        } else if (_type == FrameTypeU32) {
            vector<FrameDataU32Body> *vec 
                = _u32_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _u32_reader->GetPyObject(*vec);
        }
        return PyList_New(0);
    }
    PyObject *ReadVec(long uid, int len) {
        if (uid < 0 || len <= 0) {
            return PyList_New(0);
        }
        if (_type == FrameTypeIF) {
            vector<FrameDataIFBody> *vec = _if_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _if_reader->GetPyObjectVec(*vec);
        } else if (_type == FrameTypeTI) {
            vector<FrameDataTIBody> *vec = _ti_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _ti_reader->GetPyObjectVec(*vec);
        } else if (_type == FrameTypeString) {
            vector<FrameDataStringBody> *vec 
                = _string_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _string_reader->GetPyObjectVec(*vec);
        } else if (_type == FrameTypeU32) {
            vector<FrameDataU32Body> *vec 
                = _u32_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _u32_reader->GetPyObjectVec(*vec);
        }
        return PyList_New(0);
    }
    PyObject *ReadSimple(long uid, int len) {
        if (uid < 0 || len <= 0) {
            return PyList_New(0);
        }
        if (_type == FrameTypeIF) {
            vector<FrameDataIFBody> *vec = _if_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _if_reader->GetPyObjectSimple(*vec);
        } else if (_type == FrameTypeTI) {
            vector<FrameDataTIBody> *vec = _ti_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _ti_reader->GetPyObjectSimple(*vec);
        } else if (_type == FrameTypeString) {
            vector<FrameDataStringBody> *vec 
                = _string_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _string_reader->GetPyObjectSimple(*vec);
        } else if (_type == FrameTypeU32) {
            vector<FrameDataU32Body> *vec 
                = _u32_reader->GetData((uint64_t)uid, (uint32_t)len);
            return _u32_reader->GetPyObjectSimple(*vec);
        }
        return PyList_New(0);
    }
    void GetSize(uint64_t &index_size, uint64_t &data_size) {
        index_size = 0;
        data_size = 0;
        if (_type == FrameTypeIF) {
            _if_reader->GetSize(index_size, data_size);
            return ;
        } else if (_type == FrameTypeTI) {
            _ti_reader->GetSize(index_size, data_size);
            return ;
        } else if (_type == FrameTypeString) {
            _string_reader->GetSize(index_size, data_size);
            return ;
        } else if (_type == FrameTypeU32) {
            _u32_reader->GetSize(index_size, data_size);
            return ;
        }
    }
private:
    FrameHeaderReader<FrameIndexHeader> *_index_header_reader;
    FrameHeaderReader<FrameDataHeader> *_data_header_reader;
    FrameReader<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataIFBody> *_if_reader;
    FrameReader<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataTIBody> *_ti_reader;
    FrameReader<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataStringBody> *_string_reader;
    FrameReader<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataU32Body> *_u32_reader;
    FrameType _type;
};


#endif

