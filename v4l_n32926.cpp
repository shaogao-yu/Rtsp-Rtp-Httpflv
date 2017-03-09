#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

extern "C"
{
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev.h>
};

#include "v4l_n32926.h"

V4lN32926::V4lN32926()
{
    m_frame_index = 0;	
    mbuf = MAP_FAILED;
    mfd_video = -1;
    mfd_h264 = -1;
    memset(&m_v4l_mbuf,0x00,sizeof(m_v4l_mbuf));
}

V4lN32926::~V4lN32926()
{
    if(mbuf != MAP_FAILED)
    {
        munmap(mbuf,mbuf_size);
    }

    if(mfd_video > 0)
    {
        close(mfd_video);
    }

    if(mfd_h264 > 0)
    {
        close(mfd_h264);
    }
}

int V4lN32926::v4l_dev_open(const char *p_video_dev,int video_width,int video_height)
{
    struct video_capability v4l_cap;
    struct video_channel v4l_ch;
    struct video_picture v4l_pic;
    struct video_window v4l_win;

    memset(&v4l_cap,0x00,sizeof(v4l_cap));
    memset(&v4l_ch,0x00,sizeof(v4l_ch));
    memset(&v4l_pic,0x00,sizeof(v4l_pic));
    memset(&v4l_win,0x00,sizeof(v4l_win));

    m_width = video_width;
    m_height = video_height;

    mfd_video = open(p_video_dev,O_RDWR | O_NONBLOCK);
    if(mfd_video < 0)
    {
        printf("Error,open %s fail\r\n",p_video_dev);
        return -1;
    }

    if(-1 == ioctl(mfd_video,VIDIOCGCAP,&v4l_cap))
    {
        perror("Error,iotcl VIDIOCGCAP fail:\r\n");
        return -2;
    }
    else
    {
        printf("VIDIOCGCAP name:%s\r\n",v4l_cap.name);
        printf("VIDIOCGCAP type:%d\r\n",v4l_cap.type);
        printf("VIDIOCGCAP maxwidth:%d\r\n",v4l_cap.maxwidth);
        printf("VIDIOCGCAP maxheight:%d\r\n",v4l_cap.maxheight);
        printf("VIDIOCGCAP minwidth:%d\r\n",v4l_cap.minwidth);
        printf("VIDIOCGCAP minheight:%d\r\n",v4l_cap.minheight);
        printf("\r\n");
     }
    //VID_TYPE_CAPTURE
    //VID_TYPE_SCALES
    //VID_TYPE_SUBCAPTURE 可以捕获图像中的子区域
    //640x480

    //设置图像格式为16位的YUV420P
    memset(&v4l_pic,0,sizeof(v4l_pic));
    if(-1 == ioctl(mfd_video,VIDIOCGPICT,&v4l_pic))
    {
        perror("Error,iotcl VIDIOCGPICT fail");
        return -3;
    }
    //VIDEO_PALETTE_YUV422

    v4l_pic.depth = 16;
    v4l_pic.palette = VIDEO_PALETTE_YUV420P;
    //v4l_pic.palette = VIDEO_PALETTE_YUV422;
    if(-1 == ioctl(mfd_video,VIDIOCSPICT,&v4l_pic))
    {
        perror("Error,iotcl VIDIOCSPICT fail");
        return -4;
    }

    memset(&m_v4l_mbuf,0,sizeof(m_v4l_mbuf));
    if(-1 == ioctl(mfd_video,VIDIOCGMBUF,&m_v4l_mbuf))
    {
        perror("Error,iotcl VIDIOCGMBUF fail");
        return -5;
    }
    mbuf_size = m_v4l_mbuf.size;
    mbuf= mmap(0,m_v4l_mbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED,mfd_video, 0);
    if(mbuf == MAP_FAILED)
    {
        printf("Error,mmap fail:%s\r\n",strerror(errno));
        return -6;
    }

    return 0;
}

