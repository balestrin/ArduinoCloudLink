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
#include "JsonScanner.h"

struct tagCloudExchange;
typedef tagCloudExchange CloudExchange;


typedef Result(*OnTagListFunc)(Result result,
                               void* data,
                               const char* tagList);

typedef Result(*OnResultFunc)(Result result,
                              void* data,
                              const char* message);


Result GetTagList(CloudExchange* pCloudExchange,
                  const char* nodeId,
                  const char* filter,
                  bool flat,
                  OnTagListFunc callback,
                  void* data);

Result Subscribe(CloudExchange* pCloudExchange,
                 const char* tagName,
                 OnResultFunc callback,
                 void* data);

Result Unsubscribe(CloudExchange* pCloudExchange,
                   const char* tagName,
                   OnResultFunc callback,
                   void* data);

Result WriteTag(CloudExchange* pCloudExchange,
                const char* tagName,
                const char* jsonValue,
                OnResultFunc callback,
                void* data);

Result OnTagChanged(CloudExchange* pCloudExchange,
                    const char* tagName,
                    const char* valueJson,
                    int quality,
                    int timestamp,
                    OnResultFunc callback,
                    void* data);

Result DispatchCoreClient(JsonScanner* scanner,
                          CloudExchange* pCloudExchange);
