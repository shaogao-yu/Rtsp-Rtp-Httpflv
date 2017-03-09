#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "video2yuv_file.h"

Video2YuvFile::Video2YuvFile(const char *p_yuv_file)
{
    mfp_yuv = fopen(p_yuv_file,"w+");

    if(mfp_yuv == NULL)
    {
        printf("Open Yuv File Fail:%s\r\n",p_yuv_file);
    }
}

Video2YuvFile::~Video2YuvFile()
{
    fclose(mfp_yuv);
}

void Video2YuvFile::v4l_yuv_sample_handle(char *p_yuv_data,int data_len)
{
    int nwrite;
    static int cap_num = 0;

    nwrite = fwrite(p_yuv_data,1,data_len,mfp_yuv);
    if(nwrite != data_len)
    {
        printf("Write Yuv File Fail\r\n");
    }

    //抓捕10帧做测试
    cap_num++;
    if(cap_num >= 10)
    {
        exit(1);
    }
}