int V4lN32926::v4l_capture_start(void)
{
    unsigned int u32Capture;
        
    u32Capture = VIDEO_START;
    if(ioctl(mfd_video,VIDIOCCAPTURE, &u32Capture) < 0)
    {   
        printf("Error,VIDIOCCAPTURE fail:%s\r\n",strerror(errno));
            
        return -1; 
    }   

    return 0;
}

int V4lN32926::v4l_capture_stop(void)
{
    unsigned int u32Capture;
        
    u32Capture = VIDEO_STOP;
    if(ioctl(mfd_video,VIDIOCCAPTURE, &u32Capture) < 0)
    {
        printf("Error,VIDIOCCAPTURE fail:%s\r\n",strerror(errno));

        return -1;
    }

    return 0;
}

int V4lN32926::v4l_capture_wait(void)
{
    int try_cnt = 0;
    int result = 0;

    //等待捕获完成
    while(ioctl(mfd_video,VIDIOCSYNC,&m_frame_index) < 0 && \
        (errno == EAGAIN || errno == EINTR))
    {
        usleep(5000);//等待5ms
        try_cnt++;
        if(try_cnt > 200)//超时
        {
            printf("Error,VIDIOCSYNC time out:\r\n",strerror(errno));
            result = -1;
            break;
        }
    }

    return result;
}

//addr_type   0:获取应用层输出YUV芯片的地址
//            1:获取用于H264编码的地址
unsigned int V4lN32926::v4l_capture_get_frame_addr(int addr_type)
{   
    unsigned int addr = NULL;
    struct v4l2_buffer v4l_buff;
    int index;
    
    if(addr_type == 0)
    {   
        addr = (unsigned int)((unsigned int)mbuf + m_v4l_mbuf.offsets[m_frame_index]);
    }
    else
    {
        v4l_buff.index = m_frame_index;
        //使用硬件H264编码
        if(ioctl(mfd_video, VIDIOCGCAPTIME, &v4l_buff) < 0)
        {
            printf("Error,VIDIOCGCAPTIME\r\n");
        }
        else
        {
            addr = v4l_buff.m.userptr;
        }
    }

    return addr;
}

//yuv_type
//0:输出单纯的YUV图片
//1:输出H264编码的图片
int V4lN32926::v4l_capture_frame(int yuv_type)
{
    struct video_mmap v4l_mmap;
    int result = 0;

    memset(&v4l_mmap,0x00,sizeof(v4l_mmap));

    v4l_mmap.frame = m_frame_index;
    if(yuv_type == 0)
    {
        v4l_mmap.format = VIDEO_PALETTE_YUV420P;
    }
    else
    {
        v4l_mmap.format = VIDEO_PALETTE_YUV420P_MACRO; 
    }
    //v4l_mmap.format = VIDEO_PALETTE_YUV420P_MACRO;
    //v4l_mmap.format = VIDEO_PALETTE_YUV420P;
    v4l_mmap.width = m_width;
    v4l_mmap.height = m_height;

    //开始捕获
    if(-1 == ioctl(mfd_video,VIDIOCMCAPTURE,&v4l_mmap))
    {
        printf("Error,VIDIOCMCAPTURE fail:%s\r\n",strerror(errno));
        result = -1;
    }

    return result;
}

void V4lN32926::v4l_capture_next_frame(void)
{
    m_frame_index = (m_frame_index + 1) % m_v4l_mbuf.frames;
}

void V4lN32926::v4l_yuv_sample_handle(char *p_yuv_data,int data_len)
{
    
}

