#ifndef __RTSP_RTP_H264_SERVER_H__
#define __RTSP_RTP_H264_SERVER_H__

#include "v4l_n32926.h"
#include "epoll_server.h"
#include "rtp_server.h"
#include "rtsp_server.h"

class RtspRtpH264Server : public RtspServer,public V4lN32926,public RtpServer
{
private:
    pthread_t ph_h264_sample;
    pthread_t ph_rtp_server;
public:
    static void *h264_sample_handle(void *arg);
    static void *rtp_server_handle(void *arg);
    void run(void);
    RtspRtpH264Server(int port,int cli_max);
    ~RtspRtpH264Server();
    void rtsp_setup_rtp(int fd,char *ip,int port);
    void rtsp_play_rtp(int fd);
    void rtsp_close_rtp(int fd);
    void epoll_close(int client_fd);
    void v4l_h264_sample_handle(char *p_h264_data,int data_len);
};

#endif
