//  Copyright © 2015 Infrared5, Inc. All rights reserved.
//
//  The accompanying code comprising examples for use solely in conjunction with Red5 Pro (the "Example Code") 
//  is  licensed  to  you  by  Infrared5  Inc.  in  consideration  of  your  agreement  to  the  following  
//  license terms  and  conditions.  Access,  use,  modification,  or  redistribution  of  the  accompanying  
//  code  constitutes your acceptance of the following license terms and conditions.
//  
//  Permission is hereby granted, free of charge, to you to use the Example Code and associated documentation 
//  files (collectively, the "Software") without restriction, including without limitation the rights to use, 
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
//  persons to whom the Software is furnished to do so, subject to the following conditions:
//  
//  The Software shall be used solely in conjunction with Red5 Pro. Red5 Pro is licensed under a separate end 
//  user  license  agreement  (the  "EULA"),  which  must  be  executed  with  Infrared5,  Inc.   
//  An  example  of  the EULA can be found on our website at: https://account.red5pro.com/assets/LICENSE.txt.
// 
//  The above copyright notice and this license shall be included in all copies or portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,  INCLUDING  BUT  
//  NOT  LIMITED  TO  THE  WARRANTIES  OF  MERCHANTABILITY, FITNESS  FOR  A  PARTICULAR  PURPOSE  AND  
//  NONINFRINGEMENT.   IN  NO  EVENT  SHALL INFRARED5, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//  WHETHER IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM,  OUT  OF  OR  IN CONNECTION 
//  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "device.h"

static struct vfl2_device_context context_vfl2;

uint64_t get_timestamp_ms()
{
	// Total seconds live * 1000,  plus latest micro time divided by 1000 = total milliseconds not counting initial micro offset.
	uint64_t current = ((context_vfl2.bufferinfo.timestamp.tv_sec - context_vfl2.first_timestamp.tv_sec) * 1000) + (context_vfl2.bufferinfo.timestamp.tv_usec / 1000);
	//Shave off initial microtime offset in milliseconds.
	uint64_t milliseconds = current - (context_vfl2.first_timestamp.tv_usec / 1000);
	return milliseconds;
}

void copy_format(r5config *copy, r5config *config_in)
{
	copy->audio = config_in->audio;
	copy->audio_rate = config_in->audio_rate;
	copy->channel_count = config_in->channel_count;
	copy->height = config_in->height;
	copy->video = config_in->video;
	copy->video_bitrate = config_in->video_bitrate;
	copy->video_format = config_in->video_format;
	copy->width = config_in->width;
}

void get_format_vfl2(r5config *copy)
{
	copy->audio = context_vfl2.config.audio;
	copy->audio_rate = context_vfl2.config.audio_rate;
	copy->channel_count = context_vfl2.config.channel_count;
	copy->height = context_vfl2.config.height;
	copy->video = context_vfl2.config.video;
	copy->video_bitrate = context_vfl2.config.video_bitrate;
	copy->video_format = context_vfl2.config.video_format;
	copy->width = context_vfl2.config.width;
}

uint32_t fill_buffer_vfl2(media_sample* sample)
{

	if (!sample)
	{// unlikely.
		return 0;
	}
	// bufferinfo in focus.
	// Dequeue a buffer
	if (ioctl(context_vfl2.file_desc, VIDIOC_DQBUF, &context_vfl2.bufferinfo) < 0)
	{
		perror("Could not dequeue the buffer, VIDIOC_DQBUF");
		return 0;
	}
	//What did we get?
	if (context_vfl2.bufferinfo.bytesused)
	{
		if (context_vfl2.frame_count == 0)
		{//Time epoc
			context_vfl2.first_timestamp.tv_sec = context_vfl2.bufferinfo.timestamp.tv_sec;
			context_vfl2.first_timestamp.tv_usec = context_vfl2.bufferinfo.timestamp.tv_usec;
		}
		context_vfl2.frame_count++;

		//collect annex-b h264 data.
		sample->size = context_vfl2.bufferinfo.bytesused;
		sample->buffer =(uint8_t*) malloc(sample->size);

		//some framerate / resolutions will require more than one hardware buffer.
		//we asked for 2, buffer0 and buffer1.
		if (context_vfl2.bufferinfo.index == 0)
		{
			memcpy(sample->buffer, context_vfl2.buffer0, sample->size);
		}
		else if (context_vfl2.bufferinfo.index == 1)
		{
			memcpy(sample->buffer, context_vfl2.buffer1, sample->size);
		}
		//epoc math.
		sample->timestamp = get_timestamp_ms();
		sample->type = context_vfl2.config.video_format;//fourccc H264

		//Return buffer to hardware for refill.
		if (ioctl(context_vfl2.file_desc, VIDIOC_QBUF, &context_vfl2.bufferinfo) < 0)
		{
			perror("fb, Could not queue buffer, VIDIOC_QBUF");
			return 1;
		}
	}

	return 0;
}

