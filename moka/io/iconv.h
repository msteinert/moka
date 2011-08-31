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

#ifndef MOKA_IO_ICONV_H
#define MOKA_IO_ICONV_H

#include <iconv.h>
#include "moka/module.h"
#include <string>

namespace moka {

namespace io {

class Buffer;
class Iconv;

} // namespace io

} // namespace moka

class moka::io::Iconv {
public:
  static v8::Handle<v8::Value> New(const char* to, const char* from);

  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  v8::Handle<v8::Value> Convert(Buffer* in);

private: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

  static v8::Handle<v8::Value> ToGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> FromGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> ToString(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Convert(const v8::Arguments& arguments);

private: // Private methods
  v8::Handle<v8::Value> Construct(const char* to, const char* from);

  Iconv();

  ~Iconv();

  Iconv(Iconv const& that);

  void operator=(Iconv const& that);

  v8::Handle<v8::Value> EnsureBuffer(size_t length);

  v8::Handle<v8::Value> PruneBuffer();

private: // Private data
  iconv_t cd_;
  size_t length_;
  char* buffer_;
  std::string to_;
  std::string from_;
};

#endif // MOKA_IO_ICONV_H

// vim: tabstop=2:sw=2:expandtab
