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
#include "MailList.h"
#include "CloudExchange.h"
#include "StringEx.h"
#include "chrono.h"


#ifdef POST_MAIL_STATUS
const char* GetFunctionName(Functions f)
{
  switch (f)
  {
  case FUNC_NONE:
    return "None";

  case   FUNC_SUBSCRIBE:
    return "Subscribe";

  case FUNC_UNSUBSCRIBE:
    return "Unsubscribe";

  case FUNC_GETTAGLIST:
    return "GetTagList";

  case FUNC_WRITETAG:
    return "WriteTag";

  case FUNC_ONTAGCHANGED:
    return "OnTagChanged";

  case  FUNC_ONTAGCHANGEDRESULT:
    return "OnTagChangedResult";

  case FUNC_ON_LOGIN_RESULT:
    return "OnLoginResult";

  case FUNC_ON_WRITETAG_RESULT:
    return "OnWriteResult";

  case FUNC_ON_SUBSCRIBE_RESULT:
    return "OnSubscribeResult";

  case FUNC_ON_UNSUBSCRIBE_RESULT:
    return "OnUnsubscribeResult";

  case FUNC_ONTAGLIST:
    return "OnTagList";

  case FUNC_RESET:
    return "Reset";

  case FUNC_LOGIN:
    return "Login";

  case FUNC_ACCESS_DENIED:
    return "AccessDenied";

  default:
    ASSERT(0);
  }

  return "?";
}
#endif



#ifdef SERVER_SIDE
//ids do server são pares
#define ID_BEGIN  2
#else
//ids do client são impares
#define ID_BEGIN  1
#endif

Result Mail_Init(Mail* pMail,
                 bool bWaitingResponse,
                 int idOpt)
{
  static int s_id = ID_BEGIN;

  String_Init(&pMail->params, NULL);
  
  pMail->callback = NULL;  
  pMail->pNext = NULL;
  pMail->data = NULL;
  
  if (idOpt == 0)
  {
    pMail->msgId = s_id;
    s_id = s_id + 2;
  }
  else
  {
    pMail->msgId = idOpt;
  }

  pMail->FuncID = FUNC_NONE;
  pMail->timestamp = time(NULL);
  pMail->bWaitingResponse = bWaitingResponse;
  
#ifdef POST_MAIL_STATUS
  AddPost("Mail", "");
  StrBuilder strBuilder;
  StrBuilder_Init(&strBuilder, 20);
  StrBuilder_Append(&strBuilder, "Mail.");
  StrBuilder_AppendInt(&strBuilder, (int)pMail);
  AddPost(strBuilder.c_str, "");
  StrBuilder_Destroy(&strBuilder);
#endif

  return RESULT_OK;
}

Result Mail_Create(Mail** ppMail,
                   bool bWaitingResponse,
                   int idOpt)
{
  ASSERT(*ppMail == NULL);
  Mail* p = (Mail*)Malloc(sizeof(Mail) * 1);
  Result result = RESULT_OUT_OF_MEM;

  if (p)
  {
    Mail_Init(p, bWaitingResponse, idOpt);
    *ppMail = p;
    result = RESULT_OK;
  }

  return result;
}

void Mail_Destroy(Mail* pMail)
{
  String_Destroy(&pMail->params);

#ifdef POST_MAIL_STATUS
  StrBuilder strBuilder;
  StrBuilder_Init(&strBuilder, 20);
  StrBuilder_Append(&strBuilder, "Mail.");
  StrBuilder_AppendInt(&strBuilder, (int)pMail);
  RemovePost(strBuilder.c_str, "");
  StrBuilder_Destroy(&strBuilder);
#endif
}

void Mail_Delete(Mail* pMail)
{
  if (pMail)
  {
    ASSERT(pMail->data == NULL);
    ASSERT(pMail->callback == NULL);

    Mail_Destroy(pMail);
    Free(pMail);
  }
}

