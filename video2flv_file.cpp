#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "video2flv_file.h"

Video2FlvFile::Video2FlvFile(const char *p_flv_file)
{
    mfp_flv = fopen(p_flv_file,"w+");

    if(mfp_flv == NULL)
    {
        printf("Open Flv File Fail:%s\r\n",p_flv_file);
    }

}

Video2FlvFile::~Video2FlvFile()
{
    fclose(mfp_flv);

}

void Video2FlvFile::v4l_h264_sample_handle(char *p_h264_data,int data_len)
{
    char sps_data[9];
    int sps_len;
    char pps_data[4];
    int pps_len;
    int pos1,pos2;
	char *p_data;
    static int test_cnt = 0;
    int nalu_valid;
    static char *p_flv_buf = NULL;
    static char *p_tmp_buf = NULL;
    static int timestamp = 0;

    if(p_flv_buf == NULL)
    {
        p_flv_buf = new char[(m_width * m_height * 3) / 2];
    }
    if(p_tmp_buf == NULL)
    {
        p_tmp_buf = new char[(m_width * m_height * 3) / 2];
    }

    //检测NALU是否有效
    nalu_valid = 0;
    if(p_h264_data[0] == 0x00 && p_h264_data[1] == 0x00 && \
        p_h264_data[2] == 0x00 && p_h264_data[3] == 0x01)
    {   
        nalu_valid = 4;
    }   
    else if(p_h264_data[0] == 0x00 && p_h264_data[1] == 0x00 && \
        p_h264_data[2] == 0x01)
    {   
        nalu_valid = 3;
    } 

    if(!nalu_valid)
    {
        return;
    }

    p_data = &p_h264_data[nalu_valid];
    data_len -= nalu_valid;

    //接收到第一帧
    if(p_data[0] == 0x67)
    {
        memcpy(sps_data,p_data,9);
        sps_len = 9;
        memcpy(pps_data,&p_data[13],4);
        pps_len = 4;

        //flv header
        pos1 = flv_make_header(1,0,&p_flv_buf[0]);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        //pre tag size
        pos1 = flv_make_pre_tag_size(&p_flv_buf[0],0);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        //script tag header
        pos2 = flv_make_tag_script_data(1,m_width,m_height,25,0,44100,&p_tmp_buf[0]);
        pos1 = flv_make_tag_header(&p_flv_buf[0],SCRIT_DATA,pos2,0);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        fwrite(&p_tmp_buf[0],pos2,1,mfp_flv);
        //pre tag size
        pos1 = flv_make_pre_tag_size(&p_flv_buf[0],pos1 + pos2);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        //video tag avc header
        pos2 = flv_make_tag_avc_header(&p_tmp_buf[0],1,AVC_sequence_header);
        pos2 += flv_make_tag_avc_seq_header(&p_tmp_buf[pos2],sps_data,sps_len,pps_data,pps_len);
        pos1 = flv_make_tag_header(&p_flv_buf[0],VIDEO,pos2,0);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        fwrite(&p_tmp_buf[0],pos2,1,mfp_flv);
        //pre tag size
        pos1 = flv_make_pre_tag_size(&p_flv_buf[0],pos1 + pos2);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        //video tag avc data
        pos2 = flv_make_tag_avc_header(&p_tmp_buf[0],1,AVC_NALU);
        pos2 += flv_make_tag_avc_data(&p_tmp_buf[pos2],&p_data[21],data_len - 21);
        pos1 = flv_make_tag_header(&p_flv_buf[0],VIDEO,pos2,0);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        fwrite(&p_tmp_buf[0],pos2,1,mfp_flv);
    }
    else
    {
        //pre tag size
        pos1 = flv_make_pre_tag_size(&p_flv_buf[0],pos1 + pos2);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        //video tag avc data
        if(p_tmp_buf[0] == 0x65)
        {
            pos2 = flv_make_tag_avc_header(&p_tmp_buf[0],1,AVC_NALU);
        }
        else
        {
            pos2 = flv_make_tag_avc_header(&p_tmp_buf[0],0,AVC_NALU);
        }
        pos2 += flv_make_tag_avc_data(&p_tmp_buf[pos2],&p_data[0],data_len);
        timestamp += 40;
        pos1 = flv_make_tag_header(&p_flv_buf[0],VIDEO,pos2,timestamp);
        fwrite(&p_flv_buf[0],pos1,1,mfp_flv);
        fwrite(&p_tmp_buf[0],pos2,1,mfp_flv);

        test_cnt++;
        if(test_cnt > 100)
        {
            fclose(mfp_flv);
            exit(1);
        }

    }
}

