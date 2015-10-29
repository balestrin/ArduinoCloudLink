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
#include "StrBuilder.h"
#include <string.h>
#include <stdlib.h>

#include <stdlib.h>

#include <string.h>
#include <stdarg.h> //va_start et al
#include <stdio.h>
#include <stddef.h>
#ifdef USE_UTF8
#include  "Utf8.h"
#endif
#include "StringEx.h"

static int IsEmptyStr(const char* psz)
{
  return psz == NULL || psz[0] == '\0';
}

Result StrBuilder_Init(StrBuilder* p, size_t capacity)
{
  Result result = RESULT_OK;
  p->c_str = NULL;
  p->size = 0;
  p->capacity = 0;

  if (capacity > 0)
  {
    result = StrBuilder_Reserve(p, capacity);
  }

  return result;
}

void StrBuilder_Swap(StrBuilder* str1,
                     StrBuilder* str2)
{
  StrBuilder temp;
  memcpy(&temp, str1, sizeof(StrBuilder));  
  memcpy(str1, str2, sizeof(StrBuilder));
  memcpy(str2, &temp, sizeof(StrBuilder));  
}

void StrBuilder_Destroy(StrBuilder* p)
{
  if (p)
  {
    Free(p->c_str);
    p->c_str = NULL;
    p->size = 0;
    p->capacity = 0;
  }
}

Result StrBuilder_Reserve(StrBuilder* p, size_t nelements)
{
  Result r = RESULT_OK;

  if (nelements > p->capacity)
  {
    char* pnew = (char*)Realloc(p->c_str,
                                (nelements + 1) * sizeof(p->c_str[0]));

    if (pnew)
    {
      if (p->c_str == NULL)
      {
        pnew[0] = '\0';
      }

      p->c_str = pnew;
      p->capacity = nelements;
    }
    else
    {
      r = RESULT_OUT_OF_MEM;      
    }
  }

  return r;
}

static Result Grow(StrBuilder* p, size_t nelements)
{
  Result r = RESULT_OK;

  if (nelements > p->capacity)
  {
#ifdef ARDUINO
    size_t new_nelements = nelements;
#else
    size_t new_nelements = p->capacity + p->capacity / 2;
#endif

    if (new_nelements < nelements)
    {
      new_nelements = nelements;
    }

    r = StrBuilder_Reserve(p, new_nelements);
  }

  return r;
}

Result StrBuilder_SetN(StrBuilder* p,
                       const char* source,
                       size_t nelements)
{
  Result r = Grow(p, nelements);

  if (r == 0)
  {
    strncpy_s(p->c_str, p->capacity + 1, source, nelements);
    p->c_str[nelements] = '\0';
    p->size = nelements;
  }

  return r;
}

Result StrBuilder_Set(StrBuilder* p,
                      const char* source)
{
  StrBuilder_Clear(p);
  return StrBuilder_SetN(p, source, strlen(source));
}

Result StrBuilder_AppendN(StrBuilder* p,
                          const char* source,
                          size_t nelements)
{
  if (IsEmptyStr(source))
  {
    return RESULT_OK;
  }

  Result r = Grow(p, p->size + nelements);

  if (r == RESULT_OK)
  {
    strncpy_s(p->c_str + p->size,
              (p->capacity + 1) - p->size,
              source,
              nelements);
    p->c_str[p->size + nelements] = '\0';
    p->size += nelements;
  }

  return r;
}

Result StrBuilder_Append(StrBuilder* p,
                         const char* source)
{
  if (IsEmptyStr(source))
  {
    return RESULT_OK;
  }

  return StrBuilder_AppendN(p, source, strlen(source));
}

Result StrBuilder_AppendVFmt(StrBuilder* p,
                             const char* format,
                             va_list args)
{
  if (format == NULL)
  {
    return RESULT_NULL_PTR;
  }

  Result result = RESULT_OK;


  const char *types = format;
  const char *begin = format;
  const char *end = format;

  while (*types != '\0')
  {
    if (result != RESULT_OK)
    {
      break;
    }

    if (*types == '%')
    {
      if (begin != end)
      {
        result = StrBuilder_AppendN(p, begin, (end - begin));
        if (result != RESULT_OK)
        {
          break;
        }

      }


      ++types;
      switch (*types)
      {
        case 's':
        result = StrBuilder_Append(p, (const char*)va_arg(args, const char *));
        ++types;
        break;
        case 'b':
        result = StrBuilder_Append(p, (bool)(va_arg(args, bool)) ? "true" : "false");
        ++types;
        break;
        case 'd':
        result = StrBuilder_AppendInt(p, (int)va_arg(args, int));
        ++types;
        break;
        case '%':
        ++types;
        result = StrBuilder_AppendChar(p, '%');
        break;
        case 'S':
        ++types;
        result = StrBuilder_AppendQuotedJsonStr(p, (const char*)va_arg(args, const char *));
        break;
        case '\0':
        break;
        default:
        result = RESULT_FAIL;
        ASSERT(0);
        break;
      }

      begin = types;
      end = types;
    }
    else
    {
      ++types;
      ++end;
    }
  }

  if (result == RESULT_OK &&
      (begin != end))
  {
    result = StrBuilder_AppendN(p, begin, (end - begin));
  }
  
  return result;
}

