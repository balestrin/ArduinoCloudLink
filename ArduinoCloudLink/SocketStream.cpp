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
#include "SocketStream.h"
#include <limits.h>

Result SocketStream_Init(SocketStream* pSocketStream,
                         TCPClient* pTCPClient)
{
  ASSERT(pSocketStream != NULL);
  ASSERT(pTCPClient != NULL);
  pSocketStream->current_pos = 0;
  pSocketStream->has_put_back = false;
  pSocketStream->put_back_character = '\0';
  pSocketStream->max_pos = INT_MAX;
  pSocketStream->tcp_connection = pTCPClient;
  pSocketStream->read_so_far = 0;
  
  StringS_Init(pSocketStream->buffer);
  pSocketStream->buffer_size = StringS_Size(pSocketStream->buffer);

  pSocketStream->pcurrent = pSocketStream->buffer;
  pSocketStream->data_size = 0;
  return RESULT_OK;
}

void SocketStream_Destroy(SocketStream*)
{
  StringS_Destroy(&pSocketStream->buffer);
}

void SockStream_PutBack(SocketStream* pSocketStream, char ch)
{
  ASSERT(pSocketStream != NULL);
  pSocketStream->has_put_back = true;
  pSocketStream->put_back_character = ch;
}

Result SocketStream_ReadNBytes(SocketStream* pSocketStream,
                               int content_length)
{
  ASSERT(pSocketStream != NULL);
  pSocketStream->max_pos = pSocketStream->current_pos + content_length;
  return RESULT_OK;
}

Result SockStream_Get(SocketStream* pSocketStream,
                      char* pChar)
{
  ASSERT(pSocketStream != NULL);
  ASSERT(pChar != NULL);

  if (pSocketStream->has_put_back)
  {
    *pChar = pSocketStream->put_back_character;
    pSocketStream->has_put_back = false;
    pSocketStream->put_back_character = '\0';
    return RESULT_OK;
  }

  if (pSocketStream->current_pos == pSocketStream->max_pos)
  {
    return RESULT_EOF;
  }

  if (pSocketStream->buffer == pSocketStream->pcurrent ||
      pSocketStream->pcurrent == pSocketStream->buffer + pSocketStream->buffer_size)
  {
    pSocketStream->pcurrent = pSocketStream->buffer;
    int bytes_read = 0;
    Result er = TCPClient_Recv(pSocketStream->tcp_connection,
                               pSocketStream->buffer,
                               pSocketStream->buffer_size,
                               &bytes_read);

    if (er != RESULT_OK)
    {
      pSocketStream->read_so_far = 0;
      pSocketStream->data_size = 0;
      return er;
    }

    pSocketStream->read_so_far += bytes_read;
    pSocketStream->data_size = (size_t)bytes_read;
  }

  pSocketStream->current_pos++;
  *pChar = *pSocketStream->pcurrent;
  pSocketStream->pcurrent++;
  return RESULT_OK;
}



