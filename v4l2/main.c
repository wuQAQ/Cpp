#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/videodev2.h>

int main(void)
{    
    struct v4l2_capability cap;  
    int fd = open("/dev/video0", O_RDWR | O_NONBLOCK);
    int ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);

    printf("DriverName:%s\nCard Name:%s\nBus info:%s\nDriverVersion:%u.%u.%u\n",
        cap.driver,cap.card,cap.bus_info,
        (cap.version>>16)&0XFF,(cap.version>>8)&0XFF,
        cap.version&0XFF);  

    struct v4l2_fmtdesc fmtdesc;  
    fmtdesc.index=0;  
    fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    printf("Supportformat:\n");  
    while (ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc)!=-1)  
    {  
        printf("\t%d.%s\n", fmtdesc.index+1, fmtdesc.description);  
        fmtdesc.index++;  
    }  

    struct v4l2_format fmt;  
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;  
    if(ioctl(fd,VIDIOC_TRY_FMT,&fmt)==-1)  
        if(errno==EINVAL)  
            printf("notsupport format RGB32!\n");  

    while (getchar() != '\n')
    {

    }

    return 0;
}