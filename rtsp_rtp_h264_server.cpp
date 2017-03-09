#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <unistd.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fifo_list.h"
}

#include "rtsp_rtp_h264_server.h"

void RtspRtpH264Server::v4l_h264_sample_handle(char *p_h264_data,int data_len)
{
    int nalu_valid;
    char *p_data;

    //检测NALU是否有效
    nalu_valid = 0;
    if(p_h264_data[0] == 0x00 && p_h264_data[1] == 0x00 && \
        p_h264_data[2] == 0x00 && p_h264_data[3] == 0x01)
    {   
        nalu_valid = 4;
    }   
    else if(p_h264_data[0] == 0x00 && p_h264_data[1] == 0x00 && \
        p_h264_data[2] == 0x01)
    {   
        nalu_valid = 3;
    } 

    if(!nalu_valid)
    {
        return;
    }

    p_data = &p_h264_data[nalu_valid];
    data_len -= nalu_valid;

    list_fifo_push(p_data,data_len);
}

RtspRtpH264Server::RtspRtpH264Server(int port,int cli_max):RtspServer(port,cli_max),RtpServer(cli_max)
{

}

RtspRtpH264Server::~RtspRtpH264Server()
{

}

void *RtspRtpH264Server::h264_sample_handle(void *arg)
{
    RtspRtpH264Server *ptr = (RtspRtpH264Server *)arg;

    ptr->v4l_dev_open("/dev/video0",640,480);
    ptr->v4l_capture_start();
    ptr->v4l_h264_open("/dev/w55fa92_264enc");
    ptr->v4l_h264_sample_run();
}

void *RtspRtpH264Server::rtp_server_handle(void *arg)
{
    RtspRtpH264Server *ptr = (RtspRtpH264Server *)arg;
    
    ptr->rtp_run();
}

void RtspRtpH264Server::run(void)
{
    list_fifo_init();

    if(0 != pthread_create(&ph_h264_sample,NULL,h264_sample_handle,this))
    {
        printf("Create H264 Sample Handle Pthread Fail\r\n");
        return;
    }

    if(0 != pthread_create(&ph_rtp_server,NULL,rtp_server_handle,this))
    {
        printf("Create Rtp Handle Pthread Fail\r\n");
        return;
    }

    rtsp_run();
}

void RtspRtpH264Server::rtsp_setup_rtp(int fd,char *ip,int port)
{
    rtp_setup_client(fd,ip,port);    
}

void RtspRtpH264Server::rtsp_close_rtp(int fd)
{
    rtp_close_client(fd);   
}

void RtspRtpH264Server::rtsp_play_rtp(int fd)
{
    rtp_play_client(fd);
}

void RtspRtpH264Server::epoll_close(int client_fd)
{
    rtp_close_client(client_fd);   
}

