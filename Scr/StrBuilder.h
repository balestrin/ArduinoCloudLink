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

#include <string.h>
#include "Result.h"

typedef struct
{
    char*  c_str;
    size_t size;
    size_t capacity;

} StrBuilder;

#define STRBUILDER_INIT { 0, 0, 0 }
#define STRBUILDER_DEFAULT_SIZE 17

Result StrBuilder_Init(StrBuilder* p, size_t capacity);

Result StrBuilder_Reserve(StrBuilder* p, size_t nelements);

void StrBuilder_Attach(StrBuilder* wstr, 
                       char* psz,
                       size_t nBytes);

void StrBuilder_Destroy(StrBuilder* wstr);
void StrBuilder_Swap(StrBuilder* str1, StrBuilder* str2);


void StrBuilder_Clear(StrBuilder* wstr);

Result StrBuilder_SetN(StrBuilder* p,
                       const char* source,
                       size_t nelements);

Result StrBuilder_Set(StrBuilder* p,
                      const char* source);

Result StrBuilder_AppendN(StrBuilder* p,
                          const char* source,
                          size_t nelements);

Result StrBuilder_Append(StrBuilder* p,
                         const char* source);


Result StrBuilder_AppendFmt(StrBuilder* p,
                            const char* format, 
                            ...);

Result StrBuilder_AppendVFmt(StrBuilder* p,
                            const char* format,
                            va_list arg);


Result StrBuilder_AppendJsonStr(StrBuilder* p,
                                const char* source);


char* StrBuilder_Release(StrBuilder* p);

Result StrBuilder_AppendInt(StrBuilder * p, int i);

Result StrBuilder_AppendJsonString(StrBuilder * p, const char* psz);

Result StrBuilder_AppendWChar(StrBuilder * p, wchar_t wch);
Result StrBuilder_AppendW(StrBuilder * p, const wchar_t* psz);
Result StrBuilder_AppendChar(StrBuilder * p, char wch);

Result StrBuilder_AppendQuotedJsonStr(StrBuilder* p,
                                      const char* source);