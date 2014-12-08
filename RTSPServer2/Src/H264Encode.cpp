#include <H264Encode.h>

H264Encode::H264Encode(){
	i_pts = 0;
	x264Encoder = NULL;
	pPicOut = NULL;
	nnal = 0;
	nals = NULL;
}

H264Encode::~H264Encode(){
}

void H264Encode::x264_init(AVPicture picture, int width, int height){
	x264_param_default_preset(&param, "veryfast", "zerolatency");//即时编码
	param.i_width = width;
	param.i_height = height;
	param.i_fps_num = 25;
	param.i_fps_den = 1;

	param.i_keyint_max = 25;//在此间隔设置IDR关键帧
	param.b_intra_refresh = 1;

	param.b_annexb = 1;

	x264_param_apply_profile(&param, "baseline");//基本档次
	x264Encoder = x264_encoder_open(&param);

	x264_picture_alloc(&xPic, X264_CSP_I420, width, height);

	xPic.img.plane[0] = picture.data[0];
	xPic.img.plane[1] = picture.data[1];
	xPic.img.plane[2] = picture.data[2];

	pPicOut = new x264_picture_t;
}

//一帧图像编码后可能生成几个nal，nals是一个nal的指针，nnal表示这帧数据有几个nal。而nal里面保存了数据信息。
void H264Encode::x264_encode(){
	xPic.i_pts = i_pts++;
	x264_encoder_encode(x264Encoder, &nals, &nnal, &xPic, pPicOut);
}


