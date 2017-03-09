#ifndef __VIDEO2YUV_FILE_H__
#define __VIDEO2YUV_FILE_H__

#include "v4l_n32926.h"

class Video2YuvFile : public V4lN32926
{
private:
    FILE *mfp_yuv;
public:
    Video2YuvFile(const char *p_yuv_file);
    ~Video2YuvFile();
    void v4l_yuv_sample_handle(char *p_yuv_data,int data_len);
};

#endif
