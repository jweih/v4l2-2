//主要是为了设置SDP 描述
#include <H264OnDemandServerMediaSubsession.h>
#include <v4L2FramedSource.h>
#include <live/H264VideoStreamFramer.hh>
#include <live/H264VideoRTPSink.hh>

H264OnDemandServerMediaSubsession::H264OnDemandServerMediaSubsession(
		UsageEnvironment &env, FramedSource *source):
	OnDemandServerMediaSubsession(env, true){
		mp_source = source;
		mp_sdp_line = NULL;
		mp_dummy_rtpsink = NULL;
		m_done = 0;
	}

H264OnDemandServerMediaSubsession::~H264OnDemandServerMediaSubsession(){
}

void H264OnDemandServerMediaSubsession::chrForAuxSDPLine1(){
	if(mp_dummy_rtpsink->auxSDPLine()){
		m_done = 0xff;
	}else{
		int delay = 100 * 1000; //100ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(delay, chkForAuxSDPLine, this);
	}
}

const char* H264OnDemandServerMediaSubsession::getAuxSDPLine(RTPSink *sink, FramedSource *source){
	if(mp_sdp_line)
		return mp_sdp_line;

	mp_dummy_rtpsink = sink;
	mp_dummy_rtpsink->startPlaying(*source, 0, 0);
	//mp_dummy_rtpsink->startPlaying(*source, afterPlayingDummy, this);
	chkForAuxSDPLine(this);
	m_done = 0;
	envir().taskScheduler().doEventLoop(&m_done);
	mp_sdp_line = strdup(mp_dummy_rtpsink->auxSDPLine());
	mp_dummy_rtpsink->stopPlaying();

	return mp_sdp_line;
}

RTPSink *H264OnDemandServerMediaSubsession::createNewRTPSink(
		Groupsock *rtpsock, unsigned char type, FramedSource *source){
	return H264VideoRTPSink::createNew(envir(), rtpsock, type);
}

FramedSource *H264OnDemandServerMediaSubsession::createNewStreamSource(
		unsigned sid, unsigned &bitrate){
	bitrate = 500;
	return H264VideoStreamFramer::createNew(envir(), mp_source);
}

char const *H264OnDemandServerMediaSubsession::sdpLines(){
	return fSDPLines = (char*)
		"m=video 0 RTP/AVP 96\r\n"
		"c=IN IP4 0.0.0.0\r\n"//c=<nettype> <addrtype> <connection-address>
		"b=AS:96\r\n"//CT方式是设置整个会议的带宽，AS是设置单个会话的带宽。缺省带宽是千比特每秒
		"a=rtpmap:96 H264/90000\r\n"//a=rtpmap:<payload type> <encoding name>/<clock rate> [/<encoding   parameters>]
		"a=fmtp:96 packetization-mode=1;profile-level-id=000000;sprop-parameter-sets=H264\r\n"//a=fmtp:<format> <format specific parameters>
		"a=control:track1\r\n";
}
