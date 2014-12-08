#ifndef H264ONDEMANDSERVERMEDIASUBSESSION_H_
#define H264ONDEMANDSERVERMEDIASUBSESSION_H_

#include <live/onDemandServerMediaSubsession.hh>

class H264OnDemandServerMediaSubsession: public OnDemandServerMediaSubsession{
	public:
		H264OnDemandServerMediaSubsession(UsageEnvironment &env, FramedSource *source);
		virtual ~H264OnDemandServerMediaSubsession();
		virtual const char* getAuxSDPLine(RTPSink *sink, FramedSource *source);
		virtual RTPSink *createNewRTPSink(Groupsock *rtpsock, unsigned char type, FramedSource *source);
		virtual FramedSource *createNewStreamSource(unsigned sid, unsigned &bitrate);
		virtual char const *sdpLines();

	private:
		static void afterPlayingDummy(void *ptr){
			((H264OnDemandServerMediaSubsession*) ptr)->m_done = 0xff;
		}

		static void chkForAuxSDPLine(void *ptr){
			((H264OnDemandServerMediaSubsession*) ptr)->chkForAuxSDPLine1();
		}

		void chkForAuxSDPLine1();

	private:
		FramedSource *mp_source;//对应V4L2FramedSource
		char *mp_sdp_line;
		RTPSink *mp_dummy_rtpsink;
		char m_done;
};

#endif
