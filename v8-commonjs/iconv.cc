// Copyright 2011 Michael Steinert. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * The names of the copyright holder, the author, nor any contributors
//       may be used to endorse or promote products derived from this
//       software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iconv.h>
#include <string>
#include "v8-commonjs/iconv.h"

namespace commonjs {

Iconv::Iconv()
  : length_(0)
  , data_(NULL) {}

Iconv::~Iconv() {
  if (data_) {
    ::free(data_);
  }
}

v8::Handle<v8::Value> Iconv::Convert(const char* data, uint32_t length,
    const char* tocode, const char* fromcode) {
  v8::HandleScope handle_scope;
  iconv_t cd = ::iconv_open(tocode, fromcode);
  if ((iconv_t *)-1 == cd) {
    if (EINVAL == errno) {
      std::string message("Conversion from ");
      message.append(fromcode);
      message.append(" to ");
      message.append(tocode);
      message.append(" is not available");
      return handle_scope.Close(v8::Exception::TypeError(
            v8::String::New(message.c_str())));
    }
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  size_t size = length * sizeof(wchar_t);
  size_t outbytesleft = size;
  data_ = static_cast<char*>(::malloc(size));
  if (!data_) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  char* outbuf = data_;
  size_t inbytesleft = length;
  char* in_buffer = static_cast<char*>(::malloc(inbytesleft + 1));
  if (!in_buffer) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  memcpy(in_buffer, data, length);
  char* inbuf = in_buffer;
  int converted = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  while (-1 == converted) {
    if (EINVAL == errno) {
      // Junk at the end of the buffer, ignore it
      break;
    } else if (E2BIG == errno) {
      // Double the size of the output buffer
      size *= 2;
      char* new_buffer = static_cast<char*>(::realloc(data_, size));
      if (!new_buffer) {
        ::free(in_buffer);
        char message[BUFSIZ];
        ::strerror_r(errno, message, BUFSIZ);
        return handle_scope.Close(v8::Exception::Error(
              v8::String::New(message)));
      }
      outbytesleft = size;
      data_ = outbuf = new_buffer;
    } else {
      // Unrecoverable error
      ::free(in_buffer);
      char message[BUFSIZ];
      ::strerror_r(errno, message, BUFSIZ);
      return handle_scope.Close(v8::Exception::Error(
            v8::String::New(message)));
    }
    inbytesleft = length;
    inbuf = in_buffer;
    converted = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  }
  if (-1 == ::iconv_close(cd)) {
    ::free(in_buffer);
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  length_ = outbuf - data_;
  ::free(in_buffer);
  return handle_scope.Close(v8::Handle<v8::Value>());
}

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
