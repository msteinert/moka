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

#include "moka/io/error.h"
#include "moka/io/stream.h"
#include <string>

namespace moka {

namespace io {

v8::Handle<v8::FunctionTemplate> Stream::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("Stream"));
  // Functions
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("close"),
      v8::FunctionTemplate::New(Close)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("read"),
      v8::FunctionTemplate::New(Read)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("write"),
      v8::FunctionTemplate::New(Write)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("flush"),
      v8::FunctionTemplate::New(Flush)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("fileno"),
      v8::FunctionTemplate::New(Fileno)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("isatty"),
      v8::FunctionTemplate::New(Isatty)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("tell"),
      v8::FunctionTemplate::New(Tell)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("seek"),
      v8::FunctionTemplate::New(Seek)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("truncate"),
      v8::FunctionTemplate::New(Seek)->GetFunction());
  // Properties
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("closed"),
      ClosedGet);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("readable"),
      ReadableGet);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("writable"),
      WritableGet);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("seekable"),
      SeekableGet);
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Stream::New(const v8::Arguments& arguments) {
  std::string message("Cannot instantiate the type ");
  message.append(*v8::String::AsciiValue(
        arguments.This()->GetConstructorName()));
  return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New(message.c_str())));
}

v8::Handle<v8::Value> Stream::Close(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> Stream::Read(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("read: Unsupported"));
}

v8::Handle<v8::Value> Stream::Write(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("write: Unsupported"));
}

v8::Handle<v8::Value> Stream::Flush(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("flush: Unsupported"));
}

v8::Handle<v8::Value> Stream::Fileno(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("fileno: Unsupported"));
}

v8::Handle<v8::Value> Stream::Isatty(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("isatty: Unsupported"));
}

v8::Handle<v8::Value> Stream::Tell(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("tell: Unsupported"));
}

v8::Handle<v8::Value> Stream::Seek(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("seek: Unsupported"));
}

v8::Handle<v8::Value> Stream::Truncate(const v8::Arguments& arguments) {
  return v8::ThrowException(Error::New("truncate: Unsupported"));
}

v8::Handle<v8::Value> Stream::ClosedGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  return v8::True();
}

v8::Handle<v8::Value> Stream::ReadableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  return v8::False();
}

v8::Handle<v8::Value> Stream::WritableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  return v8::False();
}

v8::Handle<v8::Value> Stream::SeekableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  return v8::False();
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
