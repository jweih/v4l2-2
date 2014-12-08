/*
 * V4L2FramedSource.cpp
 *
 *  Created on: 2014年1月4日
 *      Author: ny
 */

#include <V4L2FramedSource.h>

int V4L2FramedSource::nalIndex = 0;

V4L2FramedSource::V4L2FramedSource(UsageEnvironment & env) :
		FramedSource(env)
{
	output_bufsize = 128 * 1024;
	outbuf = malloc(output_bufsize);

	v4l2 = new V4L2();
	pEncode = new H264Encode();
	mp_token = NULL;
	m_started = 0;
	printf("creater\n");
	v4l2->initDev("/dev/video0", 320, 240);
	avpicture_alloc(&Picture, PIX_FMT_YUV420P, v4l2->getWidth(),
			v4l2->getHeight());
	v4l2->startStream();

	pEncode->x264_init(Picture, 320, 240);

}

V4L2FramedSource::~V4L2FramedSource()
{
}

unsigned V4L2FramedSource::maxFrameSize() const
{
	return 100 * 1024;
}

void V4L2FramedSource::doGetNextFrame()
{
printf("m_started = %d\n", m_started);
	if(m_started) return;
	m_started = 1;

	double delay = 1000.0 / 5;
	 int to_delay = delay * 1000;    // us
	 mp_token = envir().taskScheduler().scheduleDelayedTask(to_delay,
	 getNextFrame, this);
	printf("doGetNextFrame\n");
	/*if (V4L2FramedSource::nalIndex == pEncode->nnal)
	{
		v4l2->readFrame(Picture, PIX_FMT_YUV420P, v4l2->getWidth(),
				v4l2->getHeight());
		pEncode->x264_encode();
		V4L2FramedSource::nalIndex = 0;
		gettimeofday(&fPresentationTime, NULL);
	}
	memmove(fTo, pEncode->nals[V4L2FramedSource::nalIndex].p_payload,
			pEncode->nals[V4L2FramedSource::nalIndex].i_payload);
	printf("head[0]=%x\n",pEncode->nals[V4L2FramedSource::nalIndex].p_payload[4]);
	fFrameSize = pEncode->nals[V4L2FramedSource::nalIndex].i_payload;
	V4L2FramedSource::nalIndex++;
	afterGetting(this); //通知RTPSink数据获取完成*/
}

void V4L2FramedSource::getNextFrame1()
{
	int res = v4l2->readFrame(Picture, PIX_FMT_YUV420P, v4l2->getWidth(),
				v4l2->getHeight());
	if(res < 1){
		m_started = 0;
		printf("failed to readframe\n");
		return;
	}	
	pEncode->x264_encode();
	int output_datasize = 0;
	char *pout = (char*)outbuf;

	for(int i=0;i<pEncode->nnal;i++){
		if(output_datasize + pEncode->nals[i].i_payload > output_bufsize){
			output_bufsize = (output_datasize + pEncode->nals[i].i_payload + 4095)/4096 * 4096;
			outbuf  = realloc(outbuf, output_bufsize);
		}
		memcpy(pout + output_datasize, pEncode->nals[i].p_payload, pEncode->nals[i].i_payload);
		output_datasize += pEncode->nals[i].i_payload;
	}

	gettimeofday(&fPresentationTime, NULL);
	fFrameSize = output_datasize;
	if(fFrameSize > fMaxSize){
		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	}else{
		fNumTruncatedBytes = 0;
	}

	memmove(fTo, outbuf, fFrameSize);
	afterGetting(this);

	m_started = 0;
	/*if (V4L2FramedSource::nalIndex == pEncode->nnal)
	{
		v4l2->readFrame(Picture, PIX_FMT_YUV420P, v4l2->getWidth(),
				v4l2->getHeight());
		pEncode->x264_encode();
		V4L2FramedSource::nalIndex = 0;
		gettimeofday(&fPresentationTime, NULL);
	}
	memmove(fTo, pEncode->nals[V4L2FramedSource::nalIndex].p_payload,
			pEncode->nals[V4L2FramedSource::nalIndex].i_payload);
	printf("head[0]=%x\n",pEncode->nals[V4L2FramedSource::nalIndex].p_payload[4]);
	fFrameSize = pEncode->nals[V4L2FramedSource::nalIndex].i_payload;
	V4L2FramedSource::nalIndex++;
	afterGetting(this);*/
}

