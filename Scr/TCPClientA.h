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



#pragma once

#include "Config.h"
#include "Result.h"
#include <Ethernet.h>


struct TCPClient
{
  EthernetClient* client;
};

Result TCPClient_Init(TCPClient* con);

void TCPClient_Destroy(TCPClient*);

Result TCPClient_Send(TCPClient*,
                      const char* buffer,
                      int len,
                      int* bytesSend);

Result TCPClient_Recv(TCPClient*,
                      char* buffer,
                      int len, int* bytesRead);

Result TCPClient_SendAll(TCPClient* con,
                         const char* buffer,
                         int len);

Result TCPClient_Connect(TCPClient* con,
                         const char* server,
                         int port,
                         int bSecure);


bool TCPClient_IsSecure(TCPClient* pTCPClient);
bool TCPClient_IsConnected(TCPClient* pTCPClient);

Result TCPClient_Close(TCPClient* pTCPClient);

Result TCPClient_SendStr(TCPClient* pTCPClient,
                         const char* pszString);
