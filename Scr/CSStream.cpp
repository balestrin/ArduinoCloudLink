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
#include "CSStream.h"

#include "Result.h"
#include <inttypes.h>

#ifdef USE_UTF8
#include "Utf8.h"
#endif

#include "config.h"

Result SStream_Init(SStream* pThis, const char* psz)
{
  pThis->pCharacteres = psz;
  pThis->pCurrentByte = psz;

  pThis->line = 1;
  pThis->col = 1;
  pThis->bHasPutBack = 0;
  pThis->putBackCharacter = L'\0';
  pThis->bEofSent = 0;
  
  pThis->previousline = 0;
  pThis->previouscol = 0;

  return RESULT_OK;
}

void SStream_Destroy(SStream* /*pThis*/)
{
}

void SStream_Unget(SStream* pThis,
                   wchar_t ch)
{
  ASSERT(pThis->bHasPutBack == 0);
  pThis->line = pThis->previousline;
  pThis->col = pThis->previouscol;
  pThis->putBackCharacter = ch;
  pThis->bHasPutBack = 1;
}

static Result GetPutBackChar(SStream* pThis, wchar_t* ch)
{
  ASSERT(!pThis->bEofSent);
  ASSERT(pThis->bHasPutBack);

  *ch = pThis->putBackCharacter;
  pThis->bHasPutBack = 0;
  pThis->putBackCharacter = 0;
  pThis->previousline = pThis->line;
  pThis->previouscol = pThis->col;

  if (*ch == L'\n')
  {
    pThis->line++;
    pThis->col = 0;
  }
  else
  {
    pThis->col++;
  }

  return RESULT_OK;
}

static Result ReadNextChar(SStream* pThis, wchar_t* ch)
{
  ASSERT(!pThis->bHasPutBack);
  ASSERT(!pThis->bEofSent);

  Result result = RESULT_FAIL;
  uint32_t codepoint = 0;
  

  
#ifdef USE_UTF8
  uint32_t state = 0;
  while (pThis->pCurrentByte)
  {
    if (decode(&state, &codepoint, (uint8_t)*pThis->pCurrentByte))
    {
      pThis->pCurrentByte++;
      continue;
    }

    if (codepoint > 0xffff)
    {
      ASSERT(false);
      //*d++ = (uint16_t)(0xD7C0 + (codepoint >> 10));
      //*d++ = (uint16_t)(0xDC00 + (codepoint & 0x3FF));
      pThis->pCurrentByte++;
      break;
    }
    else
    {
      pThis->pCurrentByte++;
      break;
    }
  }
  result = (state == UTF8_ACCEPT) ? RESULT_OK : RESULT_FAIL;
#else
    codepoint = *pThis->pCurrentByte;
    pThis->pCurrentByte++;
    result = RESULT_OK;
#endif


    if (result == RESULT_OK)
    {
      *ch = (wchar_t)codepoint;

      if (*ch == L'\0')
      {
        *ch = '\0';
        pThis->bEofSent = 1;
      }

      pThis->previousline = pThis->line;
      pThis->previouscol = pThis->col;

      if (*ch == L'\n')
      {
        pThis->line++;
        pThis->col = 0;
      }
      else
      {
        pThis->col++;
      }
    }
    return result;
}


  Result SStream_GetChar(SStream* pThis,
                         wchar_t* ch)
  {
    if (pThis->bEofSent)
    {
      return RESULT_EOF;
    }

    Result result = RESULT_FAIL;

    if (pThis->bHasPutBack)
    {
      result = GetPutBackChar(pThis, ch);
    }
    else
    {
      result = ReadNextChar(pThis, ch);
    }

    return result;
  }

