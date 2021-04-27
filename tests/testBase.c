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

#include "testBase.h"

void getChar_and_clear(){
  char c;
  do{
    c = getchar();
  } while (c != '\n' && c != EOF);
}

int strExact (const char* a, const char* b) {
  if ( strstr(a, b) == a && strlen(a) == strlen(b) ) {
    return 1;
  }
  return 0;
}

int start_test_by_name (const char* testName, libfuncs r5Pointer, cJSON* settings) {

  if (testName == NULL){
    return 0;
  }
  if (strExact(testName, "Publish")) {
    return start_publish_test(r5Pointer, settings);
  }
  if (strExact(testName, "Publish - Shared Object")) {
    return start_publish_shared_object_test(r5Pointer, settings);
  }
}

const char * string_for_status(int status); //hiding the big switch statement at the bottom

void print_status (int32_t status, const char* message) {
  printf("Received status: %s - with message: %s\n", string_for_status(status), message );
}

void set_config (cJSON* settings, r5config* conf) {

  //set default settings
  conf->audio = 0xFF;
  conf->video = 1;
  conf->width = 640;
  conf->height = 480;
  conf->video_format = fourcc('H','2','6','4');
  conf->audio_rate = 16000;
  conf->channel_count = 1;
  conf->video_bitrate = 750;
  conf->video_framerate = 30;
  conf->protocol = r5_rtsp;
  strcpy(conf->contextName, "live");
  strcpy(conf->hostName, "192.168.0.152");
  strcpy(conf->streamName, "stream1");
  conf->recordType = R5RecordTypeLive;
  conf->rtp_aux_out = NULL;
  conf->statusCallback = &print_status;
  conf->remoteCallback = NULL;

  //apply user specified settings
  cJSON* item = cJSON_GetObjectItemCaseSensitive(settings, "host");
  if (item != NULL) {
    strcpy(conf->hostName, item->valuestring);
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "video_device");
  if (item != NULL) {
    conf->video = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "width");
  if (item != NULL) {
    conf->width = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "height");
  if (item != NULL) {
    conf->height = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "video_bitrate");
  if (item != NULL) {
    conf->video_bitrate = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "fps");
  if (item != NULL) {
    conf->video_framerate = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "sample_rate");
  if (item != NULL) {
    conf->audio_rate = item->valueint;
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "context");
  if (item != NULL) {
    strcpy(conf->contextName, item->valuestring);
  }

  item = cJSON_GetObjectItemCaseSensitive(settings, "stream1");
  if (item != NULL) {
    strcpy(conf->streamName, item->valuestring);
  }
}

int isFirstNotNull(char* in1, char* in2, char* in3) {
  if (in1 == NULL || (in2 != NULL && in2 < in1) || (in3 != NULL && in3 < in1)) {
    return 0;
  }
  return 1;
}

static int forceEscapeAll = 0;

void sanitizeInputForJSON (char* input) { //sanitize in place
    unsigned int head = 0, escapes = 0, diff = 0;
    char *quoteNext = strchr(input, '\"');
    char *slashNext = strchr(input, '/');
    char *backSlashNext = strchr(input, '\\');
    char *hold = malloc(strlen(input)*2); //worst case scenario, need to escape everything
    char *point;

    while (quoteNext != NULL || slashNext != NULL || backSlashNext != NULL) {

      if (isFirstNotNull(backSlashNext, slashNext, quoteNext) && ! forceEscapeAll) {
        if(backSlashNext[1] == '\"' || backSlashNext[1] == '/' || backSlashNext[1] == '\\') {
          //already escaping something, copy in and move on
          diff = (backSlashNext + 2) - (input + head); //+2 catches the escaped character too
          memcpy(hold + head + escapes, input + head, diff);
          head += diff;
        }
        else{ //escape the escape character
          diff = (backSlashNext + 1) - (input + head); //+1 grabs the character itself
          memcpy(hold + head + escapes, input + head, diff);
          head += diff;
          hold[head + escapes] = '\\';
          escapes++; //now passed the newly escaped backslash
        }
      }
      else {
        if (isFirstNotNull(backSlashNext, slashNext, quoteNext)) {
          point = backSlashNext;
        }
        else if (isFirstNotNull(quoteNext, slashNext, backSlashNext)) {
          point = quoteNext;
        }
        else { //process of elimination
          point = slashNext;
        }
        diff = point - (input + head);
        memcpy(hold + head + escapes, input + head, diff);
        head += diff;
        hold[head + escapes] = '\\';
        head++;
        hold[head + escapes] = *point;
        escapes++;
      }

      quoteNext = strchr(input + head, '\"');
      slashNext = strchr(input + head, '/');
      backSlashNext = strchr(input + head, '\\');
    }

    if(head != 0){
      strcpy(hold + head + escapes, input + head);

      strcpy(input, hold);
    }
    free(hold);
}

//copied from global.c - which isn't available while we're using a shared library...
const char * string_for_status(int status) {
    switch(status){
        case r5_status_connected: return  "Connected";
        case r5_status_connection_close: return "Closed";
        case r5_status_connection_error: return "Error";
        case r5_status_connection_timeout: return "Timeout";
        case r5_status_disconnected: return "Disconnect";
        case r5_status_start_streaming: return "Started Streaming";
        case r5_status_stop_streaming: return "Stopped Streaming";
        case r5_status_netstatus: return "NetStatus";
        case r5_status_audio_mute: return "Audio Muted";
        case r5_status_audio_unmute: return "Audio Unmuted";
        case r5_status_video_mute: return "Video Muted";
        case r5_status_video_unmute: return "Video Unmuted";
        case r5_status_license_error: return "Invalid License";
        case r5_status_license_valid: return "Valid License";
        case r5_status_buffer_flush_start: return "Buffer Flush Started";
        case r5_status_buffer_flush_empty: return "Buffer Flush Empty";
        case r5_status_video_render_start: return "Video Render Started";
        case r5_status_abr_level_change: return "ABR Level Changed";
        case r5_status_srtp_key_gen_error: return "SRTP Error On Device";
        case r5_status_srtp_key_handle_error: return "SRTP Error In Server Response";
    }

    return "status not found";
}
