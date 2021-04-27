/*
 * r5pro_api.h
 * Public header.
 *
 *  Created on: Oct 15, 2019
 *      Author: andy
 */
// Copyright © 2015 Infrared5, Inc. All rights reserved.
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

#ifndef R5PRO_API_H_
#define R5PRO_API_H_


#include "inttypes.h"
#include <string.h>
#include "global.h"
#include "cJSON.h"

#define fourcc(a, b, c, d) ((char)(a) | ((char)(b) << 8) | ((char)(c) << 16) | ((char)(d) << 24))

typedef enum R5RecordType_t
{
	R5RecordTypeLive,
	R5RecordTypeRecord,
	R5RecordTypeAppend
}R5RecordType;


//Receive RTP packet for side-stream transmission.
typedef void(*r5_rtp_aux_out)(uint8_t* buffer, uint32_t buffer_size, uint32_t type);
typedef void(*r5_status_cb)(int32_t status, const char* message);
typedef void(*r5_remote_cb)(const char* method, char* params);
typedef void(*r5shared_object_cb)(const char* name, const char* method, cJSON* params);

/**
 * Session configuration
 */
typedef struct r5config
{

	/** Device index */
	uint8_t audio;
	/** Device index */
	uint8_t video;
	/** Video width */
	uint32_t width;
	/** Video height */
	uint32_t height;
	/** Device format */
	uint32_t video_format;
	/** Sample rate */
	uint32_t audio_rate;
	/** Number of audio channels */
	uint8_t channel_count;
	/** Video compression bit-rate */
	uint32_t video_bitrate;
	/** Frames per second*/
	uint32_t video_framerate;
	/** Frames per key, GOP size */
	uint32_t video_key_framerate;

	int protocol;
	char hostName[256];
	char contextName[256];
	char streamName[256];
	R5RecordType recordType;
	//Receive RTP packet for side-stream transmission.
	r5_rtp_aux_out rtp_aux_out;
	r5_status_cb statusCallback;
	r5_remote_cb remoteCallback;

}r5config;


/**
 * Packet Buffer
 */
typedef struct media_sample
{
	/**
	 * Media type
	 */
	uint32_t type;
	/**
	 * Milliseconds
	 */
	uint32_t timestamp;
	uint32_t size;
	uint8_t* buffer;
}media_sample;


typedef uint32_t (*r5device_get_device_count)(void);
typedef uint32_t (*r5device_open_device)(r5config * config);
typedef void (*r5device_get_format)(r5config * config);
typedef uint32_t (*r5device_fill_buffer)(media_sample* sample);
typedef uint32_t (*r5device_free_buffer)(media_sample* sample);
typedef uint32_t (*r5device_close_device)(void);

/**
 * Audio or video capture driver baseclass.
 */
typedef struct r5device
{
	r5device_get_device_count get_device_count;
	r5device_open_device open_device;
	r5device_get_format get_format;
	r5device_fill_buffer fill_buffer;
	r5device_free_buffer free_buffer;
	r5device_close_device close_device;

}r5device;


typedef uint32_t (*r5codec_get_format)(void);
typedef uint32_t (*r5codec_open_coder)(r5config * config);
typedef uint32_t (*r5codec_do_transform)(media_sample* input, media_sample* output);
typedef void (*r5codec_close_coder)(void);
/**
 * Hardware/Software encoder
 */
typedef struct r5codec
{
	r5codec_get_format get_format;
	r5codec_open_coder open_coder;
	r5codec_do_transform do_transform;
	r5codec_close_coder close_coder;

}r5codec;

/**
*	Netstream send parameter
*/
typedef struct send_parameter
{
	const char* name;
	const char* value;
}send_parameter;

/**
 * Netstream send in-stream  cuepoint/metadata.
 */
typedef struct r5stream_send
{
	const char* event_name;
	uint32_t timestamp;
	uint32_t parameter_count;
	send_parameter* parameters;
}r5stream_send;

/**
 * Net connection call remote method.
 */
typedef struct r5invoke
{
	const char* method_name;
	const char* return_handler;
	const char* param;
}r5invoke;

