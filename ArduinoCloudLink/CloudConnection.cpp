/*
* Copyright (c) 2015 Elipse Software
*
* MIT License
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>

#include "Config.h"
#include "CloudConnection.h"
#include "MobileClient.h"
#include "StrBuilder.h"
#include "StringEx.h"
#include <stdio.h>
#include "chrono.h"
#include "TagsControl.h"
#include "EventNotification.h"


#define CloudConnection_Cast(p) ((CloudConnection*)(p))


Result CloudConnection_Init(CloudConnection* pCloudConnection,
                            const char* nameConnection,
                            const char* connection,
                            int port,
                            const char* password)
{
  ASSERT(pCloudConnection != NULL);
#ifndef ARDUINO
  pCloudConnection->stop = false;
  pCloudConnection->thread = NULL;
#endif
  Result result = MobileClient_Init(&pCloudConnection->mobileClient);

  do
  {
    if (result == RESULT_OK)
    {
      result = MobileClient_Setup(&pCloudConnection->mobileClient,
                                  connection,
                                  port);

      if (result == RESULT_OK)
      {
        result = String_Init(&pCloudConnection->connectionName, nameConnection);

        if (result == RESULT_OK)
        {
          result = CloudExchange_Init(&pCloudConnection->cloudExchange, password);

          if (result == RESULT_OK)
          {
            result = StrBuilder_Init(&pCloudConnection->strBuilderInput, 100);

            if (result == RESULT_OK)
            {
              result = StrBuilder_Init(&pCloudConnection->strBuilderOutput, 100);

              if (result == RESULT_OK)
              {
                break;
              }

              StrBuilder_Destroy(&pCloudConnection->strBuilderInput);
            }

            CloudExchange_Destroy(&pCloudConnection->cloudExchange);
          }

          String_Destroy(&pCloudConnection->connectionName);
        }
      }

      MobileClient_Destroy(&pCloudConnection->mobileClient);
    }
  } while (0);

  return result;
}


void CloudConnection_Destroy(CloudConnection* pCloudConnection)
{
  ASSERT(pCloudConnection != NULL);
#ifndef ARDUINO
  pCloudConnection->stop = true;
  int res;
  thrd_join(pCloudConnection->thread, &res);
#endif
  StrBuilder_Destroy(&pCloudConnection->strBuilderOutput);
  StrBuilder_Destroy(&pCloudConnection->strBuilderInput);
  CloudExchange_Destroy(&pCloudConnection->cloudExchange);
  String_Destroy(&pCloudConnection->connectionName);
  MobileClient_Destroy(&pCloudConnection->mobileClient);
}


Result CloudConnection_Loop(CloudConnection* pCloudConnection)
{
  ASSERT(pCloudConnection != NULL);

  if (CloudExchange_HasSession(&pCloudConnection->cloudExchange))
  {
    Update(&pCloudConnection->cloudExchange);
  }

  CloudExchange_BeginExchange(&pCloudConnection->cloudExchange,
                              &pCloudConnection->strBuilderInput);
  RaiseEvent(EVENT_BEGIN_EXCHANGE, 0);
  Result result = MobileClient_Exchange(&pCloudConnection->mobileClient,
                                        pCloudConnection->connectionName,
                                        &pCloudConnection->strBuilderInput,
                                        &pCloudConnection->strBuilderOutput);
  RaiseEvent(EVENT_END_EXCHANGE, result);
  CloudExchange_EndExchange(&pCloudConnection->cloudExchange,
                            result,
                            &pCloudConnection->strBuilderOutput);
  return result;
}

#ifndef ARDUINO

static int Loop(void* p)
{
  ASSERT(p != NULL);
  CloudConnection* pCloudConnection = CloudConnection_Cast(p);
  int i = 0;

  for (;; i++)
  {
    if (pCloudConnection->stop)
    {
      break;
    }

    CloudConnection_Loop(pCloudConnection);
  }

  return 0;
}

int CloudConnection_Start(CloudConnection* pCloudConnection)
{
  ASSERT(pCloudConnection != NULL);
  thrd_t thread;
  thrd_create(&thread, &Loop, pCloudConnection);
  return 0;
}

int CloudConnection_Stop(CloudConnection*)
{
  return 0;
}

#endif


