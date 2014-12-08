#ifndef V4L2FRAMEDSOURCE_H_
#define V4L2FRAMEDSOURCE_H_

#include <live/FramedSource.hh>
#include <live/UsageEnvironment.hh>
#include "H264Encode.h"
#include "V4L2.h"

class V4L2FramedSource: public FramedSource{
	public:
		V4L2FramedSource(UsageEnvironment &env);
		virtual ~V4L2FramedSource();
	protected:
		virtual void doGetNextFrame();
		virtual unsigned maxFrameSize() const;
	private:
		static void getNextFrame(void *ptr){
			((V4L2FramedSource*)ptr)->getNextFrame1();
		}
		void getNextFrame1();
	private:
		V4L2 *v4l2;
		H264Encode *pEncode;
		AVPicture Picture;
		void *mp_token;
		
		static int nalIndex;
};

#endif
