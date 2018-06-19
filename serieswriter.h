#ifndef _SERIESWRITER_H_
#define _SERIESWRITER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <exception>
#include <fcntl.h>  
#include "series.h"

using namespace std;


template<typename HeaderT, typename IndexT, typename BodyT>
class SeriesWriter {
public:
    SeriesWriter() {
        _data_fp = NULL;
        _data = NULL;
    }
    ~SeriesWriter() {
        if (_data != NULL) {
            free(_data);
            _data = NULL;
        }
    }
    void Open(const char *data_file, uint32_t block_cap, uint32_t off_max, 
        SeriesType type)throw(exception) {
        _data_fp = fopen(data_file, "w");
        if (_data_fp == NULL) {
            printf("open data fail\n");
            throw exception();
        }
        setvbuf(_data_fp, _data_buffer, _IOFBF, _buffer_size);

        _off_max = off_max;
        _real_off_max = 0;
        _block_cap = block_cap;
        _type = (uint32_t)type;
        _data_vec.reserve(_block_cap*2);

        if (block_cap > IDX_LIMIT) {
            _idx_flag = true;
            _block_len = sizeof(IndexT)+sizeof(BodyT)*_block_cap;
        } else {
            _idx_flag = false;
            _block_len = sizeof(BodyT)*_block_cap;
        }
        _data = (char *)malloc(_block_len*(_off_max+1));
        if (_data == NULL) {
            printf("malloc fail\n");
            throw exception();
        }
        memset((void *)_data, 0, _block_len*(_off_max+1));
    }
    void Close()throw(exception) {
        if (_data_fp != NULL) {
            if (fseek(_data_fp, 0, SEEK_SET) < 0) {
                printf("seek data fail\n");
                throw exception();
            }
            HeaderT header;
            header.off_max = _real_off_max;
            header.block_cap = _block_cap;
            header.type = _type;
            if (fwrite(&header, sizeof(HeaderT), 1, _data_fp) <= 0) {
                printf("write data header fail\n");
                throw exception();
            }
            if (fwrite(_data, _block_len*(_real_off_max+1), 1, _data_fp) <= 0) {
                printf("write data fail\n");
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
        if (_data != NULL) {
            free(_data);
            _data = NULL;
        }
    }
    void Append(uint32_t uid, char *data, uint32_t len) {
        if (uid > _off_max) {
            return ;
        }

        _data_vec.resize(0);
        _data_vec = BodyT::ParseString(_data_vec, data, len);
        if (_data_vec.size() == 0) {
            return ;
        }

        if (_data_vec.size() > _block_cap) {
            _data_vec.resize(_block_cap);
        }

        if (_idx_flag) {
            IndexT *index = (IndexT *)(_data + uid*_block_len);
            index->size = _data_vec.size();
            memcpy((void *)(_data + uid*_block_len + sizeof(IndexT)), &_data_vec[0],
                _data_vec.size() * sizeof(BodyT));
        } else {
            memcpy((void *)(_data + uid*_block_len), &_data_vec[0],
                _data_vec.size() * sizeof(BodyT));
        }

        if (_real_off_max < uid) {
            _real_off_max = uid;
        }
    }
    void Print(vector<BodyT> &vec) {
        for (typename vector<BodyT>::iterator iter = vec.begin(); iter != vec.end(); iter++) {
            BodyT::Print(*iter);
        }
    }
private:
    FILE *_data_fp;
    static const uint32_t _buffer_size = 32 * 1024 * 1024;
    char _data_buffer[_buffer_size];
    uint32_t _off_max;
    uint32_t _real_off_max;
    uint32_t _block_cap;
    uint32_t _block_len;
    uint32_t _type;
    bool _idx_flag;
    char *_data;
    vector<BodyT> _data_vec;
};

class SeriesWriterManager {
public:
    SeriesWriterManager() {
        _int_writer = new SeriesWriter<SeriesHeader, SeriesIntIndex, SeriesIntBody>();
        _float_writer = new SeriesWriter<SeriesHeader, SeriesFloatIndex, SeriesFloatBody>();
        _score_writer = new SeriesWriter<SeriesHeader, SeriesScoreIndex, SeriesScoreBody>();
    }
    ~SeriesWriterManager() {
        _int_writer->Close();
        _float_writer->Close();
        _score_writer->Close();

        delete _int_writer;
        delete _float_writer;
        delete _score_writer;
    }
    void Init(char *string1, char *string2, int block_cap, int max_id) {
        if (block_cap < 0 || max_id < 0) {
            printf("param error!!!\n");
            return ;
        }
        if (strcmp("int", string2) == 0) {
            _type = SeriesTypeInt;
            _int_writer->Open(string1, (uint32_t)block_cap, (uint32_t)max_id, _type);
        } else if (strcmp("float", string2) == 0) {
            _type = SeriesTypeFloat;
            _float_writer->Open(string1, (uint32_t)block_cap, (uint32_t)max_id, _type);
        } else if (strcmp("score", string2) == 0) {
            _type = SeriesTypeScore;
            _score_writer->Open(string1, (uint32_t)block_cap, (uint32_t)max_id, _type);
        }
    }
    void Write(int id, char *value, int len) {
        if (id < 0 || len < 0) {
            return ;
        }
        if (_type == SeriesTypeInt) {
            _int_writer->Append(id, value, len);
        } else if (_type == SeriesTypeFloat) {
            _float_writer->Append(id, value, len);
        } else if (_type == SeriesTypeScore) {
            _score_writer->Append(id, value, len);
        }
    }

private:
    SeriesWriter<SeriesHeader, SeriesIntIndex, SeriesIntBody> *_int_writer;
    SeriesWriter<SeriesHeader, SeriesFloatIndex, SeriesFloatBody> *_float_writer;
    SeriesWriter<SeriesHeader, SeriesScoreIndex, SeriesScoreBody> *_score_writer;
    SeriesType _type;
};

#endif


