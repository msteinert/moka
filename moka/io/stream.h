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
class Buffer;

} // namespace io

} // namespace moka

class moka::io::Stream {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  virtual v8::Handle<v8::Value> Close();

  /**
   * \brief Read bytes into an array-like object
   *
   * \param buffer [in/out] The array-like object to read into
   *
   * \return The number of bytes read.
   */
  v8::Handle<v8::Value> Read(v8::Handle<v8::Object> buffer);

  /**
   * \brief Read bytes into an array-like object
   *
   * \param buffer [in/out] The array-like object to read into
   * \param count [in] The number of bytes to read
   *
   * \return The number of bytes read.
   */
  v8::Handle<v8::Value> Read(v8::Handle<v8::Object> buffer, size_t count);

  /**
   * \brief Read bytes into an array-like object starting from an offset
   *
   * \param buffer [in/out] The array-like object to read into
   * \param offset [in] The offset in buffer to start writing to
   * \param count [in] The number of bytes to read
   *
   * \return The number of bytes read.
   */
  virtual v8::Handle<v8::Value> Read(v8::Handle<v8::Object> buffer,
      size_t offset, size_t count);

  /**
   * \brief Write bytes from a buffer
   *
   * \param buffer [in] The buffer to write from
   *
   * \return The number of bytes written.
   */
  v8::Handle<v8::Value> Write(Buffer* buffer);

  v8::Handle<v8::Value> Write(v8::String::Utf8Value& string);

  v8::Handle<v8::Value> Write(Buffer* buffer, size_t count);

  v8::Handle<v8::Value> Write(v8::String::Utf8Value& string, size_t count);

  v8::Handle<v8::Value> Write(Buffer* buffer, size_t offset, size_t count);

  v8::Handle<v8::Value> Write(v8::String::Utf8Value& string, size_t offset,
      size_t count);

  virtual v8::Handle<v8::Value> Write(const char* buffer, size_t offset,
      size_t count);

  virtual v8::Handle<v8::Value> Flush();

  virtual v8::Handle<v8::Value> Fileno();

  virtual v8::Handle<v8::Value> Isatty();

  /**
   * \return The absolute position.
   */
  virtual v8::Handle<v8::Value> Tell();

  /**
   * \return The new absolute position.
   */
  virtual v8::Handle<v8::Value> Seek(off_t offset, int whence);

  /**
   * \return The new file size.
   */
  virtual v8::Handle<v8::Value> Truncate(off_t length);

  virtual bool Closed() {
    return true;
  }

  virtual bool Readable() {
    return false;
  }

  virtual bool Writable() {
    return false;
  }

  virtual bool Seekable() {
    return false;
  }

public: // Convenience methods
  static v8::Handle<v8::Value> Write(v8::Handle<v8::Object> stream,
      v8::Handle<v8::Object> buffer, size_t offset, size_t count);

  static v8::Handle<v8::Value> Fileno(v8::Handle<v8::Object> stream);

  static bool Readable(v8::Handle<v8::Object> stream);

  static bool Writable(v8::Handle<v8::Object> stream);

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

  static v8::Handle<v8::Value> Truncate(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> ClosedGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> ReadableGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> WritableGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> SeekableGet(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

protected: // Protected methods
  Stream() {}

  virtual ~Stream() {}

private: // Private methods
  Stream(Stream const& that);

  void operator=(Stream const& that);
};

#endif // MOKA_IO_STREAM_H

// vim: tabstop=2:sw=2:expandtab
