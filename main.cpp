#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "epoll_server.h"
#include "http_server.h"
#include "http_flv.h"
#include "v4l_n32926.h"
#include "video2yuv_file.h"
#include "video2h264_file.h"
#include "video2flv_file.h"
#include "rtsp_rtp_h264_server.h"

using namespace std;

int main(int argc,char **argv)
{
    //Epoll服务器 ,发什么回什么demo
    //EpollServer epoll_server(1234,5);
    //epoll_server.epoll_run();

    //输出YUV文件 demo
    //Video2YuvFile video2yuv_file("/tmp/test_640x480.yuv");
    //video2yuv_file.v4l_dev_open("/dev/video0",640,480);
    //video2yuv_file.v4l_yuv_sample_run();

    //输出H264格式 demo
    //Video2H264File video2h264_file("/tmp/test_640x480.h264");
    //video2h264_file.v4l_dev_open("/dev/video0",640,480);
    //video2h264_file.v4l_capture_start();
    //video2h264_file.v4l_h264_open("/dev/w55fa92_264enc");
    //video2h264_file.v4l_h264_sample_run();

    //输出FLV文件格式 demo
    //Video2FlvFile video2flv_file("/tmp/test_640x480.flv");
    //video2flv_file.v4l_dev_open("/dev/video0",640,480);
    //video2flv_file.v4l_capture_start();
    //video2flv_file.v4l_h264_open("/dev/w55fa92_264enc");
    //video2flv_file.v4l_h264_sample_run();

    //RTSP RTP 传输H264
    RtspRtpH264Server rtsp_rtp_h264_server(1233,10);
    rtsp_rtp_h264_server.run();
    
    //HTTP服务器 demo
    //HttpServer http_server(8080,10);
    //http_server.http_run();

   //HttpFlv http_flv;
    //http_flv.run();

    return 0;
}

