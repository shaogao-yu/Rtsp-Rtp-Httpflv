#ifndef __RTP_SERVER_H__
#define __RTP_SERVER_H__

extern "C"
{
#include <netinet/in.h>
}

typedef struct
{   
    unsigned char cc:4;
    unsigned char x:1;
    unsigned char p:1;
    unsigned char v:2;
      
    unsigned char pt:7;
    unsigned char m:1;
    
    unsigned short sequence_number;
    unsigned int timestamp;
    unsigned int ssrc;
}t_rtp_header_def;

typedef struct
{
    unsigned char Type:5;
    unsigned char NRI:2;
    unsigned char F:1;
}t_FU_indicator_def;

typedef struct
{
    unsigned char Type:5;
    unsigned char R:1;
    unsigned char E:1;
    unsigned char S:1;
}t_FU_header_def;

typedef struct
{
    int fd;
    char ip[20];
    int port;
    int seq_num;
    int timestamp;
    struct sockaddr_in addr;
    int play;
}t_rtp_client_def;

class RtpServer
{
private:
    t_rtp_client_def *mpt_rtp_client;
    int m_client_max;
    int m_rtp_fd;
    char *mp_rtp_buff;
    int m_MTU_value;
    unsigned int m_rtp_ssrc;
    unsigned int m_rtp_timestamp_inc;
    struct sockaddr_in m_addr;
public:
    RtpServer(int cli_max);
    ~RtpServer();
    int rtp_send_h264_nalu(char *p_nalu,int nalu_size,t_rtp_client_def *pt_rtp_client);//struct sockaddr_in *p_cli_addr);
    void rtp_setup_client(int fd,char *ip,int port);
    void rtp_play_client(int fd);
    void rtp_close_client(int fd);
    void rtp_run(void);
};

#endif

