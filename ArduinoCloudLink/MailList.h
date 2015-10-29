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
#include "StrBuilder.h"
#include "JsonScanner.h"
#include "Config.h"
#include "chrono.h"
#include <stdlib.h>
#include "StringEx.h"

typedef enum
{
  FUNC_NONE = 0,
  FUNC_SUBSCRIBE = 1,
  FUNC_UNSUBSCRIBE = 2,
  FUNC_GETTAGLIST = 3,
  FUNC_WRITETAG = 4,
  FUNC_ONTAGCHANGED = 5,
  FUNC_NOT_USED1 = 6,
  FUNC_ONTAGLIST = 7,
  FUNC_RESET = 8,
  FUNC_LOGIN = 9,
  FUNC_ACCESS_DENIED = 10,
  FUNC_ONTAGCHANGEDRESULT = 11,
  FUNC_ON_LOGIN_RESULT = 12,
  FUNC_ON_WRITETAG_RESULT = 13,
  FUNC_ON_SUBSCRIBE_RESULT = 14,
  FUNC_ON_UNSUBSCRIBE_RESULT = 15,
} Functions;

#ifdef POST_MAIL_STATUS
const char* GetFunctionName(Functions f);
#endif

typedef struct tagMail
{
  Functions FuncID;
  int       msgId;
  void*     callback;
  void*     data;
  StringC   params;
  struct tagMail* pNext;

  time_t    timestamp;
  bool      bWaitingResponse;
} Mail;

Result Mail_Create(Mail** ppMail, bool bWaitingResponse, int idOpt);
void Mail_Delete(Mail* pMail);
void Mail_Cancel(Mail* pMail, Result result);
void Mail_UpdateStatus(Mail* pMail, const char* psz);


typedef struct 
{
  Mail* pHead;
} MailList;


Result MailList_Init(MailList* pMailList);
void MailList_Destroy(MailList* pMailList);
Mail* MailList_PopMail(MailList* pMailList, int id);
void MailList_Reset(MailList* pMailList, Result result);

Result MailList_AppendMove(MailList* pMailList, Mail** pMail);

void MailList_UpdateStatus(MailList *pMail, const char* psz);

Result MailList_GetPostMainString(MailList* pMail,
                                  const char* sessionToken,
                                  StrBuilder* pStrBuilder);
Result MailList_RemoveNotWaiting(MailList* pMailList);
Result MailList_RemoveExpired(MailList* pMailList, int maxAgeSec);

Result MailList_AppendListMoved(MailList* pMailList, MailList* pMailListSource);
