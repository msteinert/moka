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

#ifndef V8_COMMONJS_IO_H
#define V8_COMMONJS_IO_H

#include <cstdio>
#include <string>
#include "v8-commonjs/module.h"

namespace commonjs {

class Stream;

} // namespace commonjs

class commonjs::Stream {
public:
  Stream();

  Stream(FILE* file);

  ~File();

  const char* GetMessage() const {
    return message_.c_str();
  }

  bool Open(const char* filename, const char* mode);

  void Close();

  void Flush();

  void SetError();

  void SetError(const char* message);

  bool CheckError();

  void Print(char character);

  void Print(const char* string);

  void Print(v8::Handle<v8::Value> value);

  void Println(v8::Handle<v8::Value> value);

  void Write(v8::Handle<v8::Value> value);

private:
  FILE* file_;
  int error_;
  std::string message_;
};

class commonjs::PrintStream {
public:
  static v8::Local<v8::FunctionTemplate> NewFunctionTemplate(FILE* file);

  static v8::Persistent<v8::Object> New(FILE* file);

private:
  static void Delete(v8::Persistent<v8::Value> object, void* parameter);

  static v8::Handle<v8::Value> Close(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Flush(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetError(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> CheckError(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Print(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Println(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Write(const v8::Arguments& arguments);
};

#endif // V8_COMMONJS_IO_H

// vim: tabstop=2:sw=2:expandtab
