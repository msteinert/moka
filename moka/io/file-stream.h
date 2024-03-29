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

#ifndef MOKA_IO_FILE_STREAM_H
#define MOKA_IO_FILE_STREAM_H

#include "moka/io/stream.h"

namespace moka {

namespace io {

class FileStream;

} // namespace io

} // namespace moka

class moka::io::FileStream: public moka::io::Stream {
public:
  static v8::Handle<v8::Value> New(const char* file_name, const char* mode);

  static v8::Handle<v8::Value> New(int fd);

  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  virtual v8::Handle<v8::Value> Close();

  virtual v8::Handle<v8::Value> Read(v8::Handle<v8::Object> buffer,
      size_t offset, size_t count);

  virtual v8::Handle<v8::Value> Write(const char* buffer, size_t offset,
      size_t count);

  virtual v8::Handle<v8::Value> Fileno();

  virtual v8::Handle<v8::Value> Isatty();

  virtual v8::Handle<v8::Value> Tell();

  virtual v8::Handle<v8::Value> Seek(off_t offset, int whence);

  virtual v8::Handle<v8::Value> Truncate(off_t length);

  virtual bool Closed() {
    return -1 == fileno_ ? true : false;
  }

  virtual bool Readable() {
    return readable_;
  }

  virtual bool Writable() {
    return writable_;
  }

  virtual bool Seekable() {
    return seekable_;
  }

protected: // V8 interface methods
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

protected: // Protected methods
  v8::Handle<v8::Value> Construct(const char* file_name, int mode);

  v8::Handle<v8::Value> Construct(int fd);

  FileStream();

  virtual ~FileStream();

private: // Private methods
  FileStream(FileStream const& that);

  void operator=(FileStream const& that);

  v8::Handle<v8::Value> EnsureBuffer(size_t length);

  v8::Handle<v8::Value> PruneBuffer();

private: // Private data
  int fileno_;
  bool readable_;
  bool writable_;
  bool seekable_;
  char* buffer_;
  size_t length_;
};

#endif // MOKA_IO_FILE_STREAM_H

// vim: tabstop=2:sw=2:expandtab
