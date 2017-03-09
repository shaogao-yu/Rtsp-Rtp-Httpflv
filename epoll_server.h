#ifndef __EPOLL_SERVER_H__
#define __EPOLL_SERVER_H__

#define MAXSIZE     1024
#define FDSIZE      1000
#define EPOLLEVENTS 100

class EpollServer
{
private:
    int m_client_max;
    int m_listen_port;
public:
    EpollServer();
    EpollServer(int listen_port,int client_max);
    ~EpollServer();
    void epoll_run(void);
    virtual int epoll_recv(int client_fd);
    virtual int epoll_send(int client_fd);
    virtual void epoll_close(int client_fd);
private:
    int epoll_socket_bind(char *p_ip,int port);
    int epoll_socket_listen(int listenfd,int listen_max);
    void epoll_do(int listenfd);
    void epoll_handle_events(int epollfd,struct epoll_event *events,int num,int listenfd);
    void epoll_handle_accpet(int epollfd,int listenfd);
    void epoll_do_read(int epollfd,int fd);
    void epoll_do_write(int epollfd,int fd);
    void epoll_add_event(int epollfd,int fd,int state);
    void epoll_delete_event(int epollfd,int fd,int state);
    void epoll_modify_event(int epollfd,int fd,int state);
};

#endif
