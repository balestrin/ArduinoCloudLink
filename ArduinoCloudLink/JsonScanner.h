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
#include "CSStream.h"
#include "StringEx.h"

typedef enum
{
  TKEndMark,
  TKLEFT_PARENTHESIS,
  TKRIGHT_PARENTHESIS,
  TKCOMMA,
  TKCOLON,
  TKSEMICOLON,
  TKLEFT_SQUARE_BRACKET,
  TKRIGHT_SQUARE_BRACKET,
  TKLEFT_CURLY_BRACKET,
  TKRIGHT_CURLY_BRACKET,
  TKBlanks,
  TKTrue,
  TKFalse,
  TKNull,
  TKNumber,
  TKString,
} JSON_Tokens;

typedef struct
{
  SStream ss;
  JSON_Tokens token;
  size_t bufferSize;
  StringC lexeme;

} JsonScanner;

Result JsonScanner_Init(JsonScanner* p,
                        const char* jsonString,
                        size_t lexemeSize);

Result JsonScanner_Next(JsonScanner* p);
Result JsonScanner_Match(JsonScanner* p, JSON_Tokens token);
void JsonScanner_Destroy(JsonScanner* p);

Result JsonScanner_Check(JsonScanner* pJsonScanner,
                         JSON_Tokens token);
