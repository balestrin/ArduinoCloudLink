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
#include "JsonScanner.h"
#include "StrBuilder.h"
#include "StringEx.h"

static int JSON_GetNext(int state, wchar_t ch)
{
  switch (state)
  {
  case 0:
    if (ch == L'\0') {
      return 1;
    }
    else if (ch >= L'\t' && ch <= L'\n') {
      return 2;
    }
    else if (ch == L'\r') {
      return 2;
    }
    else if (ch == L' ') {
      return 2;
    }
    else if (ch == L'"') {
      return 3;
    }
    else if (ch == L'(') {
      return 4;
    }
    else if (ch == L')') {
      return 5;
    }
    else if (ch == L'+') {
      return 6;
    }
    else if (ch == L',') {
      return 7;
    }
    else if (ch == L'-') {
      return 6;
    }
    else if (ch >= L'0' && ch <= L'9') {
      return 8;
    }
    else if (ch == L':') {
      return 9;
    }
    else if (ch == L';') {
      return 10;
    }
    else if (ch == L'[') {
      return 11;
    }
    else if (ch == L']') {
      return 12;
    }
    else if (ch == L'f') {
      return 13;
    }
    else if (ch == L'n') {
      return 14;
    }
    else if (ch == L't') {
      return 15;
    }
    else if (ch == L'{') {
      return 16;
    }

    if (ch == L'}') {
      return 17;
    }

    break;

  case 1:
    /* end state for TKEndMark*/
    break;

  case 2:
    if (ch >= L'\t' && ch <= L'\n') {
      return 2;
    }
    else if (ch == L'\r') {
      return 2;
    }
    else if (ch == L' ') {
      return 2;
    }

    /* end state for TKBlanks*/
    break;

  case 3:
    if (ch >= L' ' && ch <= L'!') {
      return 3;
    }
    else if (ch == L'"') {
      return 19;
    }
    else if (ch >= L'#' && ch <= L'[') {
      return 3;
    }
    else if (ch == L'\\') {
      return 20;
    }
    else if (ch >= L']' /*&& ch <= L'\u00ff'*/) {
      return 3;
    }

    break;

  case 4:
    /* end state for TKLEFT_PARENTHESIS*/
    break;

  case 5:
    /* end state for TKRIGHT_PARENTHESIS*/
    break;

  case 6:
    if (ch >= L'0' && ch <= L'9') {
      return 8;
    }

    break;

  case 7:
    /* end state for TKCOMMA*/
    break;

  case 8:
    if (ch == L'.') {
      return 21;
    }
    else if (ch >= L'0' && ch <= L'9') {
      return 8;
    }
    else if (ch == L'E') {
      return 22;
    }
    else if (ch == L'e') {
      return 22;
    }

    /* end state for TKNumber*/
    break;

  case 9:
    /* end state for TKCOLON*/
    break;

  case 10:
    /* end state for TKSEMICOLON*/
    break;

  case 11:
    /* end state for TKLEFT_SQUARE_BRACKET*/
    break;

  case 12:
    /* end state for TKRIGHT_SQUARE_BRACKET*/
    break;

  case 13:
    if (ch == L'a') {
      return 23;
    }

    break;

  case 14:
    if (ch == L'u') {
      return 24;
    }

    break;

  case 15:
    if (ch == L'r') {
      return 25;
    }

    break;

  case 16:
    /* end state for TKLEFT_CURLY_BRACKET*/
    break;

  case 17:
    /* end state for TKRIGHT_CURLY_BRACKET*/
    break;

  case 18:
    return -1;

  case 19:
    /* end state for TKString*/
    break;

  case 20:
    if (ch == L'"') {
      return 3;
    }
    else if (ch == L'/') {
      return 3;
    }
    else if (ch == L'\\') {
      return 3;
    }
    else if (ch == L'b') {
      return 3;
    }
    else if (ch == L'f') {
      return 3;
    }
    else if (ch == L'n') {
      return 3;
    }
    else if (ch == L'r') {
      return 3;
    }
    else if (ch == L't') {
      return 3;
    }

    if (ch == L'u') {
      return 26;
    }

    break;

  case 21:
    if (ch >= L'0' && ch <= L'9') {
      return 27;
    }

    break;

  case 22:
    if (ch == L'+') {
      return 28;
    }
    else if (ch == L'-') {
      return 28;
    }
    else if (ch >= L'0' && ch <= L'9') {
      return 29;
    }

    break;

  case 23:
    if (ch == L'l') {
      return 30;
    }

    break;

  case 24:
    if (ch == L'l') {
      return 31;
    }

    break;

  case 25:
    if (ch == L'u') {
      return 32;
    }

    break;

  case 26:
    if (ch >= L'0' && ch <= L'9') {
      return 33;
    }
    else if (ch >= L'A' && ch <= L'F') {
      return 33;
    }
    else if (ch >= L'a' && ch <= L'f') {
      return 33;
    }

    break;

  case 27:
    if (ch >= L'0' && ch <= L'9') {
      return 27;
    }
    else if (ch == L'E') {
      return 22;
    }
    else if (ch == L'e') {
      return 22;
    }

    /* end state for TKNumber*/
    break;

  case 28:
    if (ch >= L'0' && ch <= L'9') {
      return 29;
    }

    break;

  case 29:
    if (ch >= L'0' && ch <= L'9') {
      return 29;
    }

    /* end state for TKNumber*/
    break;

  case 30:
    if (ch == L's') {
      return 34;
    }

    break;

  case 31:
    if (ch == L'l') {
      return 35;
    }

    break;

  case 32:
    if (ch == L'e') {
      return 36;
    }

    break;

  case 33:
    if (ch >= L'0' && ch <= L'9') {
      return 37;
    }
    else if (ch >= L'A' && ch <= L'F') {
      return 37;
    }
    else if (ch >= L'a' && ch <= L'f') {
      return 37;
    }

    break;

  case 34:
    if (ch == L'e') {
      return 38;
    }

    break;

  case 35:
    /* end state for TKNull*/
    break;

  case 36:
    /* end state for TKTrue*/
    break;

  case 37:
    if (ch >= L'0' && ch <= L'9') {
      return 39;
    }
    else if (ch >= L'A' && ch <= L'F') {
      return 39;
    }
    else if (ch >= L'a' && ch <= L'f') {
      return 39;
    }

    break;

  case 38:
    /* end state for TKFalse*/
    break;

  case 39:
    if (ch >= L'0' && ch <= L'9') {
      return 3;
    }
    else if (ch >= L'A' && ch <= L'F') {
      return 3;
    }
    else if (ch >= L'a' && ch <= L'f') {
      return 3;
    }

    break;

  default:
    ASSERT(false);
    break;
  } /*switch*/

  return -1;
}

