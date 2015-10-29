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
#include "CloudExchangeC.h"
#include "CloudExchange.h"
#include "EventNotification.h"
#include <string.h>
#include <stdarg.h>

#ifdef ARDUINO
#define TAGNAME_MAX 10
#else
#define TAGNAME_MAX 100
#endif

#define MATCH(x, tk) if (JsonScanner_Match(x, tk) != RESULT_OK)  return RESULT_FAIL;
#define CHECK(x, tk) if ((x)->token != (tk))  return RESULT_FAIL;
#define CHECKOR(x, tk, tk2) if ((x)->token != (tk) && (x)->token != (tk2))  return RESULT_FAIL;





Result Client_GetTagList(CloudExchange* pCloudExchange,
                         const char* nodeId,
                         const char* filter,
                         bool flat,
                         OnTagListFunc callback,
                         void* data);

Result Client_Subscribe(CloudExchange* pCloudExchange,
                        const char* tagName,
                        OnResultFunc callback,
                        void* data);

Result Client_Unsubscribe(CloudExchange* pCloudExchange,
                          const char* tagName,
                          OnResultFunc callback,
                          void* data);


static Result HandleWrite(JsonScanner* scanner,
                          CloudExchange* pCloudExchange,
                          int msgId)
{
  char tagName[TAGNAME_MAX];
  char jsonValue[100];
  Result result = HandleMail(scanner,                           
                           "%S%s",
                           &tagName,
                           (int)TAGNAME_MAX,
                           &jsonValue,
                           (int)100);
  if (result == RESULT_OK)
  {
    pCloudExchange->id = msgId;
    pCloudExchange->func = FUNC_ON_WRITETAG_RESULT;
    WriteTag(pCloudExchange,
             tagName,
             jsonValue,
             &PostOnResult,
             pCloudExchange);
  }

  return result;
}
static Result HandleSubscribe(JsonScanner* scanner,
                              CloudExchange* pCloudExchange,
                              int msgId)
{
  char tagName[TAGNAME_MAX];
  
  Result result = HandleMail(scanner,
                             "%S",
                             &tagName,
                             (int)TAGNAME_MAX);
  if (result == RESULT_OK)
  {
    pCloudExchange->id = msgId;
    pCloudExchange->func = FUNC_ON_SUBSCRIBE_RESULT;
    Client_Subscribe(pCloudExchange,
                     tagName,
                     &PostOnResult,
                     pCloudExchange);
  }

  return result;
}

static Result HandleUnsubscribe(JsonScanner* scanner,
                                CloudExchange* pCloudExchange,
                                int msgId)
{
  char tagName[TAGNAME_MAX];
  
  Result result = HandleMail(scanner,
                             "%S",
                             &tagName,
                             (int)TAGNAME_MAX);
  if (result == RESULT_OK)
  {
    pCloudExchange->id = msgId;
    pCloudExchange->func = FUNC_ON_UNSUBSCRIBE_RESULT;
    Client_Unsubscribe(pCloudExchange,
                     tagName,
                     &PostOnResult,
                     pCloudExchange);
  }

  return result;
}


Result Client_OnTagList(Result /*result0*/,
                        void* data,
                        const char* tagList)
{
  CloudExchange* pCloudExchange = (CloudExchange*)(data);
  int id = pCloudExchange->id;
  Mail* pMail = NULL;
  Result result = Mail_Create(&pMail, false, id);

  if (result == RESULT_OK)
  {
    StrBuilder strBuilder;
    result = StrBuilder_Init(&strBuilder, strlen(tagList));

    if (result == RESULT_OK)
    {
      result = StrBuilder_AppendQuotedJsonStr(&strBuilder, tagList);

      if (result == RESULT_OK)
      {
        pMail->FuncID = FUNC_ONTAGLIST;
        pMail->params = StrBuilder_Release(&strBuilder);//moved
        CloudExchange_PostMailMove(pCloudExchange, &pMail);
      }

      StrBuilder_Destroy(&strBuilder);
    }
    else
    {
      //faltou memoria para colocar o conteudo da carta
      //TODO colocar um int com result
      //se falahar tenta enviar  so o int na carta para avisar
    }

    Mail_Delete(pMail);
  }

  return result;
}

