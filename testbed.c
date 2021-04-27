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

#include "librprosdk/r5pro_api.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include "tests/testBase.h"

#define SO_PATH "./librprosdk/libr5pro.so"

int main (int count, char **args) {
    //load settings
    FILE* file = fopen("tests.json", "r");
    if (!file) {
      printf("Error loading settings.\n" );
      return 0;
    }

    fseek(file, 0, SEEK_END);
    size_t fileLength = (size_t)ftell(file);
    char* fileContents = (char*) malloc(fileLength + 1);
    rewind(file);
    fread(fileContents, 1, fileLength, file);
    fileContents[fileLength] = 0;
    fclose(file);

    cJSON* baseJson = cJSON_Parse(fileContents);
    free(fileContents);
    if (!baseJson) {
      printf("JSON isn't parsing correctly. \n" );
      return 0;
    }

    cJSON* so_path_json = cJSON_GetObjectItemCaseSensitive(baseJson, "r5sdk_path");
    int pathLength = so_path_json ? strlen(so_path_json->valuestring) : 0;
    char* so_path_selected;
    if (pathLength < 1) {
      pathLength = strlen(SO_PATH);
      so_path_selected = malloc(pathLength + 1);
      memcpy(so_path_selected, SO_PATH, pathLength);
    }
    else {
      so_path_selected = malloc(pathLength + 1);
      memcpy(so_path_selected, so_path_json->valuestring, pathLength);
    }
    so_path_selected[pathLength] = 0;
    printf("Pulling red5pro from directory: %s\n", so_path_selected );

    cJSON* globalSettings = cJSON_DetachItemFromObjectCaseSensitive(baseJson, "GlobalProperties");
    cJSON* testList = cJSON_DetachItemFromObjectCaseSensitive(baseJson, "Tests");
    cJSON_Delete(baseJson);

    libfuncs pointer;
    pointer.handle = dlopen(so_path_selected, RTLD_LAZY);

    if (pointer.handle) {
      pointer.get_version = (pget_version)dlsym((void*)pointer.handle, "get_version");
      pointer.create_session = (pcreate_session)dlsym((void*)pointer.handle, "create_session");
      pointer.close_session = (pclose_session)dlsym((void*)pointer.handle, "close_session");
      uint32_t version = pointer.get_version();

      printf("Red5Pro SDK version %d.%d.%d  \n", ((version >> 16) & 0xFFFF) , ((version >> 8) & 0xFF) , ((version) & 0xFF));

      while (1) {
        int totalTests = 0;

        cJSON *test, *name, *description;
        cJSON_ArrayForEach(test, testList){
          name = cJSON_GetObjectItemCaseSensitive(test, "name");
          description = cJSON_GetObjectItemCaseSensitive(test, "Description");
          printf("%d) %s : %s\n", ++totalTests, name->valuestring, description->valuestring );
        }

        int ret = -1, selection = 0;
        while (totalTests > 0) {
          printf("Input the number that matches the desired example (0 to exit):\n");
          ret = scanf("%d", &selection);
          getChar_and_clear();
          if (ret != 1) {
            printf("Input was not a number\n");
          }
          else if (selection < 0 || selection > totalTests) {
            printf("Please input a number from 0 to %d\n", totalTests);
          }
          else {
            break;
          }
        }

        if (selection < 1 || selection > totalTests) {
          break;
        }
        else {
          //need logic for grabbing the LocalProperties of the test selected
          char* copyString = cJSON_PrintUnformatted(globalSettings);
          cJSON* combinedSettings = cJSON_Parse(copyString);
          free(copyString);
          cJSON* localData = cJSON_GetArrayItem(testList, selection - 1);
          cJSON* localSettings = cJSON_GetObjectItemCaseSensitive(localData, "LocalProperties");

          if (localSettings != NULL && cJSON_GetArraySize(localSettings) > 0) {
            cJSON *item, *itemCopy;
            cJSON_ArrayForEach(item, localSettings){
              if (cJSON_GetObjectItemCaseSensitive(combinedSettings, item->string) != NULL) {
                cJSON_DeleteItemFromObjectCaseSensitive(combinedSettings, item->string);
              }
              copyString = cJSON_PrintUnformatted(item);
              itemCopy = cJSON_Parse(copyString);
              free(copyString);
              cJSON_AddItemToObjectCS(combinedSettings, item->string, itemCopy);
            }
          }

          cJSON* localName = cJSON_GetObjectItemCaseSensitive(localData, "name");

          //individual test selection logic in testBase
          start_test_by_name(localName->valuestring, pointer, combinedSettings);

          sleep(1); // give some time for end of stream logs, so they don't overwrite the test list

          cJSON_Delete(combinedSettings);
        }
      }

      dlclose(pointer.handle);
    }
    else {
      printf("Could not load r5 library: %s\n", dlerror());
    }

    cJSON_Delete(globalSettings);
    cJSON_Delete(testList);

    return 0;
}
