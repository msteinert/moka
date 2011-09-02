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

#ifndef MOKA_IO_BUFFERED_STREAM_H
#define MOKA_IO_BUFFERED_STREAM_H

#include "moka/io/stream.h"

namespace moka {

namespace io {

class BufferedStream;

} // namespace io

} // namespace moka

class moka::io::BufferedStream: public moka::io::Stream {
public:
  static v8::Handle<v8::Value> New(v8::Handle<v8::Value> stream);

  static v8::Handle<v8::Value> New(v8::Handle<v8::Value> stream,
      size_t buffer_size);

  static v8::Handle<v8::Value> New(v8::Handle<v8::Value> stream,
      v8::Handle<v8::Value> buffer);

  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  virtual v8::Handle<v8::Value> Close() {
    return static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0))->Close();
  }

  virtual v8::Handle<v8::Value> Read(v8::Handle<v8::Object> buffer,
      size_t offset, size_t count);

  virtual v8::Handle<v8::Value> Write(v8::Handle<v8::Object> buffer,
      size_t offset, size_t count);

  virtual v8::Handle<v8::Value> Flush();

  virtual v8::Handle<v8::Value> Fileno() {
    Stream* stream = static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0));
    return stream->Fileno();
  }

  virtual v8::Handle<v8::Value> Isatty() {
    Stream* stream = static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0));
    return stream->Isatty();
  }

  virtual v8::Handle<v8::Value> Tell();

  virtual v8::Handle<v8::Value> Seek(off_t offset, int whence);

  virtual v8::Handle<v8::Value> Truncate(off_t length = 0);

  virtual bool Closed() {
    return static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0))->Closed();
  }

  virtual bool Readable() {
    return static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0))->Readable();
  }

  virtual bool Writable() {
    return static_cast<Stream*>(
        stream_->GetPointerFromInternalField(0))->Writable();
  }

  virtual bool Seekable() {
    return true;
  }

protected: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

protected: // Protected methods
  v8::Handle<v8::Value> Construct(v8::Handle<v8::Object> stream);

  v8::Handle<v8::Value> Construct(v8::Handle<v8::Object> stream,
      size_t buffer_size);

  BufferedStream();

  virtual ~BufferedStream();

private: // Private methods
  BufferedStream(BufferedStream const& that);

  void operator=(BufferedStream const& that);

private: // Private data
  v8::Persistent<v8::Object> stream_;
  v8::Persistent<v8::Object> read_;
  size_t read_start_, read_end_;
  v8::Persistent<v8::Object> write_;
  size_t write_start_, write_end_;
};

#endif // MOKA_IO_BUFFERED_STREAM_H

// vim: tabstop=2:sw=2:expandtab
