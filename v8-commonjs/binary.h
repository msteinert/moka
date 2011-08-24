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

#ifndef V8_COMMONJS_BINARY_H
#define V8_COMMONJS_BINARY_H

#include "v8-commonjs/module.h"

namespace commonjs {

class Binary;

} // namespace commonjs

class commonjs::Binary {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  uint32_t GetLength() const {
    return length_;
  }

  char* GetData() {
    return data_;
  }

  v8::Handle<v8::Value> Resize(uint32_t size);

  v8::Handle<v8::Value> Join(v8::Handle<v8::Array> array, char number);

  char Get(uint32_t index) const {
    return data_[index];
  }

  void Set(uint32_t index, char value) {
    data_[index] = value;
  }

protected: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> LengthGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> ToArray(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> ToString(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> ToSource(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> DecodeToString(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> IndexOf(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> LastIndexOf(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> CodeAt(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> ByteAt(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Split(const v8::Arguments& arguments);

protected: // Protected methods
  Binary();

  virtual ~Binary();

  virtual v8::Handle<v8::Value> Construct(int length);

  virtual v8::Handle<v8::Value> Construct(v8::Handle<v8::Object> object);

  virtual v8::Handle<v8::Value> Construct(v8::Handle<v8::Array> numbers);

  virtual v8::Handle<v8::Value> Construct(v8::Handle<v8::String> string,
      v8::Handle<v8::String> charset = v8::Handle<v8::String>());

  virtual v8::Handle<v8::Value> Construct(v8::Handle<v8::Uint32> length);

private: // Private methods
  Binary(Binary const& that);

  void operator=(Binary const& that);

private: // Private data
  uint32_t size_;
  uint32_t length_;
  char* data_;
};

#endif // V8_COMMONJS_BINARY_H

// vim: tabstop=2:sw=2:expandtab
