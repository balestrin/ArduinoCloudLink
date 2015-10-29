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
#include "HttpClient.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "TCPClient.h"
#include "HttpResponse.h"
#include "StringEx.h"

Result HttpSync(TCPClient* pClient,
                const char* httpRequestText,
                int httpRequestTextLength,
                StrBuilder* strBuilder)
{
  Result result = TCPClient_SendAll(pClient,
                                    httpRequestText,
                                    httpRequestTextLength);

  if (result != RESULT_OK)
  {
    return result;
  }

  HttpResponse httpResponse;
  result = HttpResponse_Init(&httpResponse, pClient);

  if (result == RESULT_OK)
  {
    StrBuilder_Clear(strBuilder);//out
    result = HttpResponse_ReceiveHeader(&httpResponse, strBuilder);

    if (result == RESULT_OK)
    {
      StrBuilder_Clear(strBuilder);//reuse

      result = HttpResponse_ReceiveContent(&httpResponse, strBuilder);

      if (result != RESULT_OK)
      {
        StrBuilder_Clear(strBuilder);
      }
    }

    HttpResponse_Destroy(&httpResponse);
  }

  return result;
}


Result SendJson(TCPClient* pTCPClient,
                const char* connectionName,
                const char* pszOutBox,
                const char* hostName)
{
 
  ASSERT(pTCPClient != NULL);
  ASSERT(pszOutBox != NULL);
  ASSERT(!IsStrEmpty(connectionName));

  Result result;
  IER(TCPClient_SendStr(pTCPClient, "POST /api/msg/"));
  IER(TCPClient_SendStr(pTCPClient, connectionName));
  
  IER(TCPClient_SendStr(pTCPClient, " HTTP/1.1\r\n"
                        "Content-Type: application/json;charset=utf8\r\n"
                        "Connection: keep-alive\r\n"
                        "Host: "));

  IER(TCPClient_SendStr(pTCPClient, hostName));
  IER(TCPClient_SendStr(pTCPClient, "\r\n"));

  IER(TCPClient_SendStr(pTCPClient,"Content-Length: "));
  char itoabuffer[20];
  IER(IntToStr(strlen(pszOutBox), itoabuffer, 20));
  IER(TCPClient_SendStr(pTCPClient, itoabuffer));
  IER(TCPClient_SendStr(pTCPClient, "\r\n\r\n"));
  IER(TCPClient_SendStr(pTCPClient, pszOutBox));
  return result;
}

Result ReceiveContent(TCPClient* pClient,
                      StrBuilder* strBuilder,
                      int* httpResponseCode,
                      bool* pbKeepAlive)
{
  *httpResponseCode = 0;//out mesmo em caso de falha
  *pbKeepAlive = 0; //out mesmo em caso de falha

  StrBuilder_Clear(strBuilder);//out
  HttpResponse httpResponse;
  Result result = HttpResponse_Init(&httpResponse, pClient);

  if (result == RESULT_OK)
  {
    result = HttpResponse_ReceiveHeader(&httpResponse, strBuilder);

    if (result == RESULT_OK)
    {
      result = HttpResponse_ReceiveContent(&httpResponse, strBuilder);

      *httpResponseCode = httpResponse.httpResponseCode;
      *pbKeepAlive = httpResponse.keepAlive;
    }
    
    HttpResponse_Destroy(&httpResponse);
  }

  return result;
}

