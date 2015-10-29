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


#include "TCPClient.h"
#include "StringEx.h"

typedef struct 
{
  size_t     current_pos;
  bool       has_put_back;
  char       put_back_character;
  size_t     max_pos;
  TCPClient* tcp_connection;
  size_t     read_so_far;

  StringS    buffer[8];
  char*      pcurrent;
  size_t     buffer_size;
  size_t     data_size;
} SocketStream;

Result SocketStream_Init(SocketStream* p, TCPClient* con);
void SocketStream_Destroy(SocketStream* p);

void SockStream_PutBack(SocketStream* p, char ch);
Result SockStream_Get(SocketStream* p, char* ch);
Result SocketStream_ReadNBytes(SocketStream* p, int content_length);