void Mail_Cancel(Mail* pMail, Result result)
{
  if (pMail->callback)
  {
    switch (pMail->FuncID)
    {
    case FUNC_RESET:
    case FUNC_ONTAGCHANGED:
    case FUNC_WRITETAG:
    case FUNC_UNSUBSCRIBE:
    case FUNC_SUBSCRIBE:
    case FUNC_LOGIN:
    {
      OnResultFunc f = (OnResultFunc)pMail->callback;

      if (f)
      {
        f(result, pMail->data, "");
      }
    }
    break;

    case FUNC_GETTAGLIST:
    {
      OnTagListFunc f = (OnTagListFunc)pMail->callback;

      if (f)
      {
        f(RESULT_CANCELED, pMail->data, "");
      }
    }
    break;

    case  FUNC_ONTAGCHANGEDRESULT:
    case  FUNC_ON_LOGIN_RESULT:
    case  FUNC_ON_WRITETAG_RESULT:
    case  FUNC_ON_SUBSCRIBE_RESULT:
    case  FUNC_ON_UNSUBSCRIBE_RESULT:
      break;

    case FUNC_ONTAGLIST:
    {
      OnTagListFunc f = (OnTagListFunc)pMail->callback;

      if (f)
      {
        f(result, pMail->data, "");
      }
    }
    break;

    default:
      ASSERT(0);
    }
  }

  //O cancelamento move data e f
  pMail->data = NULL;
  pMail->callback = NULL;
}


void Mail_UpdateStatus(Mail* pMail, const char* psz)
{
  psz;
  pMail;
#ifdef POST_MAIL_STATUS
  StrBuilder strBuilder;
  StrBuilder_Init(&strBuilder, 20);
  StrBuilder_Append(&strBuilder, "Mail.");
  StrBuilder_AppendInt(&strBuilder, (int)pMail);
  StrBuilder strBuilder2;
  StrBuilder_Init(&strBuilder2, 20);
  StrBuilder_Append(&strBuilder2, GetFunctionName(pMail->FuncID));
  StrBuilder_Append(&strBuilder2, "(");
  StrBuilder_Append(&strBuilder2, pMail->params);
  StrBuilder_Append(&strBuilder2, ")");
  StrBuilder_Append(&strBuilder2, " ");
  StrBuilder_AppendInt(&strBuilder2, pMail->msgId);
  StrBuilder_Append(&strBuilder2, " ");
  StrBuilder_AppendInt(&strBuilder2, (int)pMail->timestamp);
  StrBuilder_Append(&strBuilder2, " ");
  StrBuilder_Append(&strBuilder2, pMail->bWaitingResponse ? "w" : "not w");
  StrBuilder_Append(&strBuilder2, psz);
  AddPost(strBuilder.c_str, strBuilder2.c_str);
  StrBuilder_Destroy(&strBuilder);
  StrBuilder_Destroy(&strBuilder2);
#endif
  
}

Result MailList_AppendListMoved(MailList* pMailList,
                                MailList* pMailListSource)
{
  return MailList_AppendMove(pMailList, &pMailListSource->pHead);  
}


Result MailList_AppendMove(MailList* pMailList, Mail** pMail)
{
  if (pMailList->pHead == NULL)
  {
    pMailList->pHead = *pMail;
  }
  else
  {
    Mail* current = pMailList->pHead;

    while (current->pNext != NULL)
    {
      current = current->pNext;
    }

    /* now we can add a new variable */
    current->pNext = *pMail;
  }

  *pMail = NULL; //moved
  
  return RESULT_OK;
}


Mail* MailList_PopMail(MailList* pMailList, int id)
{  
  Mail *prev = NULL;
  Mail *temp = pMailList->pHead;
  while (temp != NULL)
  {
    if (temp->msgId == id)
    {
      if (temp == pMailList->pHead)
      {
        pMailList->pHead = temp->pNext;        
        return temp;
      }
      else
      {
        prev->pNext = temp->pNext;        
        return temp;
      }
    }
    else
    {
      prev = temp;
      temp = temp->pNext;
    }
  }
  return NULL;
}

