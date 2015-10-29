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

#include "config.h"
#include "StringEx.h"
#include "StrBuilder.h"
#include "CloudExchange.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "TagsControl.h"
#include "Result.h"
#include "StringEx.h"
#include "EventNotification.h"

struct Subscription
{
  bool isFirst;
  bool isUsed;
  float value;
};

CustomReadFunc s_CustomRead = NULL;
CustomWriteFunc s_CustomWrite = NULL;
static int s_countSubscription = 0;
static const char* s_tags = NULL;
Subscription* s_subscriptions = NULL;


#ifndef ARDUINO

int s_digital[9] = { 0 };
float s_analog[9] = { 0 };

void digitalWrite(int i, int value)
{
  if (i == 9)
  {
    AddPost("LED9", value ? "true" : "false");
  }

  s_digital[i - 1] = value;
}

int digitalRead(int i)
{
  return s_digital[i - 1];
}

void analogWrite(int i, float value)
{
  s_analog[i - 1] = value;
}

float analogRead(int i)
{
  static int counter = 0;
  s_analog[i - 1] = (float) counter++;
  return s_analog[i - 1];
}
#endif


int Count(const char* psz)
{
  int count = 1;

  while (*psz++)
  {
    if (*psz == ';') {
      count++;
    }
  }

  return count;
}

void ClearSubscription() //clear
{
  for (int i = 0; i < s_countSubscription; i++)
  {
    s_subscriptions[i].isFirst = true;
    s_subscriptions[i].isUsed = false;
    s_subscriptions[i].value = LOW;
  }
}

void Setup(const char* s, CustomReadFunc fr, CustomWriteFunc fw)
{
  if (s == NULL)
  {
    s_tags = ARDUINO_UNO;  //uno
  }
  else
  {
    s_tags = s;
  }

  s_CustomRead = fr;
  s_CustomWrite = fw;
  s_countSubscription = Count(s_tags);
  s_subscriptions = (Subscription*)malloc(sizeof(Subscription) * s_countSubscription);
  ClearSubscription();
}

inline int IsSep(char ch)
{
  return ch == ';' || ch == '\0';
}


int FindTag(const char* psz, const char* tagName)
{
  int begin = 0;
  int c = 0;

  for (int i = 0; i <= (int)strlen(psz); i++)
  {
    if (IsSep(psz[i]))
    {
      if (strncmp(&psz[begin], tagName, strlen(tagName)) == 0)
      {
        return c;
      }

      begin = i + 1;
      c++;
    }
  }

  return -1;
}


Result SubscribeTag(const char* tagName, bool isUsed)
{
  Result result = RESULT_FAIL;
  int index = FindTag(s_tags, tagName);

  if (index != -1)
  {
    s_subscriptions[index].isUsed = isUsed;
    result = RESULT_OK;
  }
  else
  {
    result = RESULT_NOT_FOUND;
  }

  return result;
}


Result Read(const char* tagName, float* value)
{
  if (s_CustomRead)
  {
    return s_CustomRead(tagName, value);
  }

  return DefaultRead(tagName, value);
}

Result DefaultReadCore(const char* tagName,
                       float* value,
                       bool bRead)
{
  Result result = RESULT_FAIL;
  int sz = strlen(tagName);

  if (sz < 2)
  {
    return RESULT_FAIL;
  }

  if (tagName[0] == 'D')
  {
    int i = atoi(tagName + 1);

    if (bRead)
    {
      *value =  digitalRead(i) != LOW ? HIGH : LOW;
    }
    else
    {
      digitalWrite(i, *value != 0.0f ? 1 : 0);
    }

    result = RESULT_OK;
  }
  else if (tagName[0] == 'A')
  {
    int i = atoi(tagName + 1);

    if (bRead)
    {
      *value = analogRead(i);
    }
    else
    {
      analogWrite(i, *value);
    }

    result = RESULT_OK;
  }
  else
  {
    return RESULT_FAIL;
  }

  return result;
}

Result DefaultRead(const char* tagName, float* value)
{
  return DefaultReadCore(tagName, value, true);
}

Result DefaultWrite(const char* tagName, float value)
{
  LOG_ALWAYS_STR("WriteTag");
  LOG_ALWAYS(tagName);
  return DefaultReadCore(tagName, &value, false);
}

Result Write(const char* tagName, float value)
{
  if (s_CustomWrite)
  {
    return s_CustomWrite(tagName, value);
  }

  return DefaultWrite(tagName, value);
}

char* ftoa(char* a, double f, int precision)
{
  long p[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };
  char* ret = a;
  long heiltal = (long)f;
  IntToStr(heiltal, a, 10);

  while (*a != '\0') 
  {
    a++;
  }

  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  IntToStr(desimal, a, 10);
  return ret;
}


static Result OnTagChangedResult(Result result,
                                 void* data,
                                 const char* /*message*/)
{
  int word_index = (int)data;

  if (result == RESULT_OK)
  {
    LOG_ALWAYS_STR("OnTagChanged response ok");
    s_subscriptions[word_index].isFirst = false;
  }
  else
  {
    LOG_ALWAYS_STR("OnTagChanged response fail");
    s_subscriptions[word_index].isFirst = true;
  }

  return RESULT_OK;
}

Result SendChange(float value,
                  int word_index,
                  CloudExchange* pCloudExchange,
                  const char* tagName)
{
  char buffer[20];
  ftoa(buffer, value, 2);
  s_subscriptions[word_index].value = value;
  OnTagChanged(pCloudExchange,
               tagName,
               buffer,
               1,
               1,
               &OnTagChangedResult,
               (void*)word_index);
  return RESULT_OK;
}