Result StrBuilder_AppendFmt(StrBuilder* p,
  const char* format,
  ...)
{

  va_list args;
  va_start(args, format);
  Result result = StrBuilder_AppendVFmt(p, format, args);
  va_end(args);
  return result;
}

void StrBuilder_Clear(StrBuilder* p)
{
  if (p->c_str != NULL)
  {
    p->c_str[0] = '\0';
    p->size = 0;
  }
}

char* StrBuilder_Release(StrBuilder* p)
{
  char* pResult = p->c_str;

  if (pResult != NULL)
  {
    p->c_str = NULL;
    p->size = 0;
    p->capacity = 0;
  }

  return pResult;
}


Result StrBuilder_AppendInt(StrBuilder* p, int i)
{
  Result r = RESULT_FAIL;
  char buffer[20];
#ifdef ARDUINO
  itoa(i, buffer, 10);
#else
  _itoa_s(i, buffer, sizeof(buffer), 10);
#endif
  r = StrBuilder_Append(p, buffer);
  return r;
}

static Result EscapeJSON(StrBuilder* sb, const char* source)
{
  Result result = RESULT_OK;
  const char* it = source;

  while (*it && result == RESULT_OK)
  {
    switch (*it)
    {
    case '"':
    result = StrBuilder_Append(sb, "\\\"");
      break;

    case L'/':
    result = StrBuilder_Append(sb, "\\/");
      break;

    case L'\b':
    result = StrBuilder_Append(sb, "\\b");
      break;

    case L'\f':
    result = StrBuilder_Append(sb, "\\f");
      break;

    case L'\n':
    result = StrBuilder_Append(sb, "\\n");
      break;

    case L'\r':
    result = StrBuilder_Append(sb, "\\r");
      break;

    case L'\t':
    result = StrBuilder_Append(sb, "\\t");
      break;

    case L'\\':
    result = StrBuilder_Append(sb, "\\\\");
      break;

    default:
    result = StrBuilder_AppendN(sb, it, 1);
      break;
    }

    it++;
  }
  return result;
}

void StrBuilder_Attach(StrBuilder* pStrBuilder,
                       char* psz,
                       size_t nBytes)
{
  if (psz != NULL)
  {
    ASSERT(nBytes > 0);
    StrBuilder_Destroy(pStrBuilder);
    pStrBuilder->c_str = psz;
    pStrBuilder->capacity = nBytes - 1;
    pStrBuilder->size = 0;
    pStrBuilder->c_str[0] = '\0';
  }
}

Result StrBuilder_AppendWChar(StrBuilder* p, wchar_t wch)
{
#ifdef USE_UTF8
  //Estamos com pouca memória
  //o decode usa uma tabela estática
  char buffer[5] = { 0 };
  int nbytes = EncodeCharToUTF8Bytes(wch, buffer);
  return StrBuilder_AppendN(p, buffer, nbytes);
#else
  char ch = (char)wch;
  return StrBuilder_AppendN(p, &ch, 1);
#endif
}


Result StrBuilder_AppendChar(StrBuilder* p, char ch)
{
  return StrBuilder_AppendN(p, &ch, 1);
}

Result StrBuilder_AppendW(StrBuilder* p, const wchar_t* psz)
{
  Result result = RESULT_FAIL;

  while (*psz)
  {
    result = StrBuilder_AppendWChar(p, *psz);

    if (result != RESULT_OK) {
      break;
    }

    psz++;
  }

  return result;
}

Result StrBuilder_AppendJsonStr(StrBuilder* p,
                                const char* source)

{
  return EscapeJSON(p, source);
}


Result StrBuilder_AppendQuotedJsonStr(StrBuilder* p,
                                      const char* source)

{
  Result result = StrBuilder_Append(p, "\"");
  if (result == RESULT_OK)
  {
    result = EscapeJSON(p, source);
    if (result == RESULT_OK)
    {
      result = StrBuilder_Append(p, "\"");
    }
  }
  return result;
}
