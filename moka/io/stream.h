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

#ifndef MOKA_IO_STREAM_H
#define MOKA_IO_STREAM_H

#include "moka/module.h"

namespace moka {

namespace io {

class Stream;

} // namespace io

} // namespace moka

class moka::io::Stream {
public:
  static bool GetClosed(v8::Handle<v8::Value> stream) {
    return Get(stream, "closed");
  }

  static void SetClosed(v8::Handle<v8::Value> stream, bool readable) {
    Set(stream, "closed", readable);
  }

  static bool GetReadable(v8::Handle<v8::Value> stream) {
    return Get(stream, "readable");
  }

  static void SetReadable(v8::Handle<v8::Value> stream, bool readable) {
    Set(stream, "readable", readable);
  }

  static bool GetWritable(v8::Handle<v8::Value> stream) {
    return Get(stream, "writable");
  }

  static void SetWritable(v8::Handle<v8::Value> stream, bool writable) {
    Set(stream, "writable", writable);
  }

  static bool GetSeekable(v8::Handle<v8::Value> stream) {
    return Get(stream, "seekable");
  }

  static void SetSeekable(v8::Handle<v8::Value> stream, bool seekable) {
    Set(stream, "seekable", seekable);
  }

  static v8::Handle<v8::FunctionTemplate> GetTemplate();

protected: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Close(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Read(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Write(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Flush(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Fileno(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Isatty(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Tell(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> Seek(const v8::Arguments& arguments);

protected: // Protected methods
  Stream() {}

  virtual ~Stream() {}

private: // Private methods
  Stream(Stream const& that);

  void operator=(Stream const& that);

  static bool Get(v8::Handle<v8::Value> stream, const char* property) {
    return stream->ToObject()->Get(
        v8::String::NewSymbol(property))->ToBoolean()->Value();
  }

  static void Set(v8::Handle<v8::Value> stream, const char* property,
      bool value) {
    stream->ToObject()->Set(v8::String::NewSymbol(property),
        value ? v8::True() : v8::False(),
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
};

#endif // MOKA_IO_STREAM_H

// vim: tabstop=2:sw=2:expandtab
