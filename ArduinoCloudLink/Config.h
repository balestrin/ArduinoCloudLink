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

#ifdef ARDUINO
#define CLIENT_SIDE

#endif

#ifndef __cplusplus
#include <stdbool.h>
#define inline __inline
#endif

#define SIZE_OF_ARRAY(x) (sizeof(x)/sizeof(x[0]))

#ifndef ARDUINO

//http://en.cppreference.com/w/c/string/byte/strncpy
#define __STDC_LIB_EXT1__

#include <crtdbg.h>
#define ASSERT(x) _ASSERTE(x)

#endif


#ifdef ARDUINO



//#define USE_UTF8
#include <SPI.h>
#include <avr/pgmspace.h>

//#define PROGMEM

#define LOG(x) //Serial.println(x)
#define LOGSTR(x) // Serial.println(F(x))
#define LOG_ALWAYS(x) //Serial.println(x)
#define LOG_ALWAYS_STR(x) Serial.println(F(x))

#define IER(x) if ((result = x) != RESULT_OK) return result;
#define AddPost(x, y);
#define ASSERT(x) //if (!(x)) Serial.println(F("ASSERT FAILED"))


#else

#ifndef SERVER_SIDE
#define CUSTOM_ALLOCATOR
#endif


#pragma warning(disable: 4127)

#define LOW 0.0f
#define HIGH 1.0f

#define F(x) x
#define IER(x) if ((result = x) != RESULT_OK) return result;

#define USE_UTF8
#define PROGMEM


#define LOG(x) AddPost("LOG", x)
#define LOGSTR(x) AddPost("LOG", x)
#define LOG_ALWAYS(x)  AddPost("LOG", x)
#define LOG_ALWAYS_STR(x)  AddPost("LOG", x)

#include "logs.h"
#endif

#include "dmalloc.h"


