#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#include "epoll_server.h"

#define         IP_BUF_LEN      20
#define         STREAM_NAME     "oge.h264"

class RtspServer : public EpollServer
{
private:
    int m_listen_port;
public:
    RtspServer();
    RtspServer(int port,int client_max);
    ~RtspServer();
    virtual int epoll_recv(int client_fd);
    virtual int epoll_send(int client_fd);
    virtual void epoll_close(int client_fd);
	char *rtsp_date_header(void);
	char *rtsp_get_CSeq(char *p_data);
	char *rtsp_get_URL(int clientfd);
	int rtsp_get_dsp_desc_len(int clientfd);
	char *rtsp_get_dsp_desc_data(int clientfd);
	char *rtsp_get_transport(int clientfd,char *request_data,int *p_cli_port,char *p_ip);
    char *rtsp_get_video_info(int clientfd);
    virtual void rtsp_setup_rtp(int fd,char *ip,int port);
    virtual void rtsp_play_rtp(int fd);
    virtual void rtsp_close_rtp(int fd);
    void rtsp_run(void);
};


#endif

