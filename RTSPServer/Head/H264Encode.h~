/*
 * H264Encode.h
 *
 *  Created on: 2014年1月4日
 *      Author: ny
 */

#ifndef H264ENCODE_H_
#define H264ENCODE_H_

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>

#include <x264.h>

}

class H264Encode
{
public:
	H264Encode();
	virtual ~H264Encode();
	void x264_init(AVPicture picture, int width, int height);
	void x264_encode();


public:
	x264_nal_t * nals;//存储一帧图像里面的nal单元
	int  nnal;//nal单元的数量
private:
	x264_t * x264EnCoder;
	x264_param_t param;
	x264_picture_t xPic;//输入的图像
	int64_t i_pts;
	x264_picture_t* pPicOut;//编码后的图像

};

#endif /* H264ENCODE_H_ */
