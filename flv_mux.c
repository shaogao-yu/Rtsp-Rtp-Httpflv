#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "flv_mux.h"

//返回已使用的长度
int flv_make_header(int video_support,int audio_support,char *p_buf)
{
    p_buf[0] = 'F';
    p_buf[1] = 'L';
    p_buf[2] = 'V';
 
    p_buf[3] = 0X01;//版本

    //bit0 视频,bit2 音频
    p_buf[4] = 0x00;
    if(video_support)
    {
        p_buf[4] |= 0X01;
    }
    if(audio_support)
    {
        p_buf[4] |= 0X04;
    }

    //头的长度3 + 1 + 1 +4 = 9,FLV使用大端
    p_buf[5] = 0X00;
    p_buf[6] = 0X00;
    p_buf[7] = 0X00;
    p_buf[8] = 0X09;

    return 9;
}

//返回已使用的长度
int flv_write_8(char *p_buf,char data)
{
    p_buf[0] = data;
    return 1;
}

//返回已使用的长度
int flv_write_16(char *p_buf,int data)
{
    //16位的长度
    p_buf[0] = data >> 8;
    p_buf[1] = data;
    
    return 2;
}

//返回已使用的长度
int flv_write_24(char *p_buf,int data)
{
    //16位的长度
    p_buf[0] = data >> 16;
    p_buf[1] = data >> 8;
    p_buf[2] = data;
    
    return 3;
}


//返回已使用的长度
int flv_write_32(char *p_buf,int data)
{
     //16位的长度
    p_buf[0] = data >> 24;
    p_buf[1] = data >> 16;;
    p_buf[2] = data >> 8;;
    p_buf[3] = data;
    
    return 4;
}

//返回已使用的长度
int flv_write_double(char *p_buf,double data)
{
    intfloat64 v;

    v.f = data;

    flv_write_32(p_buf,v.i >> 4);
    flv_write_32(p_buf + 4,v.i);
    
    return 8;
}

//返回已使用的长度
int flv_write_amf_string(char *p_buf,const char *p_str)
{
    int str_len;

    str_len = strlen(p_str);
 
    //16位的长度
    flv_write_16(p_buf,str_len);
    strcpy(&p_buf[2],p_str);

    return str_len + 2;
}

//返回已使用的长度
int flv_make_tag_script_data(int video_support,int video_width,int video_height,int video_fps,int audio_support,int audio_samplerate,char *p_buf)
{
    int pos = 0;
    int array_num = 0;
    
    //固定的0x02类型
    p_buf[pos] = AMF_DATA_TYPE_STRING;
    pos++;
    pos += flv_write_amf_string(&p_buf[pos],"onMetaData");
    
    //构建数组类型--------------
    p_buf[pos] = AMF_DATA_TYPE_MIXEDARRAY;
    pos++;
    
    array_num = 1;
    if(video_support)
    {
        array_num += 3;
    }
    if(audio_support)
    {
        array_num += 1;
    }
    pos += flv_write_32(&p_buf[pos],array_num);
    
    //写数组参数
    pos += flv_write_amf_string(&p_buf[pos],"duration");
    p_buf[pos] = AMF_DATA_TYPE_NUMBER;
    pos++;
    pos += flv_write_double(&p_buf[pos],0.0);

    if (video_support)
    {
        pos += flv_write_amf_string(&p_buf[pos],"width");
        p_buf[pos] = AMF_DATA_TYPE_NUMBER;
        pos++;
        pos += flv_write_double(&p_buf[pos],video_width);

        pos += flv_write_amf_string(&p_buf[pos],"height");
        p_buf[pos] = AMF_DATA_TYPE_NUMBER;
        pos++;
        pos += flv_write_double(&p_buf[pos],video_height);

        pos += flv_write_amf_string(&p_buf[pos],"framerate");
        p_buf[pos] = AMF_DATA_TYPE_NUMBER;
        pos++;
        pos += flv_write_double(&p_buf[pos],video_fps);
    }
    if (audio_support)
    {
        pos += flv_write_amf_string(&p_buf[pos],"audiosamplerate");
        p_buf[pos] = AMF_DATA_TYPE_NUMBER;
        pos++;
        pos += flv_write_double(&p_buf[pos],audio_samplerate);
    }

    pos += flv_write_amf_string(&p_buf[pos],"");
	p_buf[pos] =  AMF_DATA_TYPE_OBJECT_END;
    pos++;
    
    return pos;
}

//返回已使用的长度
int flv_make_pre_tag_size(char *p_buf,int datasize)
{
    flv_write_32(p_buf,datasize);
    return 4;
}

//返回已使用的长度
int flv_make_tag_header(char *p_buf,TagType tag_type,int datasize,int timestamp)
{
    int pos = 0;
    
    p_buf[pos] = tag_type;
    pos++;
    pos += flv_write_24(&p_buf[pos],datasize);
    pos += flv_write_24(&p_buf[pos],timestamp);
    p_buf[pos] = 0x00;
    pos++;
    pos += flv_write_24(&p_buf[pos],0);

    return pos;
}

//返回已使用的长度
int flv_make_tag_avc_header(char *p_buf,int keyframe,AVCPacketType PacketType)
{
    int pos = 0;
    
    //keyframe
    if(keyframe)
    {
        p_buf[0] = 0x17;
    }
    else
    {
        p_buf[0] = 0x27;
    }
    pos++;

    p_buf[1] = PacketType;
    pos++;

    pos += flv_write_24(&p_buf[pos],0);

    return pos;
}

//返回已使用的长度
int flv_make_tag_avc_seq_header(char *p_buf,char *p_sps,int sps_size,char *p_pps,int pps_size)
{
    int pos = 0;

    //AVCDecoderConfigurationRecord ISO-14496-15 AVC file format
    p_buf[pos] = 0x01;//configurationVersion
    pos++;
    //AVCProfileIndication + profile_compatibility + AVCLevelIndication
    memcpy(&p_buf[pos],&p_sps[1],3);
    pos += 3;
    p_buf[pos] = 0xff;//1+ 0xff 3,指数NALU长度数据字节数，NALU长度数据为四个字节
    pos++;
    //numOfSequenceParameterSets sps个数
    p_buf[pos] = 0xe1;
    pos++;
    //sps数据长度
    pos += flv_write_16(&p_buf[pos],sps_size);
    //sps数据
    memcpy(&p_buf[pos],p_sps,sps_size);
    pos += sps_size;
    
    // pps个数
    p_buf[pos] = 0x01;
    pos++;
    //sps数据长度
    pos += flv_write_16(&p_buf[pos],pps_size);
    //sps数据
    memcpy(&p_buf[pos],p_pps,pps_size);
    pos += pps_size;
    
    return pos;
}

//返回已使用的长度
int flv_make_tag_avc_data(char *p_buf,char *p_data,int data_len)
{
    int pos = 0;
    
    pos += flv_write_32(&p_buf[pos],data_len);
    
    memcpy(&p_buf[pos],p_data,data_len);
    pos += data_len;

    return pos;
}

