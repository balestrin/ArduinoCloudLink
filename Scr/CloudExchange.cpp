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
#include <string.h>
#include <stdarg.h>

#include "Config.h"
#include "CloudExchange.h"

#include "Result.h"
#include "StrBuilder.h"
#include <stdlib.h>
#include  "StringEx.h"
#include "EventNotification.h"
#include "chrono.h"

#define TIME_OUT_SEC 20
#define EXPIRE_TIME_OUT_SEC 10


Result  CloudExchange_PostMailMove(CloudExchange* pCloudExchange,
                                   Mail** pMail);

Result CloudExchange_Init(CloudExchange* pCloudExchange,
                          const char* password)
{
  Result result = RESULT_OK;
  MailList_Init(&pCloudExchange->postedMailList);
  MailList_Init(&pCloudExchange->sendingMailList);
  MailList_Init(&pCloudExchange->sentMailList);
  pCloudExchange->lastExchange = 0;
  StringS_Init(pCloudExchange->sessionToken);
  StringS_Init(pCloudExchange->password);
  StringS_Change(pCloudExchange->password, password);
  return result;
}

static Result CloudExchange_CancelMail(CloudExchange* pCloudExchange,
                                       Result result)
{
  MailList_Reset(&pCloudExchange->postedMailList, result);
  MailList_Reset(&pCloudExchange->sendingMailList, result);
  MailList_Reset(&pCloudExchange->sentMailList, result);
  return RESULT_OK;
}

void CloudExchange_DestroyEx(CloudExchange* pCloudExchange,
                             Result result)
{
  CloudExchange_CancelMail(pCloudExchange, result);
  MailList_Destroy(&pCloudExchange->postedMailList);
  MailList_Destroy(&pCloudExchange->sendingMailList);
  MailList_Destroy(&pCloudExchange->sentMailList);
  pCloudExchange->lastExchange = 0;
  StringS_Destroy(&pCloudExchange->sessionToken);
  StringS_Destroy(&pCloudExchange->password);
}

void CloudExchange_Destroy(CloudExchange* pCloudExchange)
{
  CloudExchange_DestroyEx(pCloudExchange, RESULT_CANCELED);
}

Result DispatchCore(JsonScanner* scanner,
                    CloudExchange* pCloudExchange)
{
#ifdef SERVER_SIDE
  return DispatchCoreServer(scanner, pCloudExchange);
#else
  return DispatchCoreClient(scanner, pCloudExchange);
#endif
}

Result CloudExchange_DeliveryMails(CloudExchange* pCloudExchange,
                                   StrBuilder* strBuilderInput)
{
  JsonScanner jsonScanner;
  Result result = JsonScanner_Init(&jsonScanner,
                                   strBuilderInput->c_str, 10);

  if (result == RESULT_OK)
  {
    result = DispatchCore(&jsonScanner, pCloudExchange);
    JsonScanner_Destroy(&jsonScanner);
  }

  return result;
}

bool CloudExchange_HasSession(CloudExchange* pCloudExchange)
{
  return pCloudExchange->sessionToken[0] != '\0';
}

static Result OnLoginResult(Result result,
                            void* data,
                            const char* password)
{
  if (result == RESULT_OK)
  {
    CloudExchange* pCloudExchange = (CloudExchange*)data;
    StringS_Change(pCloudExchange->sessionToken, password);
  }
  else
  {
    if (result == RESULT_ACCESS_DENIED)
    {
      RaiseEvent(EVENT_ACCESS_DENIED, 0);
    }
  }

  return RESULT_OK;
}


Result CloudExchange_BeginExchange(CloudExchange* pCloudExchange,
                                   StrBuilder* strBuilder)
{
  StrBuilder_Clear(strBuilder); //out

  if (!CloudExchange_HasSession(pCloudExchange))
  {
    Login(pCloudExchange,
          pCloudExchange->password,
          &OnLoginResult,
          pCloudExchange);
  }
  
  Result result = MailList_GetPostMainString(&pCloudExchange->postedMailList,
                  pCloudExchange->sessionToken,
                  strBuilder);

  if (result == RESULT_OK)
  {    
    MailList_RemoveNotWaiting(&pCloudExchange->postedMailList);
   
    MailList_UpdateStatus(&pCloudExchange->postedMailList, " Sending");

    MailList_AppendListMoved(&pCloudExchange->sendingMailList,
                             &pCloudExchange->postedMailList);
  }
  else
  {
    StrBuilder_Clear(strBuilder);
    MailList_Reset(&pCloudExchange->postedMailList, result);
  }

  return result;
}


Result CloudExchange_EndExchange(CloudExchange* pCloudExchange,
                                 Result exchangeResult,
                                 StrBuilder* strBuilder)
{
  Result result = RESULT_OK;

  if (exchangeResult == RESULT_OK)
  {    
    //sending -> sent
    MailList_AppendListMoved(&pCloudExchange->sentMailList,
                             &pCloudExchange->sendingMailList);

    MailList_UpdateStatus(&pCloudExchange->sentMailList, " Sent");

    result = CloudExchange_DeliveryMails(pCloudExchange, strBuilder);
    ASSERT(result == RESULT_OK);
  }
  else
  {
    RaiseEvent(EVENT_MESSAGES_NOT_SENT, 0);    
    MailList_Reset(&pCloudExchange->sendingMailList,
                   exchangeResult);
  }
  
  MailList_RemoveExpired(&pCloudExchange->sentMailList,
                         EXPIRE_TIME_OUT_SEC);

  return RESULT_OK;
}

Result Login(CloudExchange* pCloudExchange,
             const char* token,
             OnResultFunc callback,
             void* data)
{
  const int version = 1;
  return PostMail(pCloudExchange,
                  FUNC_LOGIN,
                  callback,
                  data,
                  true,
                  0,
                  "%S,%d",
                  token,
                  version);  
}

Result CloudExchange_SetPassword(CloudExchange* pCloudExchange,
                                 const char* password)
{
  return StringS_Change(pCloudExchange->password, password);  
}

Mail* CloudExchange_PopSentMail(CloudExchange* pCloudExchange,
                                int id)
{
  return MailList_PopMail(&pCloudExchange->sentMailList, id);
}


Result CloudExchange_Reset(CloudExchange* pCloudExchange,
                           Result result)
{
  StringS password2[20];
  StringS_Change(password2, pCloudExchange->password);
  CloudExchange_DestroyEx(pCloudExchange, result);
  OnReset(pCloudExchange);
  RaiseEvent(EVENT_RESET, 0);
  return CloudExchange_Init(pCloudExchange, password2);
}


Result  CloudExchange_PostMailMove(CloudExchange* pCloudExchange,
                                   Mail** ppMail)
{
  Mail_UpdateStatus(*ppMail, " posted");
  return MailList_AppendMove(&pCloudExchange->postedMailList,
                             ppMail);
}



Result HandleMail(JsonScanner* scanner,
                  const char* format,
                  ...)
{
  va_list args;
  va_start(args, format);

  Result result = RESULT_OK;

  const char *types = format;

  int isFirst = 1;

  while (*types != '\0')
  {
    if (isFirst)
    {
      isFirst = 0;
    }
    else
    {
      result = JsonScanner_Match(scanner, TKCOMMA);
      if (result != RESULT_OK)
        break;
    }

    if (result != RESULT_OK)
    {
      break;
    }

    if (*types == '%')
    {
      ++types;
      switch (*types)
      {
        case 's':
        // result = JsonScanner_Check(scanner, TKString);
        if (result == RESULT_OK)
        {
          char * pchar = (char*)va_arg(args, char*);
          int sz = (int)va_arg(args, int);

          if (strncpy_s(pchar,
            sz,
            scanner->lexeme,
            strlen(scanner->lexeme)) != 0)
          {
            result = RESULT_FAIL;
          }

          result = JsonScanner_Next(scanner);
        }
        ++types;
        break;

        case 'd':
        result = JsonScanner_Check(scanner, TKNumber);
        if (result == RESULT_OK)
        {
          int * val = (int*)va_arg(args, int*);
          *val = atoi(scanner->lexeme);
          result = JsonScanner_Next(scanner);
        }
        ++types;
        break;

        case 'b':
        result = scanner->token == TKTrue || scanner->token == TKFalse ? RESULT_OK : RESULT_FAIL;
        if (result == RESULT_OK)
        {
          bool * val = (bool*)va_arg(args, bool*);
          *val = strcmp(scanner->lexeme, "true") == 0;
          result = JsonScanner_Next(scanner);
        }
        ++types;
        break;


        case 'S':

        result = JsonScanner_Check(scanner, TKString);
        if (result == RESULT_OK)
        {
          char * pchar = (char*)va_arg(args, char*);
          int sz = (int)va_arg(args, int);
          ExtractString(pchar, sz, scanner->lexeme);
          result = JsonScanner_Next(scanner);
        }

        ++types;
        break;

        default:
        result = RESULT_FAIL;
        ASSERT(0);
        break;
      }
    }
    else
    {
      ++types;
    }
  }

  va_end(args);
  return result;
}


