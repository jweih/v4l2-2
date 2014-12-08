/*
 * Application.cpp
 *
 *  Created on: 2014年1月4日
 *      Author: ny
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "H264OnDemandServerMediaSubsession.h"
#include "V4L2FramedSource.h"

#include <live/liveMedia.hh>
#include <live/BasicUsageEnvironment.hh>
#include <live/UsageEnvironment.hh>

UsageEnvironment* env;
static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
		char const* streamName, char const* inputFileName = "Live"); // fwd
int main()
{
	// 设置使用环境。Begin by setting up our usage environment:
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	env = BasicUsageEnvironment::createNew(*scheduler);
	UserAuthenticationDatabase* authDB = NULL;

	RTSPServer* rtspServer = RTSPServer::createNew(*env, 8554, authDB);
	if (rtspServer == NULL)
	{
		*env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
		exit(1);
	}

	char const* descriptionString =
			"Session streamed by \"testOnDemandRTSPServer\"";

	char const* streamName = "live";
	ServerMediaSession* sms = ServerMediaSession::createNew(*env, streamName,
			streamName, descriptionString);
	sms->addSubsession(
			new H264OnDemandServerMediaSubsession(*env,
					new V4L2FramedSource(*env)));
	rtspServer->addServerMediaSession(sms);
	announceStream(rtspServer, sms, streamName);
	env->taskScheduler().doEventLoop(); // does not return

	return 0;// only to prevent compiler warning
}
static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
		char const* streamName, char const* inputFileName)
{
	char* url = rtspServer->rtspURL(sms);
	UsageEnvironment& env = rtspServer->envir();
	env << "\n\"" << streamName << "\" stream, from the file \""
			<< inputFileName << "\"\n";
	env << "Play this stream using the URL \"" << url << "\"\n";
	delete[] url;
}