bool IsEqualDigital(float a, float b)
{
  return (a != 0.0f) == (b != 0.0f);
}

bool IsEqualAnalog(float a, float b)
{
  //TODO depois colocar uma margem
  return a == b;
}


Result Update(CloudExchange* pCloudExchange)
{
  if (s_tags == NULL || s_subscriptions == NULL)
  {
    return RESULT_OK;
  }

  int begin = 0;
  int word_index = 0;
  Result result = RESULT_FAIL;
  const char* psz = s_tags;

  for (int i = 0; i <= (int) strlen(s_tags); i++)
  {
    if (IsSep(psz[i]))
    {
      if (s_subscriptions[word_index].isUsed)
      {
        char tagName[20] = { 0 };
        float value;
        strncpy_s(tagName, 20, &psz[begin], i - begin);
        result = Read(tagName, &value);

        if (result == RESULT_OK)
        {
          if (tagName[0] == 'D')
          {
            if (s_subscriptions[word_index].isFirst ||
                !IsEqualDigital(s_subscriptions[word_index].value, value))
            {
              result = SendChange(value != 0.0f ? 1.0f : 0.0f,
                                  word_index, 
                                  pCloudExchange,
                                  tagName);
            }
          }
          else
          {
            if (s_subscriptions[word_index].isFirst ||
                !IsEqualAnalog(value, s_subscriptions[word_index].value))
            {
              result = SendChange(value, word_index, pCloudExchange, tagName);
            }
          }
        }
        else
        {
          LOG_ALWAYS_STR("read error.");
          OnTagChanged(pCloudExchange,
                       tagName,
                       "0",
                       1,
                       1,
                       &OnTagChangedResult,
                       0);
        }
      }

      begin = i + 1;
      word_index++;
    }
  }

  return result;
}


Result WriteTag(CloudExchange* /*pCloudExchange*/,
                const char* tagName,
                const char* jsonValue,
                OnResultFunc callback,
                void* data)
{
  LOG_ALWAYS_STR("WriteTag");
  LOG_ALWAYS(tagName);
  LOG_ALWAYS(jsonValue);
  float value = (float)atof(jsonValue);

  Result result = Write(tagName, value);
  callback(result, data, "");
  return RESULT_OK;
}

Result Client_Unsubscribe(CloudExchange* /*pCloudExchange*/,
                          const char* tagName,
                          OnResultFunc callback,
                          void* data)
{
  LOG_ALWAYS_STR("Unsubscribe");
  //TODO pegar o retorno
  SubscribeTag(tagName, false);
  return callback(RESULT_OK, data, "");
}


Result Client_Subscribe(CloudExchange* /*pCloudExchange*/,
                        const char* tagName,
                        OnResultFunc callback,
                        void* data)
{
  LOG_ALWAYS_STR("Subscribe");
  
  Result r = SubscribeTag(tagName, true);

  if (r == RESULT_NOT_FOUND)
  {
    callback(RESULT_NOT_FOUND, data, "not found");
  }
  else
  {
    callback(RESULT_OK, data, "");
  }

  return RESULT_OK;
}

Result Client_GetTagList(CloudExchange* /*pCloudExchange*/,
                         const char* /*nodeId*/,
                         const char* /*filter*/,
                         bool /*flat*/,
                         OnTagListFunc callback,
                         void* data)
{
  LOG_ALWAYS_STR("GetTagList");
  callback(RESULT_OK, data, s_tags);
  return RESULT_OK;
}
/////////////////////////



Result OnReset(CloudExchange* /*pCloudExchange*/)
{
  LOG_ALWAYS_STR("OnReset");
  ClearSubscription();
  return RESULT_OK;
}


#define LED_ON(x) digitalWrite((x),(int)HIGH)
#define LED_OFF(x) digitalWrite((x),(int)LOW)

void LED_GREEN_ON()
{
  LED_ON(9);
}
void LED_GREEN_OFF()
{
  LED_OFF(9);
}
void LED_RED_ON()
{
  LED_ON(4);
}
void LED_RED_OFF()
{
  LED_OFF(4);
}


void DefaultLedEvents(Event e, size_t info)
{
  switch (e)
  {
    case EVENT_ACCESS_DENIED:
    //Os dois leds ficam ligados por um segundo e depois apagam      
    LED_GREEN_ON();
    LED_RED_ON();
    delay(1000);
    LED_GREEN_OFF();
    LED_RED_OFF();
    break;

    case EVENT_HEAP_FAIL:
    //O verde apaga e o vermelho da duas piscadas de 1 segundo
    LED_GREEN_OFF();
    LED_RED_ON();
    delay(2000);
    LED_RED_OFF();
    delay(2000);
    LED_RED_ON();
    delay(2000);
    LED_RED_OFF();

    break;

    case EVENT_RESET: //server reset
    //apaga tudo por 1 segundo
    LED_GREEN_OFF();
    LED_RED_OFF();
    delay(1000);
    break;

    case EVENT_BEGIN_EXCHANGE:
    //antes da troca
    LED_GREEN_ON();
    LED_RED_OFF();
    break;

    case EVENT_END_EXCHANGE: //depois
    {
      LED_GREEN_OFF();
      LED_RED_OFF();

      Result result = (Result)info;
      if (result != RESULT_OK)
      {
        LED_RED_ON();
        delay(1000);
      }
      else
      {
        delay(30);
        LED_GREEN_ON();
        delay(30);
        LED_GREEN_OFF();
      }

      //
    }
    break;

    default:
    ASSERT(0);
  }
}







