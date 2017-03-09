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

#include "rtp_server.h"

RtpServer::RtpServer(int cli_max):m_client_max(cli_max)
{
    mpt_rtp_client = new t_rtp_client_def[m_client_max];

    memset(mpt_rtp_client,0x00,sizeof(t_rtp_client_def) * m_client_max);

    m_MTU_value = 1500;
    mp_rtp_buff = new char[m_MTU_value];

    m_rtp_ssrc = 123;
    m_rtp_timestamp_inc = (unsigned int)(90000.0 / 25.0);

    m_rtp_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

RtpServer::~RtpServer(void)
{
    delete mpt_rtp_client;
}

void RtpServer::rtp_setup_client(int fd,char *ip,int port)
{
    int i;

    for(i = 0;i < m_client_max;i++)
    {
        if(mpt_rtp_client[i].ip[0] == 0x00)
        {
            mpt_rtp_client[i].fd = fd;
            strcpy(mpt_rtp_client[i].ip,ip);
            mpt_rtp_client[i].port = port;
    		mpt_rtp_client[i].addr.sin_family = AF_INET;
    		mpt_rtp_client[i].addr.sin_port = htons(port);
    		mpt_rtp_client[i].addr.sin_addr.s_addr = inet_addr(ip);//INADDR_ANY;  
    		bzero(&(mpt_rtp_client[i].addr.sin_zero),8);
            mpt_rtp_client[i].seq_num = 0;
            mpt_rtp_client[i].timestamp = 0;
            mpt_rtp_client[i].play = 0;
            break;
        }
    }
}

void RtpServer::rtp_close_client(int fd)
{
    int i;

    for(i = 0;i < m_client_max;i++)
    {
         if(fd == mpt_rtp_client[i].fd)
         {
            memset(mpt_rtp_client[i].ip,0x00,20);
            mpt_rtp_client[i].port = 0;
            mpt_rtp_client[i].fd = 0;
            mpt_rtp_client[i].play = 0;
         }
    }
}

void RtpServer::rtp_play_client(int fd)
{
    int i;

    for(i = 0;i < m_client_max;i++)
    {
        if(fd == mpt_rtp_client[i].fd)
        {
            mpt_rtp_client[i].play = 1;
        }
    }
}

int RtpServer::rtp_send_h264_nalu(char *p_nalu,int nalu_size,t_rtp_client_def *pt_rtp_client)//struct sockaddr_in *p_cli_addr)
{
    int payload_max = 0;
    int packet_num = 0;
    int send_size = 0;
    int have_send = 0;
    int i;
    t_rtp_header_def *pt_rtp_header;
    t_FU_indicator_def *pt_FU_indicator;
    t_FU_header_def *pt_FU_header;

    payload_max = m_MTU_value - 200;
    //小于MTU，一包一个NALU
    if(nalu_size <= payload_max)
    {
        //RTP头序号加1
        pt_rtp_client->seq_num++;
        //时间戳添加
        pt_rtp_client->timestamp += m_rtp_timestamp_inc;

        //RTP_Header + NALU

        //设置RTP头
        pt_rtp_header = (t_rtp_header_def *)&mp_rtp_buff[0];
        pt_rtp_header->v = 2;//添加版本
        pt_rtp_header->p = 0;
        pt_rtp_header->x = 0;
        pt_rtp_header->cc = 0;
        pt_rtp_header->m = 1;//最后一片
        pt_rtp_header->pt = 96;//H264类型
        pt_rtp_header->sequence_number = htons(pt_rtp_client->seq_num);
        pt_rtp_header->timestamp = htonl(pt_rtp_client->timestamp);
        pt_rtp_header->ssrc = m_rtp_ssrc;
        //设置NALU
        memcpy(&mp_rtp_buff[12],p_nalu,nalu_size);
       //发送数据
        sendto(m_rtp_fd,mp_rtp_buff,12 + nalu_size, \
            0,(const struct sockaddr*)&pt_rtp_client->addr, sizeof(pt_rtp_client->addr));

        printf("Send1 To:%s,%d,%d,%d,%d\r\n",pt_rtp_client->ip,pt_rtp_client->port,pt_rtp_client->seq_num,pt_rtp_client->timestamp,nalu_size);
    }
    //进行分包处理
    else
    {
        have_send = 1;
        payload_max -= 2;

        //时间戳添加
        pt_rtp_client->timestamp += m_rtp_timestamp_inc;

        while(have_send != nalu_size)
        {
            //RTP头序号加1
            pt_rtp_client->seq_num++;

            if(nalu_size - have_send >= payload_max)
            {
                send_size = payload_max;
            }
            else
            {
                send_size = nalu_size - have_send;
            }

            //设置RTP头
            pt_rtp_header = (t_rtp_header_def *)&mp_rtp_buff[0];
            pt_rtp_header->v = 2;
            pt_rtp_header->p = 0;
            pt_rtp_header->x = 0;
            pt_rtp_header->cc = 0;
            if((have_send + send_size) == nalu_size)//最后一片
            {
                pt_rtp_header->m = 1;
            }
            else
            {
                pt_rtp_header->m = 0;
            }
            pt_rtp_header->pt = 96;//H264类型
            pt_rtp_header->sequence_number = htons(pt_rtp_client->seq_num);
            pt_rtp_header->timestamp = htonl(pt_rtp_client->timestamp);
            pt_rtp_header->ssrc = m_rtp_ssrc;

            //设置FU indicator
            mp_rtp_buff[12] = p_nalu[0];
            pt_FU_indicator = (t_FU_indicator_def *)&mp_rtp_buff[12];
            pt_FU_indicator->Type = 28;//使用FU-A

            //设置FU header
            mp_rtp_buff[13] = p_nalu[0];
            pt_FU_header = (t_FU_header_def *)&mp_rtp_buff[13];
            if(have_send < send_size)//第一片
            {
                pt_FU_header->S = 1;
            }
            else
            {
                pt_FU_header->S = 0;
            }

            if((have_send + send_size) == nalu_size)//最后一片
            {
                pt_FU_header->E= 1;
            }
            else
            {
                pt_FU_header->E = 0;
            }
            pt_FU_header->R = 0;

            //设置NALU
            memcpy(&mp_rtp_buff[14],&p_nalu[have_send],send_size);
            //发送数据
            sendto(m_rtp_fd,mp_rtp_buff,14 + send_size, \
                0,(const struct sockaddr*)&pt_rtp_client->addr, sizeof(pt_rtp_client->addr));
            printf("Send2 To:%s,%d,%d,%d,%d\r\n",pt_rtp_client->ip,pt_rtp_client->port,pt_rtp_client->seq_num,pt_rtp_client->timestamp,send_size);
            have_send += send_size;
        }
    }

    return 0;
}

void RtpServer::rtp_run(void)
{
    int i;
    char *p_data = NULL;
    int data_len;
    char sps_data[9];
    int sps_len;
    char pps_data[4];
    int pps_len;

    while(1)
    {
        p_data = list_fifo_pop(&data_len); 
        if(p_data != NULL)
        {
            printf("Fifo Pop:%08x,%d\r\n",p_data,data_len);

            for(i = 0;i < m_client_max;i++)
            {
                if(mpt_rtp_client[i].play == 1)
                {
                     //接收到第一帧
                    if(p_data[0] == 0x67)
                    {
                        memcpy(sps_data,p_data,9);
                        sps_len = 9;
                        memcpy(pps_data,&p_data[13],4);
                        pps_len = 4;
        
                        rtp_send_h264_nalu(sps_data,9,&mpt_rtp_client[i]);
                        rtp_send_h264_nalu(pps_data,4,&mpt_rtp_client[i]);
                        rtp_send_h264_nalu(&p_data[21],data_len - 21,&mpt_rtp_client[i]);
                    }
        			else
                    {
                        rtp_send_h264_nalu(p_data,data_len,&mpt_rtp_client[i]);
                    }
                   
                }
            }

            free(p_data);
        }
        else
        {
            usleep(100000);
        }
    }
}

