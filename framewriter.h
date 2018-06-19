#ifndef _FRAME_WRITER_H_
#define _FRAME_WRITER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include "frame.h"

using namespace std;

template<typename IndexHeaderT, typename IndexT, typename DataHeaderT, typename DataT>
class FrameWriter {
public:
    FrameWriter() {
        _idx_fp = NULL;
        _data_fp = NULL;
    }
    ~FrameWriter() {
    }
    void Open(const char *idx_file, const char *data_file, long max_id,
        FrameType type)throw(exception) {
        _idx_fp = fopen(idx_file, "w");
        if (_idx_fp == NULL) {
            printf("open idx fail\n");
            throw exception();
        }
        setvbuf(_idx_fp, _idx_buffer, _IOFBF, _buffer_size);
        _data_fp = fopen(data_file, "w");
        if (_data_fp == NULL) {
            printf("open data fail\n");
            throw exception();
        }
        setvbuf(_data_fp, _data_buffer, _IOFBF, _buffer_size);
        _offset = 0;
        if (fseek(_data_fp, sizeof(DataHeaderT), SEEK_SET) < 0) {
            printf("seek fail\n");
            throw exception();
        }
        _off_max = max_id;
        _real_off_max = 0;
        _type = (uint32_t)type;
        _idx_vec.resize(_off_max);
    }
    void Close()throw(exception) {
        if (_idx_fp == NULL || _data_fp == NULL) {
            return ;
        }
        if (fseek(_idx_fp, 0, SEEK_SET) < 0) {
            printf("seek idx fail\n");
            throw exception();
        }
        IndexHeaderT indexheader(_real_off_max, _type);
        if (fwrite(&indexheader, sizeof(IndexHeaderT), 1, _idx_fp) <= 0) {
            printf("write idx header fail\n");
            throw exception();
        }
        _idx_vec.resize(_real_off_max+1);
        if (fwrite(&(_idx_vec[0]), sizeof(IndexT), _idx_vec.size(), _idx_fp) <= 0) {
            printf("write idx fail\n");
            throw exception();
        }
        if (fflush(_idx_fp) != 0) {
            printf("flush idx fail\n");
            throw exception();
        }
        if (fclose(_idx_fp) != 0) {
            printf("close idx fail\n");
            throw exception();
        }

        if (fseek(_data_fp, 0, SEEK_SET) < 0) {
            printf("seek data fail\n");
            throw exception();
        }
        DataHeaderT bodyheader(_offset, _type);
        if (fwrite(&bodyheader, sizeof(DataHeaderT), 1, _data_fp) <= 0) {
            printf("write data header fail\n");
            throw exception();
        }
        if (fflush(_data_fp) != 0) {
            printf("flush data fail\n");
            throw exception();
        }
        if (fclose(_data_fp) != 0) {
            printf("close data fail\n");
            throw exception();
        }
    }
    void Append(uint64_t uid, char *data, uint32_t len) {
        if (uid > _off_max) {
            return ;
        }

        _data_vec.resize(0);
        _data_vec = DataT::ParseString(_data_vec, data, len);

        uint64_t count = 0;
        int ret;
        for (typename vector<DataT>::iterator iter = _data_vec.begin();
            iter != _data_vec.end(); iter++) {
            ret = fwrite(&(*iter), sizeof(DataT), 1, _data_fp);
            if (ret <= 0) {
                printf("write error\n");
            } else {
                count++; 
            }
        }

        while ((uint64_t)uid > _idx_vec.size()) {
            _idx_vec.resize(_idx_vec.size()+50000000);
        }
        if (_real_off_max < uid) {
            _real_off_max = uid;
        }

        _idx_vec[uid].offset = _offset;
        _idx_vec[uid].count = count;

        _offset += count;
    }
    void Print(vector<DataT> &vec) {
        for (typename vector<DataT>::iterator iter = vec.begin();
            iter != vec.end(); iter++) {
            DataT::Print(*iter);
        }
    }
private:
    FILE *_idx_fp, *_data_fp;
    static const int _buffer_size = 32 * 1024 * 1024;
    char _idx_buffer[_buffer_size], _data_buffer[_buffer_size];
    uint64_t _offset;
    char _buffer[512];
    uint64_t _off_max;
    uint64_t _real_off_max;
    uint32_t _type;
    vector<IndexT> _idx_vec;
    vector<DataT> _data_vec;
};

class FrameWriterManager {
public:
    FrameWriterManager() {
        _if_writer = new FrameWriter<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataIFBody>();
        _ti_writer = new FrameWriter<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataTIBody>();
        _string_writer = new FrameWriter<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataStringBody>();
        _u32_writer = new FrameWriter<FrameIndexHeader, FrameIndexNormalBody,
            FrameDataHeader, FrameDataU32Body>();
    }
    ~FrameWriterManager() {
        _if_writer->Close();
        _ti_writer->Close();
        _string_writer->Close();
        _u32_writer->Close();

        delete _if_writer;
        delete _ti_writer;
        delete _string_writer;
        delete _u32_writer;
    }
    void Init(char *string1, char *string2, char *string3, long max_id) {
        if (max_id < 0) {
            printf("param error!!!\n");
            return ;
        }
        if (strcmp("int_float", string3) == 0) {
            _type = FrameTypeIF;
            _if_writer->Open(string1, string2, (uint64_t)max_id, _type);
        } else if (strcmp("type_int", string3) == 0) {
            _type = FrameTypeTI;
            _ti_writer->Open(string1, string2, (uint64_t)max_id, _type);
        } else if (strcmp("string", string3) == 0) {
            _type = FrameTypeString;
            _string_writer->Open(string1, string2, (uint64_t)max_id, _type);
        } else if (strcmp("u32", string3) == 0) {
            _type = FrameTypeU32;
            _u32_writer->Open(string1, string2, (uint64_t)max_id, _type);
        }
    }
    void Write(long id, char *value, int len) {
        if (id < 0 || len < 0) {
            return ;
        }
        if (_type == FrameTypeIF) {
            _if_writer->Append((uint64_t)id, value, (uint64_t)len);
        } else if (_type == FrameTypeTI) {
            _ti_writer->Append((uint64_t)id, value, (uint64_t)len);
        } else if (_type == FrameTypeString) {
            _string_writer->Append((uint64_t)id, value, (uint64_t)len);
        } else if (_type == FrameTypeU32) {
            _u32_writer->Append((uint64_t)id, value, (uint64_t)len);
        }
    }

private:
    FrameWriter<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataIFBody> *_if_writer;
    FrameWriter<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataTIBody> *_ti_writer;
    FrameWriter<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataStringBody> *_string_writer;
    FrameWriter<FrameIndexHeader, FrameIndexNormalBody, FrameDataHeader,
        FrameDataU32Body> *_u32_writer;
    FrameType _type;
};


#endif


