//一般操作流程（视频设备）：
//1. 打开设备文件。 int fd=open(”/dev/video0″,O_RDWR);
//2. 取得设备的capability，看看设备具有什么功能，比如是否具有视频输入,或者音频输入输出等。VIDIOC_QUERYCAP,struct v4l2_capability
//3. 选择视频输入，一个视频设备可以有多个视频输入。VIDIOC_S_INPUT,struct v4l2_input
//4. 设置视频的制式和帧格式，制式包括PAL，NTSC，帧的格式个包括宽度和高度等。
//VIDIOC_S_STD,VIDIOC_S_FMT,struct v4l2_std_id,struct v4l2_format
//5. 向驱动申请帧缓冲，一般不超过5个。struct v4l2_requestbuffers
//6. 将申请到的帧缓冲映射到用户空间，这样就可以直接操作采集到的帧了，而不必去复制。mmap
//7. 将申请到的帧缓冲全部入队列，以便存放采集到的数据.VIDIOC_QBUF,struct v4l2_buffer
//8. 开始视频的采集。VIDIOC_STREAMON
//9. 出队列以取得已采集数据的帧缓冲，取得原始采集数据。VIDIOC_DQBUF
//10. 将缓冲重新入队列尾,这样可以循环采集。VIDIOC_QBUF
//11. 停止视频的采集。VIDIOC_STREAMOFF
//12. 关闭视频设备。close(fd);

#include <linux/videodev2.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/types.h>
#include <malloc.h>
#include <math.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>

#define FILE_VIDEO "/dev/video0"
#define JPG "/lxm/picture/image%d.jpg"

typedef struct{
	void *start;
	int length;
}VideoBuffer;
VideoBuffer* usr_buf;

static unsigned int n_buffer = 0;

//set video capture ways(mmap)
int init_mmap(int fd){
	//to request frame cache, contain requested counts,即在缓存中有多少张照片
	struct v4l2_requestbuffers reqbufs;
	//request v4l2 driver allocation video cache
	//this cache locate in kernel and need mmap mapping
	memset(&reqbufs, 0, sizeof(reqbufs));
	reqbufs.count = 4;
	reqbufs.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbufs.memory = V4L2_MEMORY_MMAP;

	if(-1 == ioctl(fd, VIDIOC_REQBUFS, &reqbufs)){
		perror("Fail to ioctl 'VIDIOC_REQBUFS'");
		exit(EXIT_FAILURE);
	}

	n_buffer = reqbufs.count;
	printf("n_buffer = %d\n", n_buffer);
	usr_buf = calloc(reqbufs.count, sizeof(usr_buf));
	if(usr_buf == NULL){
		printf("Out of memory\n");
		exit(-1);
	}

	struct v4l2_buffer buf;
	//map kernel cache to user process
	for (n_buffer = 0; n_buffer < reqbufs.count; n_buffer++){
		//stand for a frame
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffer;
		// 查询序号为n_buffers 的缓冲区，得到其起始物理地址和大小
		if(-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf)){
			perror("Fail to ioctl: VIDEOC_QUERYBUF");
			exit(EXIT_FAILURE);
		}

		usr_buf[n_buffer].length = buf.length;
		usr_buf[n_buffer].start = (char*)mmap(NULL,
				buf.length, PROT_READ | PROT_WRITE, MAP_PRIVATE,
				fd, buf.m.offset);
		if(MAP_FAILED == usr_buf[n_buffer].start){
			perror("Fail to mmap");
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

//initialize camera device
int init_camera_device(int fd){
	//device function, such as video input
	struct v4l2_capability cap;
	//video standard, such as PAL, NTSC
	struct v4l2_standard std;
	//frame format
	struct v4l2_format tv_fmt;
	//check control
	struct v4l2_queryctrl query;
	//视频格式描述符
	struct v4l2_fmtdesc fmt;

	int ret;
	//get the format of video supply
	memset(&fmt, 0, sizeof(fmt));
	fmt.index = 0;//支持的格式数目
	//supply to image capture, 缓存类型
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//show all format of supply
	printf("Support format:\n");
	while(ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == 0){
		fmt.index++;
		printf("pixelformat = ''%c%c%c%c''\ndescription = ''%s''\n", 
				fmt.pixelformat&0xFF, (fmt.pixelformat >> 8)&0xFF, 
				(fmt.pixelformat >> 16)&0xFF, (fmt.pixelformat>>24)&0xFF,
				fmt.description);
	}
	//check video device driver capability
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0){
		perror("Fail to ioctl: VIDEO_QUERYCAP");
		exit(EXIT_FAILURE);
	}

	//judge whether or not to be a video-get device
	if(!(cap.capabilities & V4L2_BUF_TYPE_VIDEO_CAPTURE)){
		printf("The Current device is not a video capture device\n");
		exit(-1);
	}

	//check whether or not to support the form of video stream
	if(!(cap.capabilities & V4L2_CAP_STREAMING)){
		printf("The Current device does not support streaming i/o \n");
		exit(EXIT_FAILURE);
	}

	//set the format of camera capture data
	tv_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	tv_fmt.fmt.pix.width = 680;
	tv_fmt.fmt.pix.height = 480;
	tv_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	tv_fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	if(ioctl(fd, VIDIOC_S_FMT, &tv_fmt)<0){
		printf("Failed to ioctl: VIDIOC_S_FMT\n");
		exit(-1);
		close(fd);
	}

	init_mmap(fd);
	return 0;
}

int open_camera_device(){
	int fd;
	//open video device with block
	fd = open(FILE_VIDEO, O_RDONLY);
	if(fd < 0){
		perror(FILE_VIDEO);
		exit(EXIT_FAILURE);
	}
	return fd;
}

int start_capture(int fd){
	unsigned int i;
	enum v4l2_buf_type type;
	//place the kernel cache to a queue
	for (i = 0;i<n_buffer; i++){
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
			perror("Fail to ioctl: 'VIDIOC_QBUF'");
			exit(EXIT_FAILURE);
		}
	}

	//start capture data,VIDIOC_STREAMON即为启动视频采集命令
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMON, &type)){
		printf("i=%d.\n", i);
		perror("VIDIOC_STREAMON");
		close(fd);
		exit(EXIT_FAILURE);
	}
	return 0;
}