uint32_t free_buffer_vfl2(media_sample* sample)
{
	if (sample && sample->buffer)
	{
		free(sample->buffer);
	}
}

uint32_t get_device_count_vfl2()
{
	return 2;// /dev/video0 and /dev/video1
}

uint32_t open_device_vfl2(r5config * config)
{
	if (config)
	{
		copy_format(&context_vfl2.config, config);
		if (config->video >= 64)
		{
			return 1;
		}

		if (config->video_format == 0)
		{
			config->video_format = V4L2_PIX_FMT_H264;
		}
		char device_string[16];

		sprintf(device_string, "/dev/video%d", config->video);

		context_vfl2.file_desc = open(device_string, O_RDWR);
		if (context_vfl2.file_desc < 0)
		{
			perror("Device open error");
			return 1;
		}

		// 2. Ask the device if it can capture frames
		struct v4l2_capability capability;
		memset(&capability, 0, sizeof(capability));
		if (ioctl(context_vfl2.file_desc, VIDIOC_QUERYCAP, &capability) < 0)
		{
			char device_string[64];
			sprintf(device_string, "Error loading video device id: %d ", config->video);
			// something went wrong... exit
			perror(device_string);
			close(context_vfl2.file_desc);
			return 2;
		}

		// 3. Set Image format

		context_vfl2.imageFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		context_vfl2.imageFormat.fmt.pix.width = config->width;
		context_vfl2.imageFormat.fmt.pix.height = config->height;
		context_vfl2.imageFormat.fmt.pix.pixelformat = config->video_format;
		context_vfl2.imageFormat.fmt.pix.field = V4L2_FIELD_NONE;

		// tell the device you are using this format
		if (ioctl(context_vfl2.file_desc, VIDIOC_S_FMT, &context_vfl2.imageFormat) < 0)
		{
			perror("Device could not accept format, VIDIOC_S_FMT");
			return 3;
		}

		//what did we get?
		if (ioctl(context_vfl2.file_desc, VIDIOC_G_FMT, &context_vfl2.imageFormat) < 0)
		{
			perror("Device could not tell us the format, VIDIOC_S_FMT");
			close(context_vfl2.file_desc);
			return 3;
		}
		char f0 = (char)context_vfl2.imageFormat.fmt.pix.pixelformat & 0xFF;
		char f1 = (char)(context_vfl2.imageFormat.fmt.pix.pixelformat >> 8) & 0xFF;
		char f2 = (char)(context_vfl2.imageFormat.fmt.pix.pixelformat >> 16) & 0xFF;
		char f3 = (char)(context_vfl2.imageFormat.fmt.pix.pixelformat >> 24) & 0xFF;

		//cout << "requested: " << f0 << f1 << f2 << f3 << " " << imageFormat.fmt.pix.width << " " << imageFormat.fmt.pix.height << std::endl;

		config->video_format = context_vfl2.imageFormat.fmt.pix.pixelformat;
		config->width = context_vfl2.imageFormat.fmt.pix.width;
		config->height = context_vfl2.imageFormat.fmt.pix.height;
		f0 = (char)config->video_format & 0xFF;
		f1 = (char)(config->video_format >> 8) & 0xFF;
		f2 = (char)(config->video_format >> 16) & 0xFF;
		f3 = (char)(config->video_format >> 24) & 0xFF;
		//cout << "device configured " << f0 << f1 << f2 << f3 << " " << config.width << " " << config.height << " " << std::endl;

		// 4. Request Buffers from the device
		struct v4l2_requestbuffers requestBuffer;
		memset(&requestBuffer, 0, sizeof(requestBuffer));
		requestBuffer.count = 2; // one request buffer
		requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // request a buffer wich we an use for capturing frames
		requestBuffer.memory = V4L2_MEMORY_MMAP;

		if (ioctl(context_vfl2.file_desc, VIDIOC_REQBUFS, &requestBuffer) < 0)
		{
			perror("Could not request buffer from device, VIDIOC_REQBUFS");
			close(context_vfl2.file_desc);
			return 4;
		}
		// 5. Quety the buffer to get raw data ie. ask for the you requested buffer
		// and allocate memory for it
		context_vfl2.queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		context_vfl2.queryBuffer.memory = V4L2_MEMORY_MMAP;
		context_vfl2.queryBuffer.index = 0;
		if (ioctl(context_vfl2.file_desc, VIDIOC_QUERYBUF, &context_vfl2.queryBuffer) < 0)
		{
			perror("Could not get buffer information, VIDIOC_QUERYBUF");
			close(context_vfl2.file_desc);
			return 5;
		}
		// use a pointer to point to the newly created buffer
		// mmap() will map the memory address of the device to
		// an address in memory
		context_vfl2.buffer0 = (char*)mmap(NULL, context_vfl2.queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
			context_vfl2.file_desc, context_vfl2.queryBuffer.m.offset);

		if (MAP_FAILED == context_vfl2.buffer0)
		{
			perror("Could not map device, MAP_FAILED");
			close(context_vfl2.file_desc);
			return 6;
		}
		context_vfl2.buf0_len = context_vfl2.queryBuffer.length;
		memset(context_vfl2.buffer0, 0, context_vfl2.buf0_len);


		context_vfl2.queryBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		context_vfl2.queryBuffer.memory = V4L2_MEMORY_MMAP;
		context_vfl2.queryBuffer.index = 1;
		if (ioctl(context_vfl2.file_desc, VIDIOC_QUERYBUF, &context_vfl2.queryBuffer) < 0)
		{
			perror("Could not get buffer information, VIDIOC_QUERYBUF");

		}
		context_vfl2.buf1_len = context_vfl2.queryBuffer.length;
		context_vfl2.buffer1 = (char*)mmap(NULL, context_vfl2.queryBuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED,
			context_vfl2.file_desc, context_vfl2.queryBuffer.m.offset);

		if (MAP_FAILED == context_vfl2.buffer1)
		{
			perror("Could not map device1, MAP_FAILED");
		}
		// 6. Get a frame
		// Create a new buffer type so the device knows whichbuffer we are talking about

		memset(&context_vfl2.bufferinfo, 0, sizeof(context_vfl2.bufferinfo));
		context_vfl2.bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		context_vfl2.bufferinfo.memory = V4L2_MEMORY_MMAP;
		context_vfl2.bufferinfo.index = 0;

		// Activate streaming
		int type = context_vfl2.bufferinfo.type;
		if (ioctl(context_vfl2.file_desc, VIDIOC_STREAMON, &type) < 0)
		{
			perror("Could not start streaming, VIDIOC_STREAMON");
			munmap(context_vfl2.buffer0, context_vfl2.buf0_len);
			munmap(context_vfl2.buffer1, context_vfl2.buf1_len);
			context_vfl2.buffer0 = context_vfl2.buffer1 = 0;
			close(context_vfl2.file_desc);
			return 7;
		}
		context_vfl2.bufferinfo.index = 0;
		if (ioctl(context_vfl2.file_desc, VIDIOC_QBUF, &context_vfl2.bufferinfo) < 0)
		{
			perror("Could not queue buffer0, VIDIOC_QBUF");
			return 1;
		}
		context_vfl2.bufferinfo.index = 1;
		if (ioctl(context_vfl2.file_desc, VIDIOC_QBUF, &context_vfl2.bufferinfo) < 0)
		{
			perror("Could not queue buffer1, VIDIOC_QBUF");
		}
		printf("vfl2 device opened \n");
		return 0;
	}
	return 1;
}

