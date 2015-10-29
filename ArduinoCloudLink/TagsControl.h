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
#include "CloudExchange.h"
#include "EventNotification.h"

#define ARDUINO_UNO "D1;D2;D3;D4;D5;D6;D7;D8;D9;D10;D11;D12;D13;A1;A2;A3;A4;A5"


typedef Result(*CustomReadFunc)(const char* tagName, float* value);
typedef Result(*CustomWriteFunc)(const char*tagName, float value);
void Setup(const char* s, CustomReadFunc fr, CustomWriteFunc fw);


Result Write(const char* tagName, float value);
Result Read(const char* tagName, float* value);

Result DefaultRead(const char*tagName, float*value);
Result DefaultWrite(const char*tagName, float value);

Result SubscribeTag(const char* tagName, bool isUsed);
Result UnsubscribeTag(const char* tagName);

Result Update(CloudExchange* pServerLink);

void DefaultLedEvents(Event e, size_t info);