int process_image(void *addr, int length){
	FILE* fp;
	static int num = 0;
	char image_name[20];

	sprintf(image_name, JPG, num++);
	if((fp = fopen(image_name, "w")) == NULL){
		perror("Fail to fopen");
		exit(-1);
	}
	fwrite(addr, length, 1, fp);
	usleep(500);
	fclose(fp);
	return 0;
}

int read_frame(int fd){
	struct v4l2_buffer buf;
	unsigned int i;
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	//put cache from queue
	if(-1 == ioctl(fd, VIDIOC_DQBUF, &buf)){
		perror("Fail to ioctl: VIDIOC_DQBUF");
		exit(-1);
	}

	assert(buf.index<n_buffer);
	//read process space's data to a file
	process_image(usr_buf[buf.index].start, usr_buf[buf.index].length);
	if(-1 == ioctl(fd, VIDIOC_QBUF, &buf)){
		perror("Fail to ioctl 'VIDIOC_QBUF'");
		exit(-1);
	}
	return 1;
}

int mainloop(int fd){
	int count = 10;

	while(count-- > 0){
		for(;;){
			fd_set fds;
			struct timeval tv;
			int r;

			FD_ZERO(&fds);
			FD_SET(fd, &fds);

			//Timeout
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			r = select(fd+1, &fds, NULL, NULL, &tv);

			if(-1 == r){
				if(EINTR == errno){
					continue;
				}
				perror("Fail to select");
				exit(-1);
			}

			if( 0 == r){
				fprintf(stderr, "select timeout\n");
				exit(-1);
			}

			if(read_frame(fd))
				break;
		}
	}
	return 0;
}

void stop_capture(int fd){
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd, VIDIOC_STREAMOFF, &type)){
		perror("Fail to ioctl 'VIDIOC_STREAMOFF'");
		exit(-1);
	}
	return;
}

void close_camera_device(int fd){
	unsigned int i;
	for(i = 0;i<n_buffer; i++){
		if(-1 == munmap(usr_buf[i].start, usr_buf[i].length)){
			exit(-1);
		}
	}
	free(usr_buf);
	
	if(-1 == close(fd)){
		perror("Fail to close fd");
		exit(-1);
	}
	return ;
}

int main(){
	int fd; 
	fd = open_camera_device();
	init_camera_device(fd);
	start_capture(fd);
	mainloop(fd);
	stop_capture(fd);
	close_camera_device(fd);
	return 0;
}
