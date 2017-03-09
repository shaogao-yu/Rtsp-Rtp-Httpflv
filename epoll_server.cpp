#include "cstdio" 
#include "cstdlib"
#include "cstring"
#include "cerrno"

extern "C"
{
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
}

#include "epoll_server.h"

EpollServer::EpollServer()
{
    m_listen_port = 1234;
    m_client_max = 5;

    printf("Server Listen Port:%d\r\n",m_listen_port);
    printf("Server Client Max:%d\r\n",m_client_max);
}

EpollServer::EpollServer(int listen_port,int client_max)
{
    m_listen_port = listen_port;
    m_client_max = client_max;

    printf("Server Listen Port:%d\r\n",m_listen_port);
    printf("Server Client Max:%d\r\n",m_client_max);
}

EpollServer::~EpollServer()
{
    
}

void EpollServer::epoll_run(void)
{
    int listenfd;

    listenfd = epoll_socket_bind(INADDR_ANY,m_listen_port);
    epoll_socket_listen(listenfd,m_client_max);
    epoll_do(listenfd);
}

int EpollServer::epoll_socket_bind(char *p_ip,int port)
{
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd == -1)
    {
        printf("Creat Socket fail\r\n");

        return fd;
    }

    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);//htonl使用十六进制地址，如：192.168.1.1为0xC0A80101
    //inet_pton(AF_INET,ip,&addr.sin_addr);
    addr.sin_port = htons(port);

    if(bind(fd,(struct sockaddr*)&addr,sizeof(addr)) == -1)
    {
        printf("Bind Addr fail!!\r\n");
        return -1;
    }

    return fd;
}

int EpollServer::epoll_socket_listen(int listenfd,int listen_max)
{
    if(listen(listenfd,listen_max) == -1)
    {
        printf("Socket Listen Fail\r\n");

        return -1;
    }
    
    return 0;
}

void EpollServer::epoll_do(int listenfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    int ret;

    //创建一个描述符
    epollfd = epoll_create(FDSIZE);
    //添加监听描述符事件
    epoll_add_event(epollfd,listenfd,EPOLLIN);
    for (;;)
    {
        //获取已经准备好的描述符事件
        ret = epoll_wait(epollfd,events,EPOLLEVENTS,-1);
        epoll_handle_events(epollfd,events,ret,listenfd);
    }

    close(epollfd);
}

void EpollServer::epoll_handle_events(int epollfd,struct epoll_event *events,int num,int listenfd)
{
    int i;
    int fd;

    //进行选好遍历
    for (i = 0;i < num;i++)
    {
        fd = events[i].data.fd;
        //根据描述符的类型和事件类型进行处理
        if ((fd == listenfd) &&(events[i].events & EPOLLIN))
        {
            epoll_handle_accpet(epollfd,listenfd);
        }
        else if (events[i].events & EPOLLIN)
        {
            epoll_do_read(epollfd,fd);
        }
        else if (events[i].events & EPOLLOUT)
        {
            epoll_do_write(epollfd,fd);
        }
     }
}

void EpollServer::epoll_handle_accpet(int epollfd,int listenfd)
{
    int clifd;
    struct sockaddr_in cliaddr;
    socklen_t  cliaddrlen = sizeof(cliaddr);
    clifd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen);
    if (clifd == -1)
    {
        perror("accpet error:\r\n");
    }
    else
    {
        printf("accept a new client: %s:%d\r\n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
        //添加一个客户描述符和事件
        epoll_add_event(epollfd,clifd,EPOLLIN);
    }
}

int EpollServer::epoll_recv(int client_fd)
{
    char tmp_buf[MAXSIZE];
    int ret;

    memset(tmp_buf,0x00,MAXSIZE);
    ret = read(client_fd,tmp_buf,MAXSIZE);
    printf("recv message:%s\r\n",tmp_buf);

    write(client_fd,tmp_buf,strlen(tmp_buf));

    if(ret > 0)
    {
        ret = 1;
    }

    return ret;
}

int EpollServer::epoll_send(int client_fd)
{
    int ret = 2;

    return ret;
}

void EpollServer::epoll_close(int client_fd)
{
    
}

void EpollServer::epoll_do_read(int epollfd,int fd)
{
    int ret;

   // nread = read(fd,read_buf,MAXSIZE);
    ret = epoll_recv(fd);

    if (ret == -1)
    {
        perror("read error:\r\n");
        epoll_close(fd);
        close(fd);
        epoll_delete_event(epollfd,fd,EPOLLIN);
    }
    else if (ret == 0)
    {
        fprintf(stderr,"client close.\r\n");
        epoll_close(fd);
        close(fd);
        epoll_delete_event(epollfd,fd,EPOLLIN);
    }
    else if(ret == 1)
    {
        //printf("read message is : %s\r\n",read_buf);
        
        //修改描述符对应的事件，由读改为写
        epoll_modify_event(epollfd,fd,EPOLLIN);
    }
    else if(ret == 2)
    {
        epoll_modify_event(epollfd,fd,EPOLLOUT);
    }
}

void EpollServer::epoll_do_write(int epollfd,int fd)
{
    int ret;

    //ret = write(fd,p_write_buf,write_size);
    ret = epoll_send(fd);

    if (ret == -1)
    {
        perror("write error:\r\n");
        epoll_close(fd);
        close(fd);
        epoll_delete_event(epollfd,fd,EPOLLOUT);
    }
    else if(ret == 1)
    {
        epoll_modify_event(epollfd,fd,EPOLLIN);
    }
    else if(ret == 2)
    {
         epoll_modify_event(epollfd,fd,EPOLLOUT);
    }

    //memset(buf,0,MAXSIZE);
}

void EpollServer::epoll_add_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
}

void EpollServer::epoll_delete_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,&ev);
}

void EpollServer::epoll_modify_event(int epollfd,int fd,int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&ev);
}