static int JSON_IsInterleave(JSON_Tokens tk)
{
  return tk == TKBlanks;
}

static int JSON_GetTokenFromState(int state, JSON_Tokens* pToken)
{
  ASSERT(pToken != NULL);

  switch (state)
  {
  case 1:
    *pToken = TKEndMark;
    break;

  case 2:
    *pToken = TKBlanks;
    break;

  case 4:
    *pToken = TKLEFT_PARENTHESIS;
    break;

  case 5:
    *pToken = TKRIGHT_PARENTHESIS;
    break;

  case 7:
    *pToken = TKCOMMA;
    break;

  case 8:
    *pToken = TKNumber;
    break;

  case 9:
    *pToken = TKCOLON;
    break;

  case 10:
    *pToken = TKSEMICOLON;
    break;

  case 11:
    *pToken = TKLEFT_SQUARE_BRACKET;
    break;

  case 12:
    *pToken = TKRIGHT_SQUARE_BRACKET;
    break;

  case 16:
    *pToken = TKLEFT_CURLY_BRACKET;
    break;

  case 17:
    *pToken = TKRIGHT_CURLY_BRACKET;
    break;

  case 19:
    *pToken = TKString;
    break;

  case 27:
    *pToken = TKNumber;
    break;

  case 29:
    *pToken = TKNumber;
    break;

  case 35:
    *pToken = TKNull;
    break;

  case 36:
    *pToken = TKTrue;
    break;

  case 38:
    *pToken = TKFalse;
    break;

  default:
    return 0;
  }

  return 1;
}

