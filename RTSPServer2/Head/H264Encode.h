#ifndef H264ENCODE_H_
#define H264ENCODE_H_

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>

#include <x264.h>
}

class H264Encode{
	public:
		H264Encode();
		virtual ~H264Encode();
		void x264_init(AVPicture picture, int width, int height);
		void x264_encode();

	public:
		x264_nal_t *nals;
		int nnal;
	private:
		x264_t *x264Encoder;//编码器
		x264_param_t param;//编码器的参数
		x264_picture_t xPic;//存储图片信息
		int64_t i_pts;
		x264_picture_t *pPicOut;
};

#endif
