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
#include <climits>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include "moka/io/file-stream.h"
#include <sstream>

namespace moka {

namespace io {

FileStream::FileStream()
  : fileno_(-1)
  , buffer_(NULL)
  , length_(0) {}

FileStream::~FileStream() {
  if (-1 < fileno_) {
    ::close(fileno_);
  }
}

v8::Handle<v8::Value> FileStream::New(const char* file_name, const char* mode) {
  v8::Handle<v8::Value> argv[2] = {
    v8::String::New(file_name),
    v8::String::New(mode)
  };
  return GetTemplate()->GetFunction()->NewInstance(2, argv);
}

v8::Handle<v8::Value> FileStream::New(int fd) {
  v8::Handle<v8::Value> argv[1] = { v8::Int32::New(fd) };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

v8::Handle<v8::FunctionTemplate> FileStream::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->Inherit(Stream::GetTemplate());
  templ->SetClassName(v8::String::NewSymbol("FileStream"));
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> FileStream::Close() {
  if (-1 == ::close(fileno_)) {
    std::stringstream stream;
    stream << fileno_;
    return v8::ThrowException(
        Module::ErrnoException::New(stream.str().c_str(), errno));
  }
  fileno_ = -1;
  return v8::True();
}

v8::Handle<v8::Value> FileStream::Read(v8::Handle<v8::Object> buffer,
    size_t offset, size_t count) {
  v8::Handle<v8::Value> value = EnsureBuffer(count);
  if (value->IsUndefined()) {
    return value;
  }
  size_t bytes = 0;
  while (count > SSIZE_MAX) {
    ssize_t status = ::read(fileno_, buffer_ + bytes, SSIZE_MAX);
    if (-1 == status) {
      std::stringstream stream;
      stream << fileno_;
      return v8::ThrowException(
          Module::ErrnoException::New(stream.str().c_str(), errno));
    }
    bytes += status;
    count -= status;
    if (0 == status) {
      break;
    }
  }
  while (bytes < count) {
    ssize_t status = ::read(fileno_, buffer_ + bytes, count);
    if (-1 == status) {
      std::stringstream stream;
      stream << fileno_;
      return v8::ThrowException(
          Module::ErrnoException::New(stream.str().c_str(), errno));
    }
    bytes += status;
    count -= status;
    if (0 == status) {
      break;
    }
  }
  value = PruneBuffer();
  if (value->IsUndefined()) {
    return value;
  }
  for (size_t index = 0; index < bytes; ++index) {
    buffer->Set(index + offset, v8::Uint32::New(buffer_[index]));
  }
  return v8::Uint32::New(bytes);
}

v8::Handle<v8::Value> FileStream::Write(const char* buffer, size_t offset,
    size_t count) {
  size_t bytes = 0;
  while (count > SSIZE_MAX) {
    ssize_t status = ::write(fileno_, buffer + offset, SSIZE_MAX);
    if (-1 == status) {
    } else if (0 == status) {
      return v8::Uint32::New(bytes);
    }
    offset += status;
    bytes += status;
    count -= status;
  }
  while (bytes < count) {
    ssize_t status = ::write(fileno_, buffer + offset, count);
    if (-1 == status) {
    } else if (0 == status) {
      return v8::Uint32::New(bytes);
    }
    offset += status;
    bytes += status;
    count -= status;
  }
  return v8::Uint32::New(bytes);
}

v8::Handle<v8::Value> FileStream::Fileno() {
  return v8::Int32::New(fileno_);
}

v8::Handle<v8::Value> FileStream::Isatty() {
  return ::isatty(fileno_) ? v8::True() : v8::False();
}

v8::Handle<v8::Value> FileStream::Tell() {
  off_t position = ::lseek(fileno_, 0, SEEK_CUR);
  if (-1 == position) {
    std::stringstream stream;
    stream << fileno_;
    return v8::ThrowException(
        Module::ErrnoException::New(stream.str().c_str(), errno));
  }
  return v8::Uint32::New(position);
}

v8::Handle<v8::Value> FileStream::Seek(off_t offset, int whence) {
  off_t position = ::lseek(fileno_, offset, whence);
  if (-1 == position) {
    std::stringstream stream;
    stream << fileno_;
    return v8::ThrowException(
        Module::ErrnoException::New(stream.str().c_str(), errno));
  }
  return v8::Uint32::New(position);
}

v8::Handle<v8::Value> FileStream::Truncate(off_t length) {
  int status = ::ftruncate(fileno_, length);
  if (-1 == status) {
    std::stringstream stream;
    stream << fileno_;
    return v8::ThrowException(
        Module::ErrnoException::New(stream.str().c_str(), errno));
  }
  return v8::Uint32::New(length);
}

v8::Handle<v8::Value> FileStream::New(
    const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  int flags = O_RDONLY;
  FileStream* self = NULL;
  switch (arguments.Length()) {
  case 2:
    if (arguments[1]->IsString()) {
      flags = 0;
      bool read = false, write = false, append = false, plus = false;
      bool read_write_append = false;
      v8::String::AsciiValue ascii_mode(arguments[1]->ToString());
      const char* mode = *ascii_mode;
      for (int index = 0; index < ascii_mode.length(); ++index) {
        switch (mode[index]) {
        case 'r':
          if (read_write_append) {
            return v8::ThrowException(v8::Exception::TypeError(
                  v8::String::New("Exactly one of read/write/append "
                    "is allowed")));
          }
          read = read_write_append = true;
          break;
        case 'w':
          if (read_write_append) {
            return v8::ThrowException(v8::Exception::TypeError(
                  v8::String::New("Exactly one of read/write/append "
                    "is allowed")));
          }
          write = read_write_append = true;
          flags |= O_CREAT | O_TRUNC;
          break;
        case 'a':
          if (read_write_append) {
            return v8::ThrowException(v8::Exception::TypeError(
                  v8::String::New("Exactly one of read/write/append "
                    "is allowed")));
          }
          append = write = read_write_append = true;
          flags |= O_CREAT;
          break;
        case '+':
          if (plus) {
            return v8::ThrowException(v8::Exception::TypeError(
                  v8::String::New("'+' may only occur once")));
          }
          plus = read = write = true;
          break;
        default:
          std::string message("Invalid mode: '");
          message.append(1, mode[index]);
          message.append(1, '\'');
          return v8::ThrowException(v8::Exception::TypeError(
                v8::String::New(message.c_str())));
        }
      }
      if (read && write) {
        flags |= O_RDWR;
      } else if (read) {
        flags |= O_RDONLY;
      } else if (write) {
        flags |= O_WRONLY;
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("One of read/write/append is required")));
      }
      if (append) {
        flags |= O_APPEND;
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be a string")));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsString()) {
      self = new FileStream;
      if (self) {
        v8::Handle<v8::Value> value = self->Construct(
            *v8::String::Utf8Value(arguments[0]->ToString()), flags);
        if (value->IsUndefined()) {
          delete self;
          return value;
        }
      }
    } else if (arguments[0]->IsInt32()) {
      // File descriptor
      self = new FileStream;
      if (self) {
        v8::Handle<v8::Value> value =
          self->Construct(arguments[0]->ToInt32()->Value());
        if (value->IsUndefined()) {
          delete self;
          return value;
        }
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be a string")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One or two argument(s) allowed")));
  }
  if (!self) {
    return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(FileStream));
  v8::Persistent<v8::Object> file_stream =
    v8::Persistent<v8::Object>::New(arguments.This());
  file_stream->SetInternalField(0, v8::External::New(self));
  file_stream.MakeWeak(static_cast<void*>(self), Delete);
  return file_stream;
}

void FileStream::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<FileStream*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(
      -static_cast<int>(sizeof(FileStream)));
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> FileStream::Construct(const char* file_name, int mode) {
  fileno_ = ::open(file_name, mode, 0666);
  if (fileno_ < 0) {
    return v8::ThrowException(Module::ErrnoException::New(file_name, errno));
  }
  readable_ = mode & O_RDWR || mode & O_RDONLY;
  writable_ = mode & O_RDWR || mode & O_WRONLY;
  if (mode & O_APPEND) {
    if (-1 == ::lseek(fileno_, 0, SEEK_END)) {
      return v8::ThrowException(Module::ErrnoException::New(file_name, errno));
    }
    seekable_ = true;
  } else {
    seekable_ = -1 == ::lseek(fileno_, 0, SEEK_CUR) ? false : true;
  }
  return v8::True();
}

v8::Handle<v8::Value> FileStream::Construct(int fd) {
  fileno_ = fd;
  int mode = ::fcntl(fileno_, F_GETFL);
  if (-1 == mode) {
    std::stringstream stream;
    stream << fd;
    return v8::ThrowException(
        Module::ErrnoException::New(stream.str().c_str(), errno));
  }
  readable_ = mode & O_RDWR || mode & O_RDONLY;
  writable_ = mode & O_RDWR || mode & O_WRONLY;
  seekable_ = -1 == ::lseek(fileno_, 0, SEEK_CUR) ? false : true;
  return v8::True();
}

v8::Handle<v8::Value> FileStream::EnsureBuffer(size_t length) {
  if (length_ < length) {
    if (length < BUFSIZ) {
      length = BUFSIZ;
    }
    char* buffer = static_cast<char*>(
        ::realloc(static_cast<void*>(buffer_), length));
    if (!buffer) {
      return v8::ThrowException(Module::ErrnoException::New(errno));
    }
    v8::V8::AdjustAmountOfExternalAllocatedMemory(length - length_);
    buffer_ = buffer;
    length_ = length;
  }
  return v8::True();
}

v8::Handle<v8::Value> FileStream::PruneBuffer() {
  if (BUFSIZ < length_) {
    char* buffer = static_cast<char*>(
        ::realloc(static_cast<void*>(buffer_), BUFSIZ));
    if (!buffer) {
      return v8::ThrowException(Module::ErrnoException::New(errno));
    }
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-(length_ - BUFSIZ));
    buffer_ = buffer;
    length_ = BUFSIZ;
  }
  return v8::True();
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
