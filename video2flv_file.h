#ifndef __VIDEO2FLV_FILE_H__
#define __VIDEO2FLV_FILE_H__

extern "C"
{
#include "flv_mux.h"
}

#include "v4l_n32926.h"

class Video2FlvFile : public V4lN32926
{
private:
    FILE *mfp_flv;
public:
    Video2FlvFile(const char *p_flv_file);
    ~Video2FlvFile();
    void v4l_h264_sample_handle(char *p_h264_data,int data_len);
};

#endif

