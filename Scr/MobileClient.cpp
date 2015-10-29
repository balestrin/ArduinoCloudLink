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
#include "Config.h"
#include <stdio.h>
#include "MobileClient.h"
#include "HttpClient.h"
#include "StringEx.h"
#include "TCPClient.h"
#include "StrBuilder.h"


Result MobileClient_Init(MobileClient* pMobileClient)
{
  ASSERT(pMobileClient != NULL);
  Result result = TCPClient_Init(&pMobileClient->tcpClient);
  
  if (result == RESULT_OK)
  {
    String_Init(&pMobileClient->server, NULL);
    pMobileClient->port = 0;
  }

  return result;
}

void MobileClient_Destroy(MobileClient* pMobileClient)
{
  ASSERT(pMobileClient != NULL);

  TCPClient_Destroy(&pMobileClient->tcpClient);
  String_Destroy(&pMobileClient->server);
}


Result MobileClient_Setup(MobileClient* pMobileClient,
                          const char* server,
                          int port)
{
  ASSERT(pMobileClient != NULL);

  Result result = String_Change(&pMobileClient->server,
                                server);
  if (result == RESULT_OK)
  {
    pMobileClient->port = port;
  }

  return result;
}

Result MobileClient_Exchange(MobileClient* pMobileClient,
                             const char* connectionName,
                             StrBuilder* pStrBuilderIn,
                             StrBuilder* pStrBuilderOut)
{
  ASSERT(pMobileClient != NULL);
  ASSERT(pStrBuilderIn != NULL);
  ASSERT(pStrBuilderOut != NULL);
  ASSERT(!IsStrEmpty(connectionName));
  ASSERT(!IsStrEmpty(pMobileClient->server));
  StrBuilder_Clear(pStrBuilderOut); //out

  Result result = TCPClient_Connect(&pMobileClient->tcpClient,
                                    pMobileClient->server,
                                    pMobileClient->port,
                                    false);

  if (result == RESULT_OK)
  {    
    result = SendJson(&pMobileClient->tcpClient,
                      connectionName,
                      pStrBuilderIn->c_str,
                      pMobileClient->server);
    
    if (result == RESULT_OK)
    {
      int httpResponseCode = 0;
      bool bKeepAlive = false;

      result = ReceiveContent(&pMobileClient->tcpClient,
                              pStrBuilderOut,
                              &httpResponseCode,
                              &bKeepAlive);
      if (result == RESULT_OK)
      {
        if (!bKeepAlive)
        {
          TCPClient_Close(&pMobileClient->tcpClient);
        }

        if (httpResponseCode != 200 &&
            httpResponseCode != 304)
        {
          result = RESULT_FAIL;
        }
      }
    }    
  }

  if (result != RESULT_OK)
  {
    TCPClient_Close(&pMobileClient->tcpClient);
  }

  return result;
}

