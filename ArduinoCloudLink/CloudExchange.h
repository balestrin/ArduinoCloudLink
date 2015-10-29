
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
#include "StrBuilder.h"
#include "JsonScanner.h"
#include "chrono.h"
#include "MailList.h"
#include "StringEx.h"

#ifdef SERVER_SIDE
#include "CloudExchangeS.h"
#else
#include "CloudExchangeC.h"
#endif


Result Login(CloudExchange* pCloudExchange,
             const char* password,
             OnResultFunc callback,
             void* data);

Result OnReset(CloudExchange* pCloudExchange);


struct tagCloudExchange
{
  MailList  postedMailList;
  MailList  sendingMailList;
  MailList  sentMailList;

  //
  void* pContext;
  int   id;
  Functions func;
  //

  time_t lastExchange;
  StringS sessionToken[20];
  StringS password[20];
};

Result CloudExchange_DeliveryMails(CloudExchange* pCloudExchange,
                                   StrBuilder* strBuilderInput);

Result CloudExchange_Init(CloudExchange* pCloudExchange,
                          const char* password);

void CloudExchange_Destroy(CloudExchange* pCloudExchange);

void CloudExchange_DestroyEx(CloudExchange* pCloudExchange,
                             Result result);

Result CloudExchange_SetPassword(CloudExchange* pCloudExchange,
                                 const char* password);

Mail* CloudExchange_PopSentMail(CloudExchange* pCloudExchange,
                                int id);

Result CloudExchange_Reset(CloudExchange* pCloudExchange,
                           Result result);

Result CloudExchange_BeginExchange(CloudExchange* pCloudExchange,
                                   StrBuilder* strBuilder);

Result CloudExchange_EndExchange(CloudExchange* pCloudExchange,
                                 Result exchangeResult,
                                 StrBuilder* strBuilder);

Result CloudExchange_OnTimer(CloudExchange* pCloudExchange);

bool CloudExchange_HasSession(CloudExchange* pCloudExchange);

Result  CloudExchange_PostMailMove(CloudExchange* pCloudExchange,
                                   Mail** ppMail);


Result HandleMail(JsonScanner* scanner,
                  const char* format,
                  ...);

Result PostMail(CloudExchange* pCloudExchange,
                Functions funcID,
                OnResultFunc callback,
                void* data,
                bool bWaitingResponse,
                int idOpt,
                const char* format,
                ...);

void ExtractString(char* dest,
                          int destSize,
                          const char* source);



Result HandleOnResult(JsonScanner* scanner,
                      CloudExchange* pCloudExchange,
                      int msgId);

Result PostOnResult(Result result,
                    void* data,
                    const char* message);
