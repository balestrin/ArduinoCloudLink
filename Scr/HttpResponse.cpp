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
#include "config.h"
#include "HttpResponse.h"
#include "SocketStream.h"
#include "StringEx.h"
#include "StrBuilder.h"

static Result AppendUntilEndOfLine(SocketStream* pSocketStream,
                                   StrBuilder* pStrBuilder,
                                   int* /*socketError*/)
{
  ASSERT(pSocketStream != NULL);
  ASSERT(pStrBuilder != NULL);
  char ch;
  Result result;

  while ((result = SockStream_Get(pSocketStream, &ch)) == RESULT_OK)
  {
    if (ch == '\r')
    {
      result = StrBuilder_AppendChar(pStrBuilder, ch);

      if (result != RESULT_OK)
      {
        break;
      }

      result = SockStream_Get(pSocketStream, &ch);

      if (ch != '\n')
      {
        SockStream_PutBack(pSocketStream, ch);
      }

      StrBuilder_AppendChar(pStrBuilder, ch);

      if (result != RESULT_OK)
      {
        break;
      }

      break;
    }

    result = StrBuilder_AppendChar(pStrBuilder, ch);

    if (result != RESULT_OK)
    {
      break;
    }
  }

  return result;
}

static Result AppendUntil(SocketStream* pSocketStream,
                          StrBuilder* pStrBuilder,
                          char sep,
                          int* /*socketError*/)
{
  ASSERT(pSocketStream != NULL);
  ASSERT(pStrBuilder != NULL);
  char ch;
  Result result;

  while ((result = SockStream_Get(pSocketStream, &ch)) == RESULT_OK)
  {
    if (ch == sep)
    {
      result = StrBuilder_AppendChar(pStrBuilder, ch);
      break;
    }

    result = StrBuilder_AppendChar(pStrBuilder, ch);

    if (result != RESULT_OK)
    {
      break;
    }
  }

  return result;
}

enum HeaderType
{
  HEADER_TYPE_NONE,
  HEADER_TYPE_CONTENT_LENGTH,
  HEADER_TYPE_CONNECTION,
};

static HeaderType GetHeaderType(const char* header)
{
  ASSERT(header != NULL);
  HeaderType r = HEADER_TYPE_NONE;

  if (strcasecmp(header, "Content-Length:") == 0)
  {
    r = HEADER_TYPE_CONTENT_LENGTH;
  }
  else if (strcasecmp(header, "Connection:") == 0)
  {
    r = HEADER_TYPE_CONNECTION;
  }

  return r;
}

Result GetContent(SocketStream* pSocketStream,
                  size_t content_length,
                  StrBuilder* pStrBuilder,
                  int* /*socketError*/)
{
  ASSERT(pSocketStream != NULL);
  ASSERT(pStrBuilder != NULL);
  StrBuilder_Clear(pStrBuilder);//out

  SocketStream_ReadNBytes(pSocketStream, content_length);
  char ch;
  Result result;

  while ((result = SockStream_Get(pSocketStream, &ch)) == RESULT_OK)
  {
    result = StrBuilder_AppendChar(pStrBuilder, ch);

    if (result != RESULT_OK) {
      break;
    }
  }

  if (result == RESULT_EOF)
  {
    result = RESULT_OK;
  }

  return result;
}

Result HttpResponse_Init(HttpResponse* pHttpResponse,
                         TCPClient* pTCPClient)
{
  ASSERT(pHttpResponse != NULL);
  ASSERT(pTCPClient != NULL);
  pHttpResponse->pTCPClient = pTCPClient;
  pHttpResponse->contentLength = 0;
  pHttpResponse->httpResponseCode = 500;
  pHttpResponse->keepAlive = false;

  Result result = SocketStream_Init(&pHttpResponse->socketStream,
                                    pHttpResponse->pTCPClient);
  return result;
}

void HttpResponse_Destroy(HttpResponse* pHttpResponse)
{
  ASSERT(pHttpResponse != NULL);
  SocketStream_Destroy(&pHttpResponse->socketStream);  
}

Result ParseHttpHeader(SocketStream* pSocketStream,
                       StrBuilder* pStrBuilder,
                       size_t* pContentLength /*out*/,
                       int* httpResultCode,
                       bool* pbKeepAlive)
{
  ASSERT(httpResultCode != NULL);
  ASSERT(pContentLength != NULL);
  ASSERT(pSocketStream != NULL);
  ASSERT(pStrBuilder != NULL);

  *httpResultCode = 500;//TODO enum
  
  int socketError = 0;
  //http://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.2
  //HTTP/1.0
  Result result = AppendUntil(pSocketStream,
                              pStrBuilder,
                              ' ',
                              &socketError);

  if (result == RESULT_OK)
  {
    int iPos = pStrBuilder->size;
    //200
    result = AppendUntil(pSocketStream,
                         pStrBuilder,
                         ' ',
                         &socketError);

    if (result == RESULT_OK)
    {
      *httpResultCode = atoi(pStrBuilder->c_str + iPos);

      //OK
      result = AppendUntilEndOfLine(pSocketStream,
                                    pStrBuilder,
                                    &socketError);

      if (result == RESULT_OK)
      {
        for (;;)
        {
          char ch;
          result = SockStream_Get(pSocketStream, &ch);

          if (result == RESULT_OK)
          {
            if (ch == '\r')
            {
              ch = 0;
              result = SockStream_Get(pSocketStream, &ch);

              if (result == RESULT_OK)
              {
                break;
              }
              else
              {
                break;
              }
            }
            else
            {
              SockStream_PutBack(pSocketStream, ch);
            }

            size_t currentLen = pStrBuilder->size;
            result = AppendUntil(pSocketStream,
                                 pStrBuilder,
                                 ':',
                                 &socketError);
            HeaderType headerType = GetHeaderType(pStrBuilder->c_str + currentLen);

            if (result == RESULT_OK)
            {
              size_t currentLen2 = pStrBuilder->size;
              result = AppendUntilEndOfLine(pSocketStream,
                                            pStrBuilder,
                                            &socketError);

              switch (headerType)
              {
              case HEADER_TYPE_NONE:
                break;

              case HEADER_TYPE_CONTENT_LENGTH:
                *pContentLength = (size_t) atoi(pStrBuilder->c_str + currentLen2);
                break;

              case HEADER_TYPE_CONNECTION:              
              *pbKeepAlive = strstr(pStrBuilder->c_str, "keep-alive") != NULL;
                break;

              default:
                ASSERT(false);
              }
            }

            StrBuilder_Clear(pStrBuilder);
          }
        }
      }
    }
  }

  return result;
}

Result HttpResponse_ReceiveHeader(HttpResponse* pHttpResponse,
                                  StrBuilder* pStrBuilder)
{
  ASSERT(pHttpResponse != NULL);
  ASSERT(pStrBuilder != NULL);
  size_t content_length;
  int httpResponseCode = 0;
  bool bKeepAlive = false;
  Result result = ParseHttpHeader(&pHttpResponse->socketStream,
                                  pStrBuilder,
                                  &content_length,
                                  &httpResponseCode,
                                  &bKeepAlive);

  if (result == RESULT_OK)
  {
    pHttpResponse->contentLength = content_length;
    pHttpResponse->httpResponseCode = httpResponseCode;
    pHttpResponse->keepAlive = bKeepAlive;
  }
  else
  {
    StrBuilder_Clear(pStrBuilder);
  }

  return result;
}


Result HttpResponse_ReceiveContent(HttpResponse* pHttpResponse,
                                   StrBuilder* pStrBuilder)
{
  ASSERT(pHttpResponse != NULL);
  ASSERT(pStrBuilder != NULL);
  int socketError;
  Result result = GetContent(&pHttpResponse->socketStream,
                             pHttpResponse->contentLength,
                             pStrBuilder,
                             &socketError);
  return result;
}

