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
#include "../../librprosdk/device.h"

int start_simple_publish (r5session* session) {

  r5device* vfl2 = (r5device*)malloc(sizeof(r5device));
  make_vfl2_device(vfl2);
  session->set_r5camera(session->mem_ptr, vfl2);
  if (!session->init(session->mem_ptr, session->config)) {
    session->start(session->mem_ptr);
    return 0; // publishing successfully
  } else {
    return 1; // there was a problem
  }
}

int start_publish_test (libfuncs r5Pointer, cJSON* settings) {

  printf("Press ENTER to begin publishing, and then again to exit\n");
  getChar_and_clear();

  r5session* session = r5Pointer.create_session();

  int err = 0;
  if (session && session->mem_ptr) {
      printf("Session created\n ");
      set_config(settings, session->config);

      err = start_simple_publish(session);
      if(!err){
      //wait for user input, then end
        getChar_and_clear();

        session->stop(session->mem_ptr);
      }
      else {
        printf("There was a problem starting the stream - Test aborted\n");
      }

      r5device* cam = session->get_r5camera(session->mem_ptr);

      r5Pointer.close_session(session);

      free(cam);
  } else {
    printf("There was a problem creating the session - Test aborted\n");
    return 0;
  }

  return 1;
}