Result PostMail(CloudExchange* pCloudExchange,
                Functions funcID,
                OnResultFunc callback,
                void* data,
                bool bWaitingResponse,
                int idOpt,
                const char* format,
                ...)
{
  va_list ap;
  va_start(ap, format);

  Mail* pMail = NULL;
  Result result = Mail_Create(&pMail, bWaitingResponse, idOpt);

  if (result == RESULT_OK)
  {
    StrBuilder strBuilder;
    result = StrBuilder_Init(&strBuilder, 20);

    if (result == RESULT_OK)
    {
      result = StrBuilder_AppendVFmt(&strBuilder,
                                     format,
                                     ap);

      if (result == RESULT_OK)
      {
        ASSERT(pMail->params == NULL);
        pMail->params = StrBuilder_Release(&strBuilder); //moved
        pMail->FuncID = funcID;
        pMail->data = data;
        pMail->callback = (void*)callback;
        CloudExchange_PostMailMove(pCloudExchange, &pMail); //Moved
      }

      StrBuilder_Destroy(&strBuilder);//moved
    }

    Mail_Delete(pMail);//moved
  }

  if (result != RESULT_OK)
  {
    callback(result, data, "");
  }

  va_end(ap);
  return result;
}





void ExtractString(char* dest,
                   int destSize,
                   const char* source)
{
  size_t sl = strlen(source);

  if (sl > 2)
  {
    //TODO Falta o decode
    //TODO UNSCAPE
    strncpy_s(dest,
              destSize,
              source + 1,
              sl - 2);
    dest[sl - 2] = '\0';
  }
  else if (sl == 2)
  {
    dest[0] = '\0';
  }
  else
  {
    ASSERT(0);
  }
}


#define MATCH(x, tk) if (JsonScanner_Match(x, tk) != RESULT_OK)  return RESULT_FAIL;
#define CHECK(x, tk) if ((x)->token != (tk))  return RESULT_FAIL;


Result HandleOnResult(JsonScanner* scanner,
                             CloudExchange* pCloudExchange,
                             int msgId)
{
  Result result = RESULT_OK;
  
  Mail* pMail = CloudExchange_PopSentMail(pCloudExchange, msgId);

  if (pMail)
  {
    if (pMail->callback)
    {
      OnResultFunc f = (OnResultFunc)pMail->callback;
      char message[100];
      int result;
      
      if (HandleMail(scanner,
                 "%d%S",
                 &result,
                 &message,
                 100) != RESULT_OK)
      {
        StringS_Change(message, "error!");
        result = RESULT_FAIL;
      }
      

      f((Result)result, pMail->data, message);

      pMail->data = NULL;//moved
      pMail->callback = NULL;//nao usado
    }

    Mail_Delete(pMail);
  }

   
  return result;
}


Result PostOnResult(Result result,
                    void* data,
                    const char* message)
{
  CloudExchange* pCloudExchange = (CloudExchange*)(data);
  int id = pCloudExchange->id;
  Functions func = pCloudExchange->func;

  return PostMail(pCloudExchange,
                  func,
                  NULL,
                  0,
                  false,
                  id,
                  "%d,%S",
                  (int)result,
                  message);    
}