static Result HandleGetTagList(JsonScanner* scanner,
                               CloudExchange* pCloudExchange,
                               int msgId)
{
  char nodeid[10];
  char filter[10];
  bool bFlat;
  Result result = HandleMail(scanner,
                             "%S%S%b",
                             &nodeid,
                             10,
                             &filter,
                             10,
                             &bFlat);

  if (result == RESULT_OK)
  {

    //pCloudExchange->func = FUNC_ON_UNSUBSCRIBE_RESULT;
    pCloudExchange->id = msgId;
    Client_GetTagList(pCloudExchange,
                      nodeid,
                      filter,
                      bFlat,
                      &Client_OnTagList,
                      pCloudExchange);
  }

  return result;
}


Result DispatchCoreClient(JsonScanner* scanner,
                          CloudExchange* pCloudExchange)
{
  pCloudExchange;
  Result result = JsonScanner_Next(scanner);

  if (result == RESULT_EOF)
  {
    //OK nada para ler
    return RESULT_OK;
  }

  MATCH(scanner, TKLEFT_SQUARE_BRACKET);

  if (scanner->token == TKRIGHT_SQUARE_BRACKET)
  {
    //ok array vazio
    return RESULT_OK;
  }

  bool bLoginRequired = false;

  for (;;)
  {
    MATCH(scanner, TKLEFT_SQUARE_BRACKET);
    CHECK(scanner, TKNumber);
    int fId = atoi(scanner->lexeme);
    MATCH(scanner, TKNumber);
    MATCH(scanner, TKCOMMA);
    CHECK(scanner, TKNumber);
    int msgId = atoi(scanner->lexeme);
    MATCH(scanner, TKNumber);

    if (bLoginRequired)
    {
      if (fId != FUNC_LOGIN)
      {
        return RESULT_ACCESS_DENIED;
      }
    }

    if (scanner->token == TKCOMMA)
    {
      //significa que tem mais parametros
      MATCH(scanner, TKCOMMA);
    }

    switch (fId)
    {
      case FUNC_ACCESS_DENIED:
      CloudExchange_Reset(pCloudExchange, RESULT_ACCESS_DENIED);
      break;

      case FUNC_GETTAGLIST:
      result = HandleGetTagList(scanner, pCloudExchange, msgId);
      break;

      case FUNC_SUBSCRIBE:
      result = HandleSubscribe(scanner, pCloudExchange, msgId);
      break;

      case FUNC_UNSUBSCRIBE:
      result = HandleUnsubscribe(scanner, pCloudExchange, msgId);
      break;

      case FUNC_WRITETAG:
      result = HandleWrite(scanner, pCloudExchange, msgId);
      break;

      case FUNC_ON_LOGIN_RESULT:
      case FUNC_ONTAGCHANGEDRESULT:
      result = HandleOnResult(scanner, pCloudExchange, msgId);
      break;

      case FUNC_RESET:
      CloudExchange_Reset(pCloudExchange, RESULT_CANCELED);
      break;

      default:
      ASSERT(0);
      break;
    }

    MATCH(scanner, TKRIGHT_SQUARE_BRACKET);

    if (scanner->token == TKCOMMA)
    {
      MATCH(scanner, TKCOMMA);
    }
    else
    {
      break;
    }
  }

  MATCH(scanner, TKRIGHT_SQUARE_BRACKET);
  return result;
}



Result OnTagChanged(CloudExchange* pCloudExchange,
                    const char* tagName,
                    const char* jsonValue,
                    int quality,
                    int timestamp,
                    OnResultFunc callback,
                    void* data)
{
  return PostMail(pCloudExchange,
                  FUNC_ONTAGCHANGED,
                  callback,
                  data,
                  true,
                  0,
                  "%S,%s,%d,%d",
                  tagName,
                  jsonValue,
                  quality,
                  timestamp);
}
