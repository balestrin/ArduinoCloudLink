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
#include "config.h"

#include "StringEx.h"
#include <stdlib.h>
#include <string.h>
#include <stdlib.h> //malloc


#ifndef ARDUINO

/*
* This array is designed for mapping upper and lower case letter
* together for a case independent comparison.  The mappings are
* based upon ascii character sequences.
*/
static const char charmap[] = {
  '\000', '\001', '\002', '\003', '\004', '\005', '\006', '\007',
  '\010', '\011', '\012', '\013', '\014', '\015', '\016', '\017',
  '\020', '\021', '\022', '\023', '\024', '\025', '\026', '\027',
  '\030', '\031', '\032', '\033', '\034', '\035', '\036', '\037',
  '\040', '\041', '\042', '\043', '\044', '\045', '\046', '\047',
  '\050', '\051', '\052', '\053', '\054', '\055', '\056', '\057',
  '\060', '\061', '\062', '\063', '\064', '\065', '\066', '\067',
  '\070', '\071', '\072', '\073', '\074', '\075', '\076', '\077',
  '\100', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
  '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
  '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
  '\170', '\171', '\172', '\133', '\134', '\135', '\136', '\137',
  '\140', '\141', '\142', '\143', '\144', '\145', '\146', '\147',
  '\150', '\151', '\152', '\153', '\154', '\155', '\156', '\157',
  '\160', '\161', '\162', '\163', '\164', '\165', '\166', '\167',
  '\170', '\171', '\172', '\173', '\174', '\175', '\176', '\177',
  '\200', '\201', '\202', '\203', '\204', '\205', '\206', '\207',
  '\210', '\211', '\212', '\213', '\214', '\215', '\216', '\217',
  '\220', '\221', '\222', '\223', '\224', '\225', '\226', '\227',
  '\230', '\231', '\232', '\233', '\234', '\235', '\236', '\237',
  '\240', '\241', '\242', '\243', '\244', '\245', '\246', '\247',
  '\250', '\251', '\252', '\253', '\254', '\255', '\256', '\257',
  '\260', '\261', '\262', '\263', '\264', '\265', '\266', '\267',
  '\270', '\271', '\272', '\273', '\274', '\275', '\276', '\277',
  '\300', '\301', '\302', '\303', '\304', '\305', '\306', '\307',
  '\310', '\311', '\312', '\313', '\314', '\315', '\316', '\317',
  '\320', '\321', '\322', '\323', '\324', '\325', '\326', '\327',
  '\330', '\331', '\332', '\333', '\334', '\335', '\336', '\337',
  '\340', '\341', '\342', '\343', '\344', '\345', '\346', '\347',
  '\350', '\351', '\352', '\353', '\354', '\355', '\356', '\357',
  '\360', '\361', '\362', '\363', '\364', '\365', '\366', '\367',
  '\370', '\371', '\372', '\373', '\374', '\375', '\376', '\377',
};

int strcasecmp(const char* s1, const char* s2)
{
  register const char* cm = charmap,
                       *us1 = (const char*)s1,
                        *us2 = (const char*)s2;

  while (cm[*us1] == cm[*us2++])
    if (*us1++ == '\0')
    {
      return (0);
    }

  return (cm[*us1] - cm[*--us2]);
}

int strncasecmp(const char* s1, const char* s2, size_t n)
{
  if (n != 0)
  {
    register const char* cm = charmap,
                         *us1 = (const char*)s1,
                          *us2 = (const char*)s2;

    do
    {
      if (cm[*us1] != cm[*us2++])
      {
        return (cm[*us1] - cm[*--us2]);
      }

      if (*us1++ == '\0')
      {
        break;
      }
    } while (--n != 0);
  }

  return (0);
}
#else
errno_t strncpy_s(char* dest,
                  size_t /*sizeInBytes*/,
                  const char* src,
                  size_t maxCount)
{
  return strncpy(dest, src, maxCount) != NULL ? 1 : 0;
}
#endif


void String_Reset(StringC* pString)
{
  String_Destroy(pString);
  String_Init(pString, NULL);
}

void String_Destroy(StringC* pString)
{
  Free(*pString);
  *pString = NULL;
}

void String_Attach(StringC* pString, char* psz)
{
  Free(*pString);
  *pString = psz;
}

void String_Swap(StringC* pA, StringC* pB)
{
  StringC temp = *pA;
  *pA = *pB;
  *pB = temp;
}

Result String_Change(StringC *pS1,
                     const char* psz1)
{
  Result result;

  StringC s1;
  result = String_Init(&s1, psz1);
  if (result == RESULT_OK)
  {    
      String_Swap(&s1, pS1);
      String_Destroy(&s1);
  }
  return result;
}

Result String_ChangeTwo(StringC *pS1,
                        const char* psz1,
                        StringC *pS2,                        
                        const char* psz2)
{
  Result result;

  StringC s1;
  result = String_Init(&s1, psz1);
  if (result == RESULT_OK)
  {
    StringC s2;
    result = String_Init(&s2, psz2);
    if (result == RESULT_OK)
    {
      String_Swap(&s1, pS1);
      String_Swap(&s2, pS2);
      String_Destroy(&s2);
    }
    String_Destroy(&s1);
  }

  return result;
}

Result String_InitWithSize(StringC* pString, size_t sz)
{
  Result result = RESULT_OK;
  if (sz > 0)
  {
    char*snew = (char*)Malloc(sizeof(char) * sz);
    if (snew != NULL)
    {
      snew[0] = '\0';
      *pString = (char*)snew;
    }
    else
    {
      result = RESULT_OUT_OF_MEM;
    }    
  }
  else
  {
    *pString = NULL;    
  }
  return result;
}

Result String_Init(StringC* pString, const char* sourceOpt)
{
  if (sourceOpt != NULL)
  {
    size_t len = strlen(sourceOpt) + 1;
    void *snew = Malloc(sizeof(char) * len);

    if (snew == NULL)
    {
      return RESULT_OUT_OF_MEM;
    }

    memcpy(snew, sourceOpt, len);

    *pString = (char*)snew;
  }
  else
  {
    *pString = NULL;
  }

  return RESULT_OK;
}


bool IsStrEmpty(const char* psz)
{
  return psz == NULL || psz[0] == 0;
}



Result IntToStr(int v, char* dest, size_t destlen)
{
#ifndef ARDUINO

  if (_itoa_s(v, dest, destlen, 10) != 0)
  {
    return RESULT_FAIL;
  }

#else
  itoa(v, dest, 10);
#endif
  return RESULT_OK;
}

Result StrNew(char** dest, size_t size)
{
  ASSERT(size > 0);
  ASSERT(*dest == NULL);
  char* temp = (char*)Malloc(sizeof(char) * size);

  if (temp == NULL)
  {
    return RESULT_OUT_OF_MEM;
  }

  *dest = temp;
  return RESULT_OK;
}



Result StringCopy(char* dest,
                  size_t destsize,
                  const char* source)
{
  if (source == NULL)
  {
    *dest = '\0';
    return RESULT_OK;
  }

  for (size_t i = 0; i < destsize - 1; i++)
  {
    if (*source == '\0')
      break;

    *dest = *source;
    source++;
    dest++;
  }

  *dest = '\0';

  if (*source != 0)
  {
    //nao coube
    return RESULT_FAIL;
  }

  return RESULT_OK;
}