void V4lN32926::v4l_yuv_sample_run(void)
{
	int cap_cnt = 0;
    unsigned int yuv_frame_addr = 0;
    int yuv_size;
    int nwrite;

    v4l_capture_start();
    
	while(1)
	{
		//等待图片捕获完成
        v4l_capture_wait();
        //将系统原先的每秒30帧改成每秒25帧
	    if((cap_cnt % 6) != 0 || cap_cnt == 0)
        {
            printf("m_frame_index = %d\r\n",m_frame_index);

            //获取YUV图片地址
            yuv_frame_addr = v4l_capture_get_frame_addr(0);
            yuv_size = (m_width * m_height * 3) / 2;
            printf("Capture YUV %d,Size:%d\r\n",cap_cnt,yuv_size);

            v4l_yuv_sample_handle((char *)yuv_frame_addr,yuv_size);
        }

        //启动新的捕获，填充刚才已经处理完的缓冲区
        v4l_capture_frame(0);
        //准备读取下一缓冲区的数据
        v4l_capture_next_frame();
 
        cap_cnt++;
	}

    v4l_capture_stop();
}

int V4lN32926::v4l_h264_open(const char *p_h264_dev)
{
    avc_workbuf_t    bit_stream_buf;
//  void *enc_buf = MAP_FAILED;

    mfd_h264 = open(p_h264_dev,O_RDWR);

    if(mfd_h264 < 0)
    {
        printf("Error,open %s fail:%s\r\n",p_h264_dev,strerror(errno));
        return -1;
    }

    // Get Bitstream Buffer Information
    if((ioctl(mfd_h264,FAVC_IOCTL_ENCODE_GET_BSINFO,&bit_stream_buf)) < 0)
    {
        printf("Get avc Enc bitstream info fail\n");
        return -2;
    }
/*
    if(result == TRUE)
    {
        enc_buf = mmap(NULL, bit_stream_buf.size, PROT_READ|PROT_WRITE, MAP_SHARED, pt_h264_context->fp_h264, bit_stream_buf.offset);
        if(enc_buf == MAP_FAILED)
        {
            printf("Map ENC Bitstream Buffer Failed!\n");
            result = FALSE;
        }
    }
*/
    if(v4l_h264_set_param() < 0)
    {
        return -3;
    }

    return 0;
}

int V4lN32926::v4l_h264_set_param(void)
{
    FAVC_ENC_PARAM enc_param_init;

    memset(&enc_param_init, 0, sizeof(FAVC_ENC_PARAM));

    enc_param_init.u32API_version = H264VER;
    enc_param_init.u32FrameWidth = m_width;
    enc_param_init.u32FrameHeight = m_height;
    enc_param_init.fFrameRate = 30;
    enc_param_init.u32IPInterval = 31; //GOP,IPPPP.... I, next I frame interval 
    enc_param_init.u32MaxQuant = 51;
    enc_param_init.u32MinQuant = 0;
#ifdef RATE_CTL // Init Quant
    enc_param_init.u32Quant = 26;//26
#else
    enc_param_init.u32Quant = 0;//26
#endif

    enc_param_init.u32BitRate = 512000;//1048576;//1m rate 1024 * 1024
    enc_param_init.ssp_output = -1;
    enc_param_init.intra = -1;
    enc_param_init.bROIEnable = 0;
    enc_param_init.u32ROIX = 0;
    enc_param_init.u32ROIY = 0;
    enc_param_init.u32ROIWidth = 0;
    enc_param_init.u32ROIHeight = 0;
    m_favc_quant = enc_param_init.u32Quant;

#ifdef RATE_CTL
    memset(&m_h264_ratec, 0, sizeof(H264RateControl));
    H264RateControlInit(&m_h264_ratec,\
        enc_param_init.u32BitRate,\
        RC_DELAY_FACTOR,RC_AVERAGING_PERIOD, \
        RC_BUFFER_SIZE_BITRATE,\
        (int)enc_param_init.fFrameRate,\
        (int) enc_param_init.u32MaxQuant,\
        (int)enc_param_init.u32MinQuant,\
        (unsigned int)enc_param_init.u32Quant,\
        enc_param_init.u32IPInterval);
#endif

    if (ioctl(mfd_h264, FAVC_IOCTL_ENCODE_INIT, &enc_param_init) < 0)
    {
        printf("Handler_1 Error to set FAVC_IOCTL_ENCODE_INIT\r\n");

        return -1;
    }

    return 0;
}

