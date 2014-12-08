#include "V4L2.h"

V4L2::V4L2(){
	fd = -1;
	buffers = NULL;
	width = 0;
	height = 0;
	CLEAR(fmt);
	//注意：如果该视频设备驱动不支持你所设定的图像格式，视频驱动会重新修改struct v4l2_format结构体变量的值为该视频设备所支持的图像格式，所以在程序设计中，设定完所有的视频格式后，要获取实际的视频格式，要重新读取struct v4l2_format结构体变量。
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pixel.pixel_format = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
}

V4L2::~V4L2(){
	close(fd);
}

V4L2::getWidth(){
	return width;
}

V4L2::getHeight(){
	return height;
}

bool V4L2::setSize(int width, int height){
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	//设置视频格式
	if(ioctl(fd, VIDIOC_S_FMT, &fmt) == -1){
		printf("Failed to ioctl: VIDIOC_S_FMT\n");
		return false;
	}
	getSizeInfo();
	return true;
}

void V4L2::getSizeInfo(){
	if(ioctl(fd, VIDIOC_G_FMT, &fmt) == -1){
		printf("Failed to ioctl: VIDIOC_G_FM\n");
		return;
	}
	this->width = fmt.fmt.pix.width;
	this->height = fmt.fmt.pix.height;

	// Print Stream Format
	printf("Stream Format Informations:\n");
	printf(" type: %d\n", fmt.type);
	printf(" width: %d\n", fmt.fmt.pix.width);
	printf(" height: %d\n", fmt.fmt.pix.height);
	char fmtstr[8];
	memset(fmtstr, 0, 8);
	memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
	printf(" pixelformat: %s\n", fmtstr);
	printf(" field: %d\n", fmt.fmt.pix.field);
	printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
	printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
	printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
	printf(" priv: %d\n", fmt.fmt.pix.priv);
	printf(" raw_date: %s\n", fmt.fmt.raw_data);
}

bool V4L2::initDev(const char* devName, int width, int height){
	v4l2_capability cap;

	struct v4l2_fmtdesc fmtdesc;//视频格式描述符
	//get the format of video supply
	CLEAR(fmtdesc);
	fmtdesc.index = 0;//支持的格式数目
	//supply to image capture, 缓存类型
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//show all format of supply
	printf("Support format:\n");
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0){
		fmtdesc.index++;
		printf("pixelformat = ''%c%c%c%c''\ndescription = ''%s''\n", 
				fmtdesc.pixelformat&0xFF, (fmtdesc.pixelformat >> 8)&0xFF, 
				(fmtdesc.pixelformat >> 16)&0xFF, 
				(fmtdesc.pixelformat>>24)&0xFF,fmtdesc.description);
	}

	fd = open(devName, O_RDWR, 0);//打开设备
	if(fd == -1){
		printf("Can't open %s\n", devName);
		return false;
	}
	//查询设备功能
	if(ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)){
		printf("Failed to ioctl: VIDIOC_QUERYCAP\n");
		return false;
	}

	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)){
		printf("Can't capture video\n");
		return false;
	}

	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		printf("The current device doesn't support streaming\n");
		return false;
	}

	if(!setSize(width, height)){
		return false;
	}

	// Print capability infomations
	printf("Capability Informations:\n");
	printf(" driver: %s\n", cap.driver);
	printf(" card: %s\n", cap.card);
	printf(" bus_info: %s\n", cap.bus_info);
	printf(" version: %08X\n", cap.version);
	printf(" capabilities: %08X\n", cap.capabilities);

	return initMmap();
}