uint32_t close_device_vfl2()
{
	//stop streaming.
	if (context_vfl2.file_desc)
	{
		int32_t type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (ioctl(context_vfl2.file_desc, VIDIOC_STREAMOFF, &type) < 0)
		{
			perror("Could not end streaming @ close_device, VIDIOC_STREAMOFF");
		}
		//unmap hardware buffers.
		if (context_vfl2.buffer0)
		{
			munmap(context_vfl2.buffer0, context_vfl2.buf0_len);
		}
		context_vfl2.buffer0 = 0;

		if (context_vfl2.buffer1)
		{
			munmap(context_vfl2.buffer1, context_vfl2.buf1_len);
		}
		context_vfl2.buffer1 = 0;
		//release Camera.
		close(context_vfl2.file_desc);
		context_vfl2.file_desc = 0;
	}
	return 0;
}

void make_vfl2_device(r5device* vfl2_device)
{
	memset(&context_vfl2, 0, sizeof(vfl2_device_context));
	vfl2_device->get_device_count = get_device_count_vfl2;
	vfl2_device->open_device = open_device_vfl2;
	vfl2_device->get_format = get_format_vfl2;
	vfl2_device->fill_buffer = fill_buffer_vfl2;
	vfl2_device->free_buffer = free_buffer_vfl2;
	vfl2_device->close_device = close_device_vfl2;
}
