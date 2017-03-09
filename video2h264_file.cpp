#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "video2h264_file.h"

Video2H264File::Video2H264File(const char *p_h264_file)
{
    mfp_h264 = fopen(p_h264_file,"w+");

    if(mfp_h264 == NULL)
    {
        printf("Open H264 File Fail:%s\r\n",p_h264_file);
    }
}

Video2H264File::~Video2H264File()
{
    fclose(mfp_h264);
}

void Video2H264File::v4l_h264_sample_handle(char *p_h264_data,int data_len)
{
    int nwrite;

    printf("H264 Data Addr:%08x,Data Len:%d\r\n",p_h264_data,data_len);

    nwrite = fwrite(p_h264_data,1,data_len,mfp_h264);
    if(nwrite != data_len)
    {
        printf("Write H264 File Fail\r\n");
    }
}