typedef void (*r5shared_object_connect)(void* session);
typedef void (*r5shared_object_setProperty)(void* session, const char* prop, cJSON* value);
typedef void (*r5shared_object_send)(void* session, const char* method, cJSON* params);
typedef const cJSON* (*r5shared_object_getData)(void* session);
typedef void (*r5shared_object_set_callback)(void* session, r5shared_object_cb callback);
typedef const char* (*r5shared_object_get_name)(void* session);

/**
 * Remote shared object handle
 */
typedef struct r5shared_object {
	void* session;
	r5shared_object_connect connect;
	r5shared_object_setProperty set_property;
	r5shared_object_send send;
	r5shared_object_getData get_data;
	r5shared_object_set_callback set_callback;
	r5shared_object_get_name get_name;
}r5shared_object;

typedef void (*r5connection_stream_send)(void* session, r5stream_send* param);
typedef void(*r5connection_invoke)(void* session, r5invoke* param);
typedef r5shared_object* (*r5connection_get_sharedobject)(void* session,const char* path_name);
/**
 * Network communication interface
 */
typedef struct r5connection
{
	void* mem_ptr;
	/**
	* Publisher operation
	* Sends an invoke in-stream to subscribers.
	*/
	r5connection_stream_send stream_send;
	/**
	* Publisher/Subscriber operation
	* Sends an invoke to a server-side handler.
	*/
	r5connection_invoke invoke;
	/**
	* Publisher/Subscriber operation
	* Sets up a remote shared object service handle.
	*/
	r5connection_get_sharedobject get_sharedobject;

}r5connection;

typedef uint32_t(*r5session_set_r5camera)(void* ptr_session, r5device* camera);
typedef r5device* (*r5session_get_r5camera)(void* ptr_session);
typedef uint32_t(*r5session_set_r5mic)(void* ptr_session, r5device* mic);
typedef r5device* (*r5session_get_r5mic)(void* ptr_session);
typedef uint32_t(*r5session_init)(void* ptr_session, r5config *config);
typedef uint32_t(*r5session_start)(void* ptr_session);
typedef r5connection* (*r5session_get_r5connection)(void* ptr_session);
typedef uint32_t(*r5session_stop)(void* ptr_session);
typedef void(*r5session_set_video_coder)(void* ptr_session, r5codec * codec);


/**
 * Capture session manager.
 */
typedef struct  r5session
{
	/**
	* Api pointer.
	*/
	void* mem_ptr;

	r5config* config;
	/**
	 * 1) Optional Override
	 */
	r5session_set_r5camera set_r5camera;

	r5session_get_r5camera get_r5camera;
	/**
	 * 2) Optional Override
	 */
	r5session_set_r5mic set_r5mic;

	r5session_get_r5mic get_r5mic;
	/**
	 * 3) initiate the session.
	 */
	r5session_init init;
	/**
	 * 4) Start the session.
	 */
	r5session_start start;

	r5session_get_r5connection get_r5connection;
	/**
	 * 5) Stop the session
	 */
	r5session_stop stop;
	/**
	* Optional override.
	*/
	r5session_set_video_coder set_video_coder;

}r5session;



/** Library, main API function pointer*/
typedef uint32_t(*pget_version)();
/** dll function pointer*/
typedef r5session* (*pcreate_session)();
/** dll function pointer*/
typedef void(*pclose_session)(r5session*);
/** dll function pointers*/
typedef struct libfuncs
{
	void* handle;
	pget_version get_version;
	pcreate_session create_session;
	pclose_session close_session;
}libfuncs;

/**
* Capture device hardware fail codes.
*/
typedef enum START_CODES
{
	S_OK,
	NO_DEVICE1,
	NO_DEVICE2,
	BAD_FORMAT,
	BUFFER_ALLOC_ERROR1,
	BUFFER_ALLOC_ERROR2,
	BUFFER_ACCESS_ERROR,
	UNKNOWN_ERROR
}START_CODES;


#endif /* R5PRO_API_H_ */
