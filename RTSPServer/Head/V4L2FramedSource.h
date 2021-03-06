#ifndef V4L2FRAMEDSOURCE_H_
#define V4L2FRAMEDSOURCE_H_

#include <live/FramedSource.hh>
#include <live/UsageEnvironment.hh>
#include "H264Encode.h"
#include "V4L2.h"

#include <signal.h>

class V4L2FramedSource: public FramedSource
{
public:
	V4L2FramedSource(UsageEnvironment & env);
	virtual ~V4L2FramedSource();
protected:
	virtual void doGetNextFrame();
	virtual unsigned maxFrameSize() const;
private:
	static void getNextFrame(void *ptr)
	{
		((V4L2FramedSource *) ptr)->getNextFrame1();
	}
	void getNextFrame1();

	static void stop(int signo){
		v4l2->stopStream();
		exit(0);
	}
private:
	static V4L2 * v4l2;
	H264Encode *pEncode;
	AVPicture Picture;
	void * mp_token;

	static int nalIndex;

	int m_started;

	int output_bufsize;
	void *outbuf;
};

#endif /* V4L2FRAMEDSOURCE_H_ */
