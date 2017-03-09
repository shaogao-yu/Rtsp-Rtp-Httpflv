#ifndef __HTTP_FLV_H__
#define __HTTP_FLV_H__

#include "http_server.h"

typedef struct
{
    int fd;
}t_flv_client_def;

class HttpFlv : public HttpServer
{
private:
    int m_client_max;
    int m_video_width;
    int m_video_height;
    char *mp_sps;
    int m_sps_len;
    char *mp_pps;
    int m_pps_len;
    t_flv_client_def *mpt_flv_client;
public:
    HttpFlv();
    HttpFlv(int port,int client_max,int width,int height);
    ~HttpFlv();
    void set_h264_pps_sps(char *p_sps,int sps_len,char *mp_pps,int pps_len);
    void add_flv_client(int fd);
    void del_flv_client(int fd);
    void send_flv_stream_proc(void);
    int send_file_hook_handle(int client_fd,char *p_file);
    void run(void);
};

#endif