bool V4L2::initMmap(){
	struct v4l2_requestbuffers req;
	unsigned int n_buffers;
	CLEAR(req);

	req.count = 4;//先要从内核申请buffer缓冲，一般选择4个缓冲，最多是5个
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	//向内存里面设置buffer,注意：VIDIOC_REQBUFS会修改req的count值，req的count值返回实际申请成功的视频缓冲区数目;
	if(-1 == ioctl(fd, VIDIOC_REQBUFS, &req)){
		if(EINVAL == errno){
			printf("%s does not support memory mapping\n", "ss");
		}else{
			printf("Failed to ioctl: VIDIOC_REQBUFS\n");
		}
		exit(EXIT_FAILURE);
	}

	buffers = (buffer*) calloc(req.count, sizeof(buffer));
	if(!buffers){
		printf("Out of memory\n");
		exit(EXIT_FAILURE);
	}

	//查询已经分配的V4L2的视频缓冲区的相关信息，包括视频缓冲区的使用状态、在内核空间的偏移地址、缓冲区长度等。通过调用VIDIOC_QUERYBUF来获取内核空间的视频缓冲区信息，然后调用函数mmap把内核空间地址映射到用户空间，这样应用程序才能够访问位于内核空间的视频缓冲区。
	for(n_buffers = 0; n_buffers < req.count; n_buffers++){
		struct v4l2_buffer buf;
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)){
			printf("Failed to ioctl: VIDIOC_QUERYBUF\n");
		}

		buffers[n_buffers].length = buf.length;
		printf("buf.length %d\n", buffers[n_buffers].length);
		buffers[n_buffers].start = mmap(NULL, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

		if(MAP_FAILED == buffers[n_buffers].start){
			printf("Failed to mmap\n");
			exit(EXIT_FAILURE);
		}
	}
	return 1;
}

bool V4L2::startStream(){
	unsigned int n_buffers;
	enum v4l2_buf_type type;
	//将申请到的帧缓冲全部入队列，以便存放采集到的数据
	for (n_buffers = 0; n_buffers<4; n_buffers++){
		v4l2_buffer buf;
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;
		//放入缓存
		if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
			printf("Failed to ioctl: VIDIOC_QBUF\n");
			return false;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMON, &type)){
		printf("n_buffers=%d.\n", n_buffers);
		perror("Failed to ioctl: VIDIOC_STREAMON\n");
		close(fd);
		exit(EXIT_FAILURE);
	}else
		printf("Video stream on\n")；
	return 0;
}

//摄像头数据，主要是YUYV格式数据，所以需要对格式进行转换，转换后的数据保存在AVPicture里面，格式是由FMT制定的，输出图像尺寸也是width_des, height_des决定的
bool V4L2::readFrame(AVPicture &pPictureDes, AVPixelFormat FMT, int width_des, int height_des){
	v4l2_buffer buf;
	AVPicture pPictureSrc;
	SwsContext *pSwxCtx;
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf)){
		printf("Failed to ioctl: VIDIOC_DQBUF\n");
		return false;
	}
	pPictureSrc.data[0] = (unsigned char*)buffers[buf.index].start;
	pPictureSrc.data[1] = pPictureSrc.data[2] = pPictureSrc.data[3] = NULL;
	pPictureSrc.linesize[0] = fmt.fmt.pix.bytesperline;

	int i = 0;
	for(i = 1;i<8;i++){
		pPictureSrc.linesize[i] = 0;
	}
	pSwsCtx = sws_getContext(width, height, PIX_FMT_YUYV422, width_des, height_des, FMT, SWS_BICUBIC, 0, 0, 0);
	int rs = sws_scale(pSwsCtx, pPictureSrc.data, pPictureSrc.linesize, 0, height, pPictureDes.data, pPictureDes.linesize);
	if(rs == -1){
		printf("Can't open to change to des image\n");
		return false;
	}
	sws_freeContest(pSwsCtx);
	if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
		printf("Failed to ioctl: VIDIOC_QBUF2\n");
		return false;
	}
	return true;
}

bool V4L2::stopStream(){
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)){
		perror("Failed to ioctl: VIDIOC_STREAMOFF\n");
		return false;
	}
	return true;
}
