/*
 * V4L2.h
 *
 *  Created on: 2013年12月17日
 *      Author: ny
 */

#ifndef V4L2_H_
#define V4L2_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>

#include <linux/videodev2.h>


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
}

#define CLEAR(x) memset(&(x),0,sizeof(x))

struct buffer
{
	void * start;
	unsigned int length;
};

class V4L2
{
public:
	V4L2();
	virtual ~V4L2();
	bool initDev(const char * devName, int width, int height);//摄像头初始化
	bool startStream();//开始启动摄像头
	bool readFrame(AVPicture & pPictureDes, AVPixelFormat FMT, int widht_des,
			int height_des);//读取一帧图像数据  保存在AVPicture
	int getWidth();
	int getHeight();
	bool setSize(int width, int height);
	bool stopStream();//停止摄像头流化
private:
	bool initMmap();//内存映射
	void getSizeInfo();//获取 摄像头图像尺寸
	int fd;//摄像头设备描述符
	int width;
	int height;
	buffer * buffers;//用内核空间的内存映射到户空间的内存
	v4l2_format fmt;//V4L2的格式信息
};

#endif /* V4L2_H_ */
