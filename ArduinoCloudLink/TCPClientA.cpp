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
#include "TCPClient.h"
#include <string.h>

#ifdef ARDUINO

Result TCPClient_Init(TCPClient* pTCPClient)
{
  ASSERT(pTCPClient != NULL);
  pTCPClient->client = new EthernetClient();
 
  return RESULT_OK;
}

Result TCPClient_SendAll(TCPClient* pTCPClient,
                         const char* pBuffer,
                         int len)
{
  ASSERT(pTCPClient != NULL);
  ASSERT(pBuffer != NULL);

  Result result = RESULT_OK;
  int count = 0;

  while (count < len)
  {
    int bytesSent = 0;
    result = TCPClient_Send(pTCPClient,
                            pBuffer + count,
                            len - count,
                            &bytesSent);

    if (result != RESULT_OK)
    {
      break;
    }

    count += bytesSent;
  }

  return result;
}

Result TCPClient_SendStr(TCPClient* pTCPClient,
                         const char* pszString)
{
  ASSERT(pTCPClient != NULL);
  ASSERT(pszString != NULL);

  return TCPClient_SendAll(pTCPClient,
                           pszString,
                           strlen(pszString));
}


Result TCPClient_Connect(TCPClient* pTCPClient,
                         const char* server,
                         int port,
                         int bSecure)
{
  if (TCPClient_IsConnected(pTCPClient))
  {
    return RESULT_OK;
  }

  if (bSecure)
  {
    return RESULT_FAIL;
  }


  if (pTCPClient->client->connect(server, port))
  {       
    return RESULT_OK;
    
  }  
  return RESULT_FAIL;  
}


Result TCPClient_Send(TCPClient* pTCPClient,
                      const char* psz,
                      int len,
                      int* bytesSent)
{
  if (!pTCPClient->client->connected())
  {
    return RESULT_FAIL;
  }

  Result r = RESULT_OK;
  *bytesSent = pTCPClient->client->write(psz, len);


  return r;
}

Result TCPClient_Recv(TCPClient* pTCPClient,
                      char* psz,
                      int len,
                      int* bytesRead)
{
  *bytesRead = 0; //out

  if (len == 0 || psz == NULL)
  {
    return RESULT_FAIL;
  }
  
  Result result = RESULT_OK;
  int count = 0;
  char* p = psz;

  while (!pTCPClient->client->available())
  {
    if (!pTCPClient->client->connected())
    {
      return RESULT_EOF;
    }
  }

  while (pTCPClient->client->available())
  {
    if (!pTCPClient->client->connected())
    {
       result = RESULT_FAIL;
      break;
    }

    char ch = pTCPClient->client->read();
    *p = ch;

    p++;
    count++;

    if (len == count)
    {
      break;
    }
  }

  *bytesRead = count;
  return result;
}

bool TCPClient_IsConnected(TCPClient* pTCPClient)
{
  ASSERT(pTCPClient != NULL);
  return pTCPClient->client->connected();
}

Result TCPClient_Close(TCPClient* pTCPClient)
{
  if (pTCPClient && pTCPClient->client)
  {
    pTCPClient->client->stop();
  }
  return RESULT_OK;
}

void TCPClient_Destroy(TCPClient* pTCPClient)
{
  TCPClient_Close(pTCPClient);
  if (pTCPClient->client)
  {
    delete pTCPClient->client;
    pTCPClient->client = NULL;
  }
}

#endif
