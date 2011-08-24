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

#ifndef V8_COMMONJS_BINARY_BYTE_STRING_H
#define V8_COMMONJS_BINARY_BYTE_STRING_H

#include "v8-commonjs/binary.h"

namespace commonjs {

class ByteString;

} // namespace commonjs

class commonjs::ByteString: public commonjs::Binary {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

protected: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

  static v8::Handle<v8::Value> GetIndex(uint32_t index,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> SetIndex(uint32_t index,
      v8::Local<v8::Value> value, const v8::AccessorInfo &info);

  static v8::Handle<v8::Integer> QueryIndex(uint32_t index,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> Join(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Get(const v8::Arguments& arguments);

private: // Private methods
  ByteString() {}

  ~ByteString() {}

  ByteString(ByteString const& that);

  void operator=(ByteString const& that);
};

#endif // V8_COMMONJS_BINARY_BYTE_STRING_H

// vim: tabstop=2:sw=2:expandtab
