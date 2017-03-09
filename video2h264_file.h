#ifndef __VIDEO2H264_FILE_H__
#define __VIDEO2H264_FILE_H__

#include "v4l_n32926.h"

class Video2H264File : public V4lN32926
{
private:
    FILE *mfp_h264;
public:
    Video2H264File(const char *p_h264_file);
    ~Video2H264File();
    void v4l_h264_sample_handle(char *p_h264_data,int data_len);
};

#endif

