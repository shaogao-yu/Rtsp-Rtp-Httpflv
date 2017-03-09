#include <cstdlib>
#include <cstdio>
#include <cstring>

extern "C"
{
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>

#include <sys/stat.h>
}

#include "http_server.h"

HttpServer::HttpServer()
{
    
}

HttpServer::HttpServer(int port,int client_max):EpollServer(port,client_max)
{

}

HttpServer::~HttpServer()
{
    
}

void HttpServer::get_res_type(char *p_type,char *p_con_type)
{
    printf("res type:%s\r\n",p_type);

    if(strncmp(p_type,"html",strlen("html")) == 0)
    {
        strcpy(p_con_type,"text/html");
    }
    else if(strncmp(p_type,"js",strlen("js")) == 0)
    {
        strcpy(p_con_type,"application/x-javascript");
    }
    else if(strncmp(p_type,"swf",strlen("swf")) == 0)
    {
        strcpy(p_con_type,"application/x-shockwave-flash");
    }
    else if(strncmp(p_type,"txt",strlen("txt")) == 0)
    {
        strcpy(p_con_type,"text/plain");
    }
    else if(strncmp(p_type,"flv",strlen("flv")) == 0)
    {
        strcpy(p_con_type,"video/x-flv");
    }
}

int HttpServer::send_file_hook_handle(int client_fd,char *p_file)
{
    return 0;
}

int HttpServer::send_file(int client_fd,char *p_file)
{
    FILE *fp;
    struct stat statbuff;
    int file_size;
    char *p_restype;
    char tmp_buf[500];
    char con_type[50];
    int ret;

    printf("send file:%s\r\n",p_file);

    //预先特殊处理特定文件如：FLV
    ret = send_file_hook_handle(client_fd,p_file);
    if(ret != 0)
    {
        return 1;
    }

    fp = fopen(p_file,"r");
    if(fp == NULL)
    {
        perror("open file fail:");
        return 1;
    }

    if(stat(p_file, &statbuff) < 0)
    {
        printf("get file size fail!!\r\n");
        return 1;
    }
    else
    {
       file_size = statbuff.st_size;
    }

    p_restype = strstr(p_file,".");
    p_restype++;

    memset(con_type,0x00,50);
    get_res_type(p_restype,con_type);

    sprintf(tmp_buf,\
    "HTTP/1.1 200 OK\r\n"\
    "Server: HttpServer\r\n"\
    "Cache-Control: no-cache\r\n"\
    "Pragma: no-cache\r\n"\
    "Connection: close\r\n"\
    "Access-Control-Allow-Origin: *\r\n"\
    "Content-Type: %s\r\n"\
    "Content-Length: %d\r\n\r\n",\
    con_type,\
    file_size);

    printf("%s",tmp_buf);

    char *p_send_data = new char[strlen(tmp_buf) + file_size];

    strcpy(p_send_data,tmp_buf);
    fread(p_send_data + strlen(tmp_buf),file_size,1,fp);
    write(client_fd,p_send_data,strlen(tmp_buf) + file_size);

    delete p_send_data;

    return 1;
}

int HttpServer::epoll_recv(int client_fd)
{
    char recv_buf[MAXSIZE];
    int nread;
    int ret;
    char *p_tmpchar;

    memset(recv_buf,0x00,MAXSIZE);
    nread = read(client_fd,recv_buf,MAXSIZE);
    ret = nread;

    printf("http server recv data:\r\n%s\r\n",recv_buf);

    if(0 == strncmp(recv_buf,"GET ",strlen("GET ")))
    {
        p_tmpchar = strstr(&recv_buf[4]," ");
        *p_tmpchar = '\0';

        printf("request url:%s\r\n",&recv_buf[4]);

        if(recv_buf[4] == '/' && recv_buf[5] == 0x00)
        {
            ret = send_file(client_fd,(char *)&"index.html");
        }
        else
        {
            ret = send_file(client_fd,&recv_buf[5]);
        }
    }

    return ret;
}

int HttpServer::client_send(int client_fd)
{
    
}

void HttpServer::http_run(void)
{
    epoll_run();
}