Result JSON_NextTokenNoInterleave(SStream* pSStream,
                                  char** ppBufferInOut,
                                  size_t* pBufferInOutSize,
                                  JSON_Tokens* pToken)
{
  ASSERT(pSStream != NULL);
  ASSERT(pToken != NULL);
  ASSERT(ppBufferInOut != NULL); 
  ASSERT(pBufferInOutSize != NULL);
  

  StrBuilder stringBuilder = STRBUILDER_INIT;
  StrBuilder_Attach(&stringBuilder,
                    *ppBufferInOut,
                    *pBufferInOutSize);

  if (*pBufferInOutSize > 0)
  {
    (*ppBufferInOut)[0] = 0;
  }

  int lastGoodState = -2;
  int currentState = 0;
  wchar_t ch;
  Result result = SStream_GetChar(pSStream, &ch);

  while (result == RESULT_OK)
  {
    currentState = JSON_GetNext(currentState, ch);

    if (lastGoodState == -2 &&
        currentState == -1)
    {
      ASSERT(false);
      result = RESULT_FAIL; //lint !e527
      break;
    }

    if (currentState == -1)
    {
      SStream_Unget(pSStream, ch);
      break;
    }

    JSON_Tokens tk2;

    if (JSON_GetTokenFromState(currentState, &tk2))
    {
      *pToken = tk2;
      lastGoodState = currentState;
    }

    StrBuilder_AppendWChar(&stringBuilder, ch);
    result = SStream_GetChar(pSStream, &ch);
  }

  *pBufferInOutSize = stringBuilder.capacity + 1;
  *ppBufferInOut = StrBuilder_Release(&stringBuilder);
  StrBuilder_Destroy(&stringBuilder);
  return result;
}

Result JSON_NextToken(SStream* pSStream,
                      char** buffer,
                      size_t* bufferSize,
                      JSON_Tokens* pToken)
{
  ASSERT(pSStream != NULL);
  ASSERT(pToken != NULL);
  Result r = RESULT_FAIL;

  for (;;)
  {
    r = JSON_NextTokenNoInterleave(pSStream, buffer, bufferSize, pToken);

    if (r == RESULT_OK)
    {
      if (!JSON_IsInterleave(*pToken))
      {
        break;
      }
    }
    else
    {
      break;
    }
  }

  return r;
}

Result JsonScanner_Init(JsonScanner* pJsonScanner,
                        const char* pJsonString,
                        size_t lexemeSize)
{
  ASSERT(pJsonScanner != NULL);
  ASSERT(pJsonString != NULL);

  Result result;
  do
  {
    result = SStream_Init(&pJsonScanner->ss,
                          pJsonString);

    if (result == RESULT_OK)
    {
      result = String_InitWithSize(&pJsonScanner->lexeme,
                                   lexemeSize);
      if (result == RESULT_OK)
      {
        pJsonScanner->token = TKEndMark;
        pJsonScanner->bufferSize = lexemeSize;
        break;
      }
      SStream_Destroy(&pJsonScanner->ss);
    }
  } while (0);

  return result;
}

Result JsonScanner_Next(JsonScanner* pJsonScanner)
{
  ASSERT(pJsonScanner != NULL);
  return JSON_NextToken(&pJsonScanner->ss,
                        &pJsonScanner->lexeme,
                        &pJsonScanner->bufferSize,
                        &pJsonScanner->token);
}

void JsonScanner_Destroy(JsonScanner* pJsonScanner)
{
  ASSERT(pJsonScanner != NULL);
  String_Destroy(&pJsonScanner->lexeme);
}

Result JsonScanner_Match(JsonScanner* pJsonScanner,
                         JSON_Tokens token)
{
  ASSERT(pJsonScanner != NULL);
  Result result = RESULT_FAIL;

  if (pJsonScanner->token == token)
  {
    result = JsonScanner_Next(pJsonScanner);
  }
  else
  {
    ASSERT(0);
  }
  if (result == RESULT_EOF)
  {
    result = RESULT_OK;
  }
  return result;
}


Result JsonScanner_Check(JsonScanner* pJsonScanner,
                         JSON_Tokens token)
{
  ASSERT(pJsonScanner != NULL);
  return pJsonScanner->token == token ? RESULT_OK : RESULT_FAIL;
}




