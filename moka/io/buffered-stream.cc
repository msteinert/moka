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
#include "moka/io/buffer.h"
#include "moka/io/buffered-stream.h"
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace moka {

namespace io {

BufferedStream::BufferedStream()
  : read_start_(0)
  , read_end_(0)
  , write_start_(0)
  , write_end_(0) {}

BufferedStream::~BufferedStream() {
  if (!stream_.IsEmpty()) {
    stream_.Dispose();
  }
  if (!read_.IsEmpty()) {
    read_.Dispose();
  }
  if (!write_.IsEmpty()) {
    write_.Dispose();
  }
}

v8::Handle<v8::Value> BufferedStream::New(v8::Handle<v8::Value> stream) {
  v8::Handle<v8::Value> argv[1] = { stream };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

v8::Handle<v8::Value> BufferedStream::New(v8::Handle<v8::Value> stream,
    size_t buffer_size) {
  v8::Handle<v8::Value> argv[2] = { stream, v8::Uint32::New(buffer_size) };
  return GetTemplate()->GetFunction()->NewInstance(2, argv);
}

v8::Handle<v8::Value> BufferedStream::New(v8::Handle<v8::Value> stream,
    v8::Handle<v8::Value> buffer) {
  v8::Handle<v8::Value> argv[2] = { stream, buffer };
  return GetTemplate()->GetFunction()->NewInstance(2, argv);
}

v8::Handle<v8::Value> BufferedStream::Read(v8::Handle<v8::Object> buffer,
    size_t offset, size_t count) {
  // TODO
  return v8::Uint32::New(0);
}

v8::Handle<v8::Value> BufferedStream::Write(v8::Handle<v8::Object> buffer,
    size_t offset, size_t count) {
  // TODO
  return v8::Uint32::New(0);
}

v8::Handle<v8::Value> BufferedStream::Flush() {
  uint32_t bytes = write_end_ - write_start_;
  if (bytes > 0) {
    v8::Handle<v8::Value> value =
      Stream::Write(stream_, write_, write_start_, bytes);
    if (value->IsUndefined()) {
      return value;
    }
    write_start_ = write_end_ = 0;
  }
  return v8::True();
}

v8::Handle<v8::Value> BufferedStream::Tell() {
  // TODO
  return v8::Uint32::New(0);
}

v8::Handle<v8::Value> BufferedStream::Seek(off_t offset, int whence) {
  // TODO
  return v8::Uint32::New(0);
}

v8::Handle<v8::Value> BufferedStream::Truncate(off_t length) {
  // TODO
  return v8::Uint32::New(0);
}

v8::Handle<v8::FunctionTemplate> BufferedStream::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->Inherit(Stream::GetTemplate());
  templ->SetClassName(v8::String::NewSymbol("BufferedStream"));
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> BufferedStream::New(
    const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  BufferedStream* self = NULL;
  switch (arguments.Length()) {
  case 2:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> stream = arguments[0]->ToObject();
      if (!Stream::GetTemplate()->HasInstance(stream)) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be a Stream")));
      }
      if (arguments[1]->IsUint32()) {
        self = new BufferedStream;
        if (self) {
          v8::Handle<v8::Value> value = self->Construct(stream,
              arguments[1]->ToUint32()->Value());
          if (value->IsUndefined()) {
            return value;
          }
        }
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an an unsigned integer")));
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an object")));
    }
    break;
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> stream = arguments[0]->ToObject();
      if (!Stream::GetTemplate()->HasInstance(stream)) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be a Stream")));
      }
      self = new BufferedStream;
      if (self) {
        v8::Handle<v8::Value> value = self->Construct(stream);
        if (value->IsUndefined()) {
          return value;
        }
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an object")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One or two argument(s) allowed")));
  }
  if (!self) {
    return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(BufferedStream));
  v8::Persistent<v8::Object> buffered_stream =
    v8::Persistent<v8::Object>::New(arguments.This());
  buffered_stream->SetInternalField(0, v8::External::New(self));
  buffered_stream.MakeWeak(static_cast<void*>(self), Delete);
  return buffered_stream;
}

void BufferedStream::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<BufferedStream*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(
      -static_cast<int>(sizeof(BufferedStream)));
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> BufferedStream::Construct(v8::Handle<v8::Object> stream) {
  v8::TryCatch try_catch;
  size_t buffer_size = BUFSIZ;
  v8::Handle<v8::Value> value = Stream::Fileno(stream);
  if (!value->IsUndefined() && value->IsInt32()) {
    struct stat buf;
    int status = ::fstat(value->ToInt32()->Value(), &buf);
    if (-1 != status && buf.st_blksize) {
      buffer_size = buf.st_blksize;
    }
  }
  return Construct(stream, buffer_size);
}

v8::Handle<v8::Value> BufferedStream::Construct(v8::Handle<v8::Object> stream,
    size_t buffer_size) {
  if (Stream::Readable(stream)) {
    v8::Handle<v8::Value> read = Buffer::New(buffer_size);
    if (read->IsUndefined()) {
      return read;
    }
    read_ = v8::Persistent<v8::Object>::New(read->ToObject());
    read_start_ = read_end_ = 0;
  }
  if (Stream::Writable(stream)) {
    v8::Handle<v8::Value> write = Buffer::New(buffer_size);
    if (write->IsUndefined()) {
      return write;
    }
    write_ = v8::Persistent<v8::Object>::New(write->ToObject());
    write_start_ = write_end_ = 0;
  }
  stream_ = v8::Persistent<v8::Object>::New(stream);
  return v8::True();
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
