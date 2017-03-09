#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include "epoll_server.h"

class HttpServer : public EpollServer
{
public:
    HttpServer();
    HttpServer(int port,int client_max);
    ~HttpServer();
    void get_res_type(char *p_type,char *p_con_type);
    int send_file(int client_fd,char *p_file);
    int epoll_recv(int client_fd);
    int client_send(int client_fd);
    virtual int send_file_hook_handle(int client_fd,char *p_file);
    void http_run(void);
};



#endif
