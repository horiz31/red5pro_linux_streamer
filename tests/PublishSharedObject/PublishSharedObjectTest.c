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

#include "../testBase.h"

//static variables to keep track of things
r5session* session = NULL;
r5shared_object* so = NULL;
int soReady = 0;

void sharedObjectTestCallback (const char* name, const char* method, cJSON* params) {
  if (session != NULL) {
    if (strExact(method, "onSharedObjectConnect")) {
      soReady = 1;
      printf("Shared Object Connected, you may send when ready\n");
    }
    else if (strExact(method, "onUpdateProperty")) {
      cJSON* item;
      char* itemPrint;
      cJSON_ArrayForEach(item, params) {
        itemPrint = cJSON_PrintUnformatted(item);
        printf("A property named \"%s\" was updated to %s\n", item->string, itemPrint );
        free(itemPrint);
      }
    }
    else if (strExact(method, "messageTransmit")) {
      char* paramPrint = cJSON_PrintUnformatted(params);
      printf("Received message from Shared Object: %s\n", paramPrint);
      free(paramPrint);
    }
  }
}

void so_status_response (int32_t status, const char* message) {
  print_status(status, message);

  if (status == r5_status_start_streaming) { //if start_streaming - connect the shared object
    printf("Stream Connected, now connecting Shared Object\n");

    r5connection* con = session->get_r5connection(session->mem_ptr);
    so = con->get_sharedobject(con->mem_ptr, "sharedChatTest");
    so->set_callback(so->session, &sharedObjectTestCallback);
    so->connect(so->session);
  }
}

void parse_so_input () {
  int buffSize = 4096; //about as large of a message as we can send, should be enough
  char buffer[buffSize];
  char *ret, *sendFind, *setFind, *temp;
  cJSON *jsonOut, *jsonTemp;
  while (1) {
    memset(buffer, 0, buffSize); //probably unnecessary, but just in case
    //get console input with fgets()
    ret = buffer;
    ret = fgets(buffer, buffSize, stdin);
    if (ret == NULL || strstr(buffer, "end") == buffer) {
      //end-of-file character breaks, or look for 'end' command
      return;
    }

    //parse command
    setFind = strstr(buffer, "set");
    sendFind = strstr(buffer, "send");

    if (setFind != buffer && sendFind != buffer) {
      //didn't find an action we want to respond to
      printf("Action not understood, please try again\n");
      continue;
    }

    if (soReady == 0) {
      printf("Shared Object not yet connected, please wait\n");
      continue;
    }

    ret = strchr(buffer, '\n');
    if(ret != NULL) {
      *ret = 0;
    }

    sanitizeInputForJSON(buffer);

    ret = strchr(buffer, ' ');
    if (ret != NULL) {
      ret++;
    }
    else {
      ret = buffer + (setFind == buffer ? 3 : 4);
    }

    if (setFind == buffer) { //set
      temp = strstr(ret, "=");
      if(temp == NULL){
        printf("Invalid format - set value must be in <key>=<jsonvalue> form\n");
        continue;
      }
      temp[0] = 0;
      temp++;
      jsonTemp = cJSON_Parse(temp);
      so->set_property(so->session, ret, jsonTemp);
    }
    else { //send
      temp = malloc(strlen(buffer) + 64);
      sprintf(temp, "[{\"message\":\"%s\",\"user\":\"linuxTestbed\"}]", ret);
      jsonTemp = cJSON_Parse(temp);
      free(temp);

      so->send(so->session, "messageTransmit", jsonTemp);

      cJSON_Delete(jsonTemp);
    }
  }
}

int start_publish_shared_object_test (libfuncs r5Pointer, cJSON* settings) {

  if (session != NULL) {
    printf("The previous test is still in progress or wasn't cleaned up properly. Aborting\n");
    return 0;
  }

  soReady = 0;

  printf("Press ENTER to begin publishing\n"
         "Once connected to the stream, the Shared Object will connect\n"
         "Use \"send <message>\" to send a message to all connected clients\n"
         "Use \"set <key>=<jsonvalue>\" to set a property to the shared object\n"
         "Use \"end\" to exit\n");
  getChar_and_clear();

  session = r5Pointer.create_session();

  int err = 0;

  if (session && session->mem_ptr) {
      printf("Session created\n ");
      set_config(settings, session->config);
      session->config->statusCallback = &so_status_response;
      soReady = 0;

      err = start_simple_publish(session);
      if (!err) {
        //wait for user input, then end
        parse_so_input();

        session->stop(session->mem_ptr);
      }
      else {
        printf("There was a problem starting the stream - Test aborted\n");
      }

      r5device* cam = session->get_r5camera(session->mem_ptr);

      r5Pointer.close_session(session);
      so = NULL;
      session = NULL;

      free(cam);
  } else {
    printf("There was a problem creating the session - Test aborted\n");
    return 0;
  }

  return !err;
}
