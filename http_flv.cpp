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
#include "fifo_list.h"
#include "flv_mux.h"
}

#include "http_flv.h"

HttpFlv::HttpFlv()
{
    
}

HttpFlv::HttpFlv(int port,int client_max,int width,int height):HttpServer(port,client_max)
{
    m_video_width = width;
    m_video_height = height;
    m_client_max = client_max;

    mp_sps = NULL;
    mp_pps = NULL;
    
    mpt_flv_client = new t_flv_client_def[m_client_max];
    memset(mpt_flv_client,0x00,sizeof(t_flv_client_def) * m_client_max);
}

HttpFlv::~HttpFlv()
{
    if(mp_sps != NULL)
    {
        delete mp_sps;
    }
    if(mp_pps != NULL)
    {
        delete mp_pps;
    }
    if(mpt_flv_client != NULL)
    {
        delete mpt_flv_client;
    }
}

void HttpFlv::set_h264_pps_sps(char *p_sps,int sps_len,char *p_pps,int pps_len)
{
    mp_sps = new char[sps_len];
    memcpy(mp_sps,p_sps,sps_len);

    mp_pps = new char[pps_len];
    memcpy(mp_pps,p_pps,pps_len);
}

void HttpFlv::add_flv_client(int fd)
{
    int i;

    for(i = 0;i < m_client_max;i++)
    {
        if(mpt_flv_client[i].fd == 0x00)
        {
            mpt_flv_client[i].fd = fd;
            break;
        }
    }
}

void HttpFlv::del_flv_client(int fd)
{
     int i;

    for(i = 0;i < m_client_max;i++)
    {
        if(mpt_flv_client[i].fd == fd)
        {
            mpt_flv_client[i].fd = 0x00;
            break;
        }
    }
}

void HttpFlv::send_flv_stream_proc(void)
{
    char *p_data;
    int data_len;
	int i;
    char tmp_data[m_video_width * m_video_height + 100];
    int tmp_len1;
    int tmp_len2;
    int timestamp = 0;
    int pos;

    while(1)
    {
        p_data = list_fifo_pop(&data_len); 
        if(p_data != NULL)
        {
            for(i = 0;i < m_client_max;i++)
            {
                if(mpt_flv_client[i].fd != 0x00)
                {
                    //video tag avc data
                    if(p_data[0] == 0x65)
                    {   
                        tmp_len1 = flv_make_tag_avc_header(&tmp_data[0],1,AVC_NALU);
                    }   
                    else
                    {   
                        tmp_len1 = flv_make_tag_avc_header(&tmp_data[0],0,AVC_NALU);
                    }   
                    tmp_len1 += flv_make_tag_avc_data(&tmp_data[tmp_len1],&p_data[0],data_len);
                    timestamp += 40; 
                    pos = flv_make_tag_header(&tmp_data[0],VIDEO,tmp_len1,timestamp);
                    if(p_data[0] == 0x65)
                    {   
                        tmp_len1 = flv_make_tag_avc_header(&tmp_data[pos],1,AVC_NALU);
                    }   
                    else
                    {   
                        tmp_len1 = flv_make_tag_avc_header(&tmp_data[pos],0,AVC_NALU);
                    }   
                    tmp_len1 += flv_make_tag_avc_data(&tmp_data[tmp_len1],&p_data[0],data_len);
                    pos += tmp_len1;

                    write(mpt_flv_client[i].fd,tmp_data,pos);
                }
            }

            free(p_data);
        }
    }
}

int HttpFlv::send_file_hook_handle(int client_fd,char *p_file)
{
    int file_len;
	char tmpdata[1024];
	int pos;
    int tmp_len1;
    int tmp_len2;

    file_len = strlen(p_file);

    //特殊处理FLV文件 
    if(p_file[file_len - 4] == '.' && p_file[file_len - 3] == 'f' && p_file[file_len - 2] == 'l' && p_file[file_len - 1] == 'v')
    {
        //flv header
        tmp_len1 = flv_make_header(1,0,&tmpdata[0]);
        pos = tmp_len1;

        //pre tag size
        tmp_len1 = flv_make_pre_tag_size(&tmpdata[pos],0);
        pos += tmp_len1;

        //script tag header
        tmp_len1 = flv_make_tag_script_data(1,m_video_width,m_video_height,25,0,44100,&tmpdata[pos]);
        tmp_len2 = flv_make_tag_header(&tmpdata[pos],SCRIT_DATA,tmp_len1,0);
        pos += tmp_len2;
        tmp_len1 = flv_make_tag_script_data(1,m_video_width,m_video_height,25,0,44100,&tmpdata[pos]);
        pos += tmp_len1;

        //pre tag size
        tmp_len1 = flv_make_pre_tag_size(&tmpdata[pos],tmp_len1 + tmp_len2);
        pos += tmp_len1;

        //video tag avc header
        tmp_len1 = flv_make_tag_avc_header(&tmpdata[pos],1,AVC_sequence_header);
        tmp_len1 += flv_make_tag_avc_seq_header(&tmpdata[pos + tmp_len1],mp_sps,m_sps_len,mp_pps,m_pps_len);
        tmp_len2 = flv_make_tag_header(&tmpdata[pos],VIDEO,tmp_len1,0);
        pos += tmp_len2;
        tmp_len1 = flv_make_tag_avc_header(&tmpdata[pos],1,AVC_sequence_header);
        pos += tmp_len1;
        tmp_len1 += flv_make_tag_avc_seq_header(&tmpdata[pos],mp_sps,m_sps_len,mp_pps,m_pps_len);
        pos += tmp_len1;

        //pre tag size
        tmp_len1 = flv_make_pre_tag_size(&tmpdata[pos],tmp_len1 + tmp_len2);
        pos += tmp_len1;

        write(client_fd,tmpdata,pos);

        add_flv_client(client_fd);
    }

    return 1;
}

void HttpFlv::run(void)
{
    list_fifo_init();    
}