Result MailList_Init(MailList* pMailList)
{
  pMailList->pHead = NULL;
  return RESULT_OK;
}

void MailList_Destroy(MailList* pMailList)
{
  MailList_Reset(pMailList, RESULT_CANCELED);
}

void MailList_Reset(MailList* pMailList, Result result)
{
  Mail* pCurrent = pMailList->pHead;

  while (pCurrent)
  {
    Mail* pTemp = pCurrent;
    pCurrent = pCurrent->pNext;
    Mail_Cancel(pTemp, result);
    Mail_Delete(pTemp);
  }

  pMailList->pHead = NULL;
}

void MailList_UpdateStatus(MailList* pMailList, 
                           const char* psz)
{
  Mail* pCurrent = pMailList->pHead;

  while (pCurrent)
  {
    Mail_UpdateStatus(pCurrent, psz);
    pCurrent = pCurrent->pNext;
  }
}


Result MailList_GetPostMainString(MailList* pMailList,
                                  const char* sessionToken,
                                  StrBuilder* pStrBuilder)
{
  ASSERT(pMailList);
  ASSERT(pStrBuilder);
  
  Result result = RESULT_FAIL;
  StrBuilder_Clear(pStrBuilder);
  IER(StrBuilder_Append(pStrBuilder, "["));
#ifndef SERVER_SIDE
  IER(StrBuilder_AppendQuotedJsonStr(pStrBuilder, sessionToken));
  IER(StrBuilder_Append(pStrBuilder, ","));
#else
  sessionToken;
#endif
  bool bFirst = true;
  Mail* pCurrent = pMailList->pHead;

  while (pCurrent)
  {
    if (bFirst)
    {
      bFirst = false;
    }
    else
    {
      IER(StrBuilder_Append(pStrBuilder, ","));
    }

    IER(StrBuilder_Append(pStrBuilder, "["));
    IER(StrBuilder_AppendInt(pStrBuilder, pCurrent->FuncID));
    IER(StrBuilder_Append(pStrBuilder, ","));
    IER(StrBuilder_AppendInt(pStrBuilder, pCurrent->msgId));
    IER(StrBuilder_Append(pStrBuilder, ","));
    IER(StrBuilder_Append(pStrBuilder, pCurrent->params));
    
    //Free some mem
    String_Reset(&pCurrent->params);

    pCurrent->params = NULL;
    IER(StrBuilder_Append(pStrBuilder, "]"));
    pCurrent = pCurrent->pNext;
  }

  IER(StrBuilder_Append(pStrBuilder, "]"));
  return result;
}



#define LIST_REMOVE_IF(pMailList, condition)\
{\
  Mail * current = (pMailList)->pHead;\
  Mail * temp_node = NULL;\
  Mail * prev_node = NULL;\
  \
  while (current)\
  {\
    if (condition)\
    {\
      temp_node = current;\
      current = temp_node->pNext;\
      \
      Mail_Cancel(temp_node, RESULT_CANCELED);\
      Mail_Delete(temp_node);\
      \
      if (prev_node == NULL)\
      {\
        (pMailList)->pHead = current;\
      }\
      else\
      {\
        prev_node->pNext = current;\
      }\
    }\
    else\
    {\
      prev_node = current;\
      current = current->pNext;\
    }\
  }\
}

Result MailList_RemoveNotWaiting(MailList* pMailList)
{
  LIST_REMOVE_IF(pMailList, !current->bWaitingResponse);

  return RESULT_OK;
}

Result MailList_RemoveExpired(MailList* pMailList,
                              int maxAgeSec)
{
  time_t now = time(NULL);
  LIST_REMOVE_IF(pMailList, (now - current->timestamp) > maxAgeSec);
  
  return RESULT_OK;
}
