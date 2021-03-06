#include <H264Encode.h>

H264Encode::H264Encode()
{
	i_pts = 0;
	x264EnCoder = NULL;
	pPicOut = NULL;
	nnal=0;
	nals=NULL;
}

H264Encode::~H264Encode()
{
}

void H264Encode::x264_init(AVPicture picture, int width, int height)
{
	x264_param_default_preset(&param, "veryfast", "zerolatency");

	param.i_width = width;
	param.i_height = height;
	param.i_fps_num = 15;//帧率分子
	param.i_fps_den = 1;//帧率分母

	param.i_keyint_max = 25;/* Force an IDR keyframe at this interval */
	param.b_intra_refresh = 1;// 是否使用周期帧内刷新替代IDR帧(Periodic Intra Refresh)

	param.b_annexb = 1;/* if set, place start codes (4 bytes) before NAL units,otherwise place size (4 bytes) before NAL units. */

	x264_param_apply_profile(&param, "baseline");
	x264EnCoder = x264_encoder_open(&param);

	x264_picture_alloc(&xPic, X264_CSP_I420, width, height);

	xPic.img.plane[0] = picture.data[0];
	xPic.img.plane[1] = picture.data[1];
	xPic.img.plane[2] = picture.data[2];
	pPicOut = new x264_picture_t;
}

void H264Encode::x264_encode()
{
	xPic.i_pts = i_pts++;
	x264_encoder_encode(x264EnCoder, &nals, &nnal, &xPic, pPicOut);	
}