void V4lN32926::v4l_h264_sample_handle(char *p_h264_data,int data_len)
{
    
}

void V4lN32926::v4l_h264_sample_run(void)
{
    int cap_cnt = 0;
    unsigned int enc_frame_addr = 0;
    int enc_frame_size = 0;
    char *p_enc_bitstream;
    int nwrite;

    p_enc_bitstream = new char[(m_width * m_height * 3) / 2];
    
    if(p_enc_bitstream == 0)
    {
        printf("Malloc Encode Buffer Fail\r\n");
        return;
    }

	while(1)
	{
		//等待图片捕获完成
        v4l_capture_wait();
        //将系统原先的每秒30帧改成每秒25帧
        if((cap_cnt % 6) != 0 || cap_cnt == 0)
        {
            //获取YUV图片地址
            enc_frame_addr = v4l_capture_get_frame_addr(1);
            //进行YUV->H264编码
            enc_frame_size = v4l_h264_encode(enc_frame_addr,p_enc_bitstream);

            //printf("Capture H264 %d,Size:%d\r\n",cap_cnt,enc_frame_size);
            
            v4l_h264_sample_handle((char *)p_enc_bitstream,enc_frame_size);
       }

        //启动新的捕获，填充刚才已经处理完的缓冲区
        v4l_capture_frame(1);
        //准备读取下一缓冲区的数据
        v4l_capture_next_frame();
        cap_cnt++;
    }

    delete p_enc_bitstream;

    v4l_capture_stop();
}

int V4lN32926::v4l_h264_encode(unsigned int frame_addr,char *p_encode_addr)
{
    FAVC_ENC_PARAM enc_param_enc;

    memset(&enc_param_enc, 0, sizeof(FAVC_ENC_PARAM));
    enc_param_enc.pu8YFrameBaseAddr = (unsigned char *)frame_addr;
    enc_param_enc.pu8UFrameBaseAddr = enc_param_enc.pu8YFrameBaseAddr + m_width * m_height;
    enc_param_enc.pu8VFrameBaseAddr = enc_param_enc.pu8UFrameBaseAddr + ((m_width * m_height) / 4);
    enc_param_enc.bitstream = p_encode_addr;
    enc_param_enc.ssp_output = -1;
    enc_param_enc.intra = -1;
    enc_param_enc.u32IPInterval = 0; // use default IPInterval that set in INIT
    enc_param_enc.u32Quant = m_favc_quant;
    enc_param_enc.bitstream_size = 0;

    //time_val.tv_sec = 0;
    //time_val.tv_usec = 0;
    //settimeofday(&time_val,NULL);
    if (ioctl(mfd_h264, FAVC_IOCTL_ENCODE_FRAME,&enc_param_enc) < 0)
    {
        printf("Error:FAVC_IOCTL_ENCODE_FRAME Fail\r\n");
        return -1;
    }
    //gettimeofday(&time_val,NULL);
    //printf("Encode Time:%d,%d\r\n",time_val.tv_sec,time_val.tv_usec);
#ifdef RATE_CTL
    if (enc_param_enc.keyframe == 0)
    {
        //printf("%d %d %d\n", enc_param.u32Quant, enc_param.bitstream_size, 0);
        H264RateControlUpdate(&m_h264_ratec, enc_param_enc.u32Quant, enc_param_enc.bitstream_size , 0);
    }
    else
    {
        //printf("%d %d %d\n", enc_param.u32Quant, enc_param.bitstream_size, 1);
        H264RateControlUpdate(&m_h264_ratec, enc_param_enc.u32Quant, enc_param_enc.bitstream_size , 1);
    }
    m_favc_quant = m_h264_ratec.rtn_quant;
    //printf(" m_favc_quant = %d\n",m_favc_quant);
    //H264RateControlUpdate(&m_h264_ratec,enc_param.bitstream_size,enc_param.frame_cost);
#endif

    return enc_param_enc.bitstream_size;
}


