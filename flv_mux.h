#ifndef __FLV_MUX_H__
#define __FLV_MUX_H__

typedef enum
{   
    AMF_DATA_TYPE_NUMBER      = 0x00,
    AMF_DATA_TYPE_BOOL        = 0x01,
    AMF_DATA_TYPE_STRING      = 0x02,
    AMF_DATA_TYPE_OBJECT      = 0x03,
    AMF_DATA_TYPE_NULL        = 0x05,
    AMF_DATA_TYPE_UNDEFINED   = 0x06,
    AMF_DATA_TYPE_REFERENCE   = 0x07,
    AMF_DATA_TYPE_MIXEDARRAY  = 0x08,
    AMF_DATA_TYPE_OBJECT_END  = 0x09,
    AMF_DATA_TYPE_ARRAY       = 0x0a,
    AMF_DATA_TYPE_DATE        = 0x0b,
    AMF_DATA_TYPE_LONG_STRING = 0x0c,
    AMF_DATA_TYPE_UNSUPPORTED = 0x0d,
} AMFDataType;

typedef enum
{
    AVC_sequence_header = 0x00,
    AVC_NALU            = 0x01,
    AVC_end_of_sequence = 0x02,
}AVCPacketType;

typedef enum
{
    AUDIO       = 8,
    VIDEO       = 9,
    SCRIT_DATA  = 18,
}TagType;

typedef union
{
    unsigned int i;
    float    f;
}intfloat32;

typedef union
{
    unsigned long int i;
    double   f;
}intfloat64;

int flv_write_8(char *p_buf,char data);
int flv_write_16(char *p_buf,int data);
int flv_write_24(char *p_buf,int data);
int flv_write_32(char *p_buf,int data);
int flv_write_double(char *p_buf,double data);
int flv_write_amf_string(char *p_buf,const char *p_str);
int flv_make_header(int video_support,int audio_support,char *p_buf);
int flv_make_pre_tag_size(char *p_buf,int datasize);
int flv_make_tag_header(char *p_buf,TagType tag_type,int datasize,int timestamp);
int flv_make_tag_script_data(int video_support,int video_width,int video_height,int video_fps,int audio_support,int audio_samplerate,char *p_buf);
int flv_make_tag_avc_header(char *p_buf,int keyframe,AVCPacketType PacketType);
int flv_make_tag_avc_seq_header(char *p_buf,char *p_sps,int sps_size,char *p_pps,int pps_size);
int flv_make_tag_avc_data(char *p_buf,char *p_data,int data_len);

#endif

