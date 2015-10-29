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
#include "Result.h"
#include "MobileClient.h"
#include "CloudExchange.h"
#include "StringEx.h"

#ifndef ARDUINO
#include "tinycthread.h"
#endif

struct CloudConnection
{
  MobileClient  mobileClient;
  StringC       connectionName;
  CloudExchange cloudExchange;
  StrBuilder    strBuilderInput;
  StrBuilder    strBuilderOutput;

#ifndef ARDUINO
  bool stop;
  thrd_t thread;
#endif
};

 
Result CloudConnection_Init(CloudConnection* pCloudConnection, 
                            const char* nameConnection,
                            const char *connection, 
                            int port, 
                            const char *password);

void CloudConnection_Destroy(CloudConnection* pCloudConnection);
Result CloudConnection_Loop(CloudConnection* pCloudConnection);


#ifndef ARDUINO
int CloudConnection_Start(CloudConnection*);
int CloudConnection_Stop(CloudConnection*);
#endif



