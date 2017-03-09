#ifndef __V4L_N32926_H__
#define __V4L_N32926_H__

#include <linux/videodev.h>

extern "C"
{
#include "ratecontrol.h"
#include "favc_avcodec.h"
};

#define     VIDEO_PALETTE_YUV420P_MACRO         50/* YUV 420 Planar Macro */
#define     VIDIOCGCAPTIME                      _IOR('v',30,struct v4l2_buffer)      /*Get Capture time */
#define     VIDEO_START                         0
#define     VIDEO_STOP                          1

#define     RATE_CTL

class V4lN32926
{
private:
    int mfd_video;
    int mfd_h264;
    void *mbuf;
    int mbuf_size;
    int m_frame_index;
    struct video_mbuf m_v4l_mbuf;
    int m_favc_quant;
    H264RateControl m_h264_ratec;
public:
    int m_width;
    int m_height;
public:
    V4lN32926();
    ~V4lN32926();
    int v4l_dev_open(const char *p_video_dev,int video_width,int video_height);
    int v4l_h264_open(const char *p_h264_dev);
    int v4l_capture_start(void);
    void v4l_yuv_sample_run(void);
    virtual void v4l_yuv_sample_handle(char *p_yuv_data,int data_len);
    void v4l_h264_sample_run(void);
    virtual void v4l_h264_sample_handle(char *p_h264_data,int data_len);
private:
    int v4l_capture_stop(void);
    int v4l_capture_wait(void);
    unsigned int v4l_capture_get_frame_addr(int addr_type);
    int v4l_capture_frame(int yuv_type);
    void v4l_capture_next_frame(void);
    int v4l_h264_set_param(void);
    int v4l_h264_encode(unsigned int frame_addr,char *p_encode_addr);
};

#endif
