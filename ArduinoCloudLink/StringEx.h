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

#ifndef ARDUINO
int strncasecmp(const char* s1, const char* s2, size_t n);
int strcasecmp(const char* s1, const char* s2);
#else
typedef int errno_t ;
errno_t strncpy_s(char * dest,
                  size_t /*sizeInBytes*/,
                  const char * src,
                  size_t maxCount);
#endif

typedef char * StringC;
#define STRINGC_INIT NULL;

Result String_InitWithSize(StringC* pString, size_t sz);
Result String_Init(StringC* pString, const char* source);
void String_Destroy(StringC* pString);
void String_Reset(StringC* pString);
void String_Attach(StringC* pString, const char* psz);
void String_Swap(StringC* pA, StringC* pB);

Result String_Change(StringC *pS1,
                     const char* psz1);

Result String_ChangeTwo(StringC *pS1,                        
                        const char* psz1,
                        StringC *pS2,
                        const char* psz2);

Result IntToStr(int v, char* dest, size_t destlen);
bool IsStrEmpty(const char* psz);


typedef char StringS;


#define StringS_Init(s) ((s)[0] = '\0')
#define StringS_Destroy(s) 
#define StringS_Change(dest, source) StringCopy(dest, SIZE_OF_ARRAY((dest)), source)

#define StringS_Size(s) SIZE_OF_ARRAY((s))

Result StringCopy(char* dest,
                  size_t destsize,
                  const char* source);

