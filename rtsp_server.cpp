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

#include "rtsp_server.h"

RtspServer::RtspServer()
{
    
}

RtspServer::RtspServer(int port,int client_max):EpollServer(port,client_max),m_listen_port(port)
{

}

RtspServer::~RtspServer()
{
    
}

int RtspServer::epoll_recv(int client_fd)
{
    char recv_buf[MAXSIZE];
	char send_buf[MAXSIZE];
    int nread;
    char *p_tmpchar;
    int tmp_port;
    char tmp_ip[IP_BUF_LEN];
    
    memset(recv_buf,0x00,MAXSIZE);
    nread = read(client_fd,recv_buf,MAXSIZE);

    printf("http server recv data:\r\n%s\r\n",recv_buf);
    if(nread <= 0)
    {
        return nread;
    }

    memset(send_buf,0x00,MAXSIZE);

    if(strncmp(recv_buf,"OPTIONS",strlen("OPTIONS")) == 0)
    {   
         printf("recv cmd:%s\r\n","OPTIONS");

         snprintf(send_buf,sizeof(send_buf),
             "RTSP/1.0 200 OK\r\n"
             "CSeq: %s\r\n"
             "%s"
             "Public: OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY\r\n\r\n",
             rtsp_get_CSeq(recv_buf), rtsp_date_header());
             write(client_fd,send_buf,strlen(send_buf));
    }
	else if(strncmp(recv_buf,"DESCRIBE",strlen("DESCRIBE")) == 0)
    {
        snprintf(send_buf,sizeof(send_buf),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %s\r\n"
            "%s"
            "Content-Base: %s/\r\n"
            "Content-Type: application/sdp\r\n"
            "Content-Length: %d\r\n\r\n"
            "%s\r\n\r\n",
            rtsp_get_CSeq(recv_buf),
            rtsp_date_header(),
            rtsp_get_URL(client_fd),
            rtsp_get_dsp_desc_len(client_fd),
            rtsp_get_dsp_desc_data(client_fd));
        write(client_fd,send_buf,strlen(send_buf));
    }
    else if(strncmp(recv_buf,"SETUP",strlen("SETUP")) == 0)
    {
        memset(tmp_ip,0x00,sizeof(tmp_ip));

        snprintf(send_buf,sizeof(send_buf),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %s\r\n"
            "%s"
            "Transport: %s\r\n"
            "Session: 1234567890\r\n\r\n",
            rtsp_get_CSeq(recv_buf),
            rtsp_date_header(),
            rtsp_get_transport(client_fd,recv_buf,&tmp_port,tmp_ip));

        write(client_fd,send_buf,strlen(send_buf));

        rtsp_setup_rtp(client_fd,tmp_ip,tmp_port);
    }
    else if(strncmp(recv_buf,"PLAY",strlen("PLAY")) == 0)  
    {   
        snprintf(send_buf,sizeof(send_buf),
            "RTSP/1.0 200 OK\r\n"
            "CSeq: %s\r\n"
            "%s"
            "Range: npt=0.000-\r\n"
            "Session: 1234567890\r\n"
            "RTP-Info: %s\r\n\r\n",
            rtsp_get_CSeq(recv_buf),
            rtsp_date_header(),
            rtsp_get_video_info(client_fd));

        write(client_fd,send_buf,strlen(send_buf));

        rtsp_play_rtp(client_fd);
    }   
    else if(strncmp(recv_buf,"TEARDOWN",strlen("TEARDOWN")) == 0)
    {
        rtsp_close_rtp(client_fd);
    }
    else
    {   
        printf("Unknow cmd\r\n");
    }

    if(send_buf[0] != 0x00)
    {
        printf("send data:\r\n%s\r\n",send_buf);
    }

    return 1;
}

int RtspServer::epoll_send(int client_fd)
{
    
}

void RtspServer::epoll_close(int client_fd)
{
    
}

char *RtspServer::rtsp_date_header(void)
{
    static char buf[200];

    memset(buf,0,200);
    time_t tt = time(NULL);
    strftime(buf, sizeof(buf), "Date: %a, %b %d %Y %H:%M:%S GMT\r\n",gmtime(&tt));
    return buf;
}

char *RtspServer::rtsp_get_CSeq(char *p_data)
{
    static char buf[200];
    char *p_pos;
    int i,j;

    memset(buf,0,200);
    p_pos = (char*)strcasestr(p_data,"CSeq:");
    if(p_pos != NULL)
    {
        for(i = 0,j = 0;p_pos[i + 5] != '\r' && p_pos[i + 5] != '\n';i++)
        {
            if(p_pos[i + 5] != ' ')
            {
                buf[j] = p_pos[i + 5];
                j++;
            }
        }
    }

    return buf; 
}

char *RtspServer::rtsp_get_URL(int clientfd)
{
    static char urlBuffer[200];
    struct sockaddr_in ourAddress;
    int namelen = sizeof(ourAddress);

    memset(urlBuffer,0,200);
    //获取跟客户端绑定的IP地址
    getsockname(clientfd, (struct sockaddr*)&ourAddress, (socklen_t*)&namelen);

    if(m_listen_port == 554)
    {
        sprintf(urlBuffer, "rtsp://%s/%s",inet_ntoa(ourAddress.sin_addr),STREAM_NAME);
    }
    else
    {
        sprintf(urlBuffer, "rtsp://%s:%hu/%s",inet_ntoa(ourAddress.sin_addr),m_listen_port,STREAM_NAME);
    }

    return urlBuffer;
}

char *RtspServer::rtsp_get_dsp_desc_data(int clientfd)
{
    static char sdpbuf[1000];
    char ip_addr[30];
    struct sockaddr_in ourAddress;
    int namelen = sizeof(ourAddress);

    memset(sdpbuf,0,1000);
    //获取跟客户端绑定的IP地址
    getsockname(clientfd, (struct sockaddr*)&ourAddress, (socklen_t*)&namelen);
    sprintf(ip_addr,"%s",inet_ntoa(ourAddress.sin_addr));

    snprintf(sdpbuf,sizeof(sdpbuf),
        "v=0\r\n"
        "o=- 0 0 IN IP4 %s\r\n"
        "s=No Title\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "t=0 0\r\n"
        "a=framerate:25\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z0IAH+kBQHsg,aM44gA==; profile-level-id=64002A\r\n"
        "a=control:streamid=1"
        //"m=audio 6666 RTP/AVP 97"
        //"a=rtpmap:97 MPEG4-GENERIC/44100/1"
        //"a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=1208"
        //"a=control:streamid=1",
        ,ip_addr);

    return sdpbuf;
}

int RtspServer::rtsp_get_dsp_desc_len(int clientfd)
{
    static char sdpbuf[1000];
    char ip_addr[30];
    struct sockaddr_in ourAddress;
    int namelen = sizeof(ourAddress);

    memset(sdpbuf,0,1000);
    //获取跟客户端绑定的IP地址
    getsockname(clientfd, (struct sockaddr*)&ourAddress, (socklen_t*)&namelen);
    sprintf(ip_addr,"%s",inet_ntoa(ourAddress.sin_addr));

    snprintf(sdpbuf,sizeof(sdpbuf),
        "v=0\r\n"
        "o=- 0 0 IN IP4 %s\r\n"
        "s=No Title\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "t=0 0\r\n"
        "a=framerate:25\r\n"
        "m=video 0 RTP/AVP 96\r\n"
        "a=rtpmap:96 H264/90000\r\n"
        "a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z0IAH+kBQHsg,aM44gA==; profile-level-id=64002A\r\n"
        "a=control:streamid=1"
        //"m=audio 6666 RTP/AVP 97"
        //"a=rtpmap:97 MPEG4-GENERIC/44100/1"
        //"a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=1208"
        //"a=control:streamid=1",
        ,ip_addr);

    return strlen(sdpbuf);
}

char *RtspServer::rtsp_get_transport(int clientfd,char *request_data,int *p_cli_port,char *p_ip)
{
    static char transport[1000];
    struct sockaddr_in ip_addr;
    int namelen = sizeof(ip_addr);
    char ip_des_addr[30];
    char ip_src_addr[30];
    int cli_port1,cli_port2;
    int i;
    char *p_tmp;

    memset(transport,0,sizeof(transport));
    p_tmp = strstr(request_data,"client_port=");
    for(i = 0;i < 100;i++)
    {
        if(p_tmp[i] == '\r' || p_tmp[i] == ';')
        {
            break;
        }

        transport[i] = p_tmp[i];
    }

    sscanf(transport,"client_port=%d-%d",&cli_port1,&cli_port2);
    *p_cli_port = cli_port1;

    //获取跟客户端绑定的IP地址
    getsockname(clientfd,(struct sockaddr*)&ip_addr, (socklen_t*)&namelen);
    sprintf(ip_src_addr,"%s",inet_ntoa(ip_addr.sin_addr));
    //获取客户端地址
    getpeername(clientfd,(struct sockaddr *)&ip_addr,(socklen_t*)&namelen);
    sprintf(ip_des_addr,"%s",inet_ntoa(ip_addr.sin_addr));

    strcpy(p_ip,ip_des_addr);

    snprintf(transport,sizeof(transport),
        "RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=6970-6971",
        ip_des_addr,ip_src_addr,cli_port1,cli_port2);

    return transport;
}

char *RtspServer::rtsp_get_video_info(int clientfd)
{
    static char rtpinfo[500];
    char ip_addr[30];
    struct sockaddr_in ourAddress;
    int namelen = sizeof(ourAddress);

    memset(rtpinfo,0,sizeof(rtpinfo));
    //获取跟客户端绑定的IP地址
    getsockname(clientfd, (struct sockaddr*)&ourAddress, (socklen_t*)&namelen);

    if(m_listen_port == 554)
    {
        sprintf(rtpinfo, "url=rtsp://%s/%s/streamid=1",inet_ntoa(ourAddress.sin_addr),STREAM_NAME);
    }
    else
    {
        sprintf(rtpinfo, "url=rtsp://%s:%hu/%s/streamid=1",inet_ntoa(ourAddress.sin_addr),m_listen_port,STREAM_NAME);
    }

    return rtpinfo;
}

void RtspServer::rtsp_setup_rtp(int fd,char *ip,int port)
{
    
}

void RtspServer::rtsp_play_rtp(int fd)
{
    
}

void RtspServer::rtsp_close_rtp(int fd)
{
    
}

void RtspServer::rtsp_run(void)
{
    epoll_run();    
}
