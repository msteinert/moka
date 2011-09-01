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
#include "moka/io/buffer.h"
#include "moka/io/error.h"
#include "moka/io/file-stream.h"
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace moka {

namespace io {

FileStream::FileStream()
  : fileno_(-1) {}

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
  // FileStream functions
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("close"),
      v8::FunctionTemplate::New(Close)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("read"),
      v8::FunctionTemplate::New(Read)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("write"),
      v8::FunctionTemplate::New(Write)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("fileno"),
      v8::FunctionTemplate::New(Fileno)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("isatty"),
      v8::FunctionTemplate::New(Isatty)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("tell"),
      v8::FunctionTemplate::New(Tell)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("seek"),
      v8::FunctionTemplate::New(Seek)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("truncate"),
      v8::FunctionTemplate::New(Truncate)->GetFunction());
  // FileStream properties
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

v8::Handle<v8::Value> FileStream::Read(Buffer* buffer) {
  return Read(buffer, 0, buffer->GetLength());
}

v8::Handle<v8::Value> FileStream::Read(Buffer* buffer, size_t count) {
  return Read(buffer, 0, count);
}

v8::Handle<v8::Value> FileStream::Read(Buffer* buffer, size_t offset,
    size_t count) {
  if (!count) {
    return v8::Uint32::New(0);
  }
  if (count > buffer->GetLength() - offset) {
    return ThrowException(v8::Exception::RangeError(
          v8::String::New("Buffer is not large enough")));
  }
  return Read(buffer->GetBuffer(), offset, count);
}

v8::Handle<v8::Value> FileStream::Read(char* buffer, size_t offset,
    size_t count) {
  size_t bytes = 0;
  while (count > SSIZE_MAX) {
    ssize_t status = ::read(fileno_, buffer + offset, SSIZE_MAX);
    if (-1 == status) {
      std::stringstream stream;
      stream << fileno_;
      return v8::ThrowException(
          Module::ErrnoException::New(stream.str().c_str(), errno));
    } else if (0 == status) {
      // End of file
      return v8::Uint32::New(bytes);
    }
    offset += status;
    bytes += status;
    count -= status;
  }
  while (bytes < count) {
    ssize_t status = ::read(fileno_, buffer + offset, count);
    if (-1 == status) {
      std::stringstream stream;
      stream << fileno_;
      return v8::ThrowException(
          Module::ErrnoException::New(stream.str().c_str(), errno));
    } else if (0 == status) {
      // End of file
      return v8::Uint32::New(bytes);
    }
    offset += status;
    bytes += status;
    count -= status;
  }
  return v8::Uint32::New(bytes);
}

v8::Handle<v8::Value> FileStream::Read() {
  size_t size = BUFSIZ > SSIZE_MAX ? SSIZE_MAX : BUFSIZ;
  char* buffer = static_cast<char*>(::malloc(size));
  if (!buffer) {
    return v8::ThrowException(Module::ErrnoException::New(errno));
  }
  ssize_t bytes;
  v8::Local<v8::String> string;
  while (true) {
    bytes = ::read(fileno_, buffer, size);
    if (-1 == bytes) {
      ::free(buffer);
      std::stringstream stream;
      stream << fileno_;
      return v8::ThrowException(
          Module::ErrnoException::New(stream.str().c_str(), errno));
    } else if (0 == bytes) {
      // End of file
      if (string.IsEmpty()) {
        ::free(buffer);
        return v8::String::New(buffer, bytes);
      } else {
        ::free(buffer);
        return v8::String::Concat(string, v8::String::New(buffer, bytes));
      }
    }
    if (string.IsEmpty()) {
      string = v8::String::New(buffer, bytes);
    } else {
      string = v8::String::Concat(string, v8::String::New(buffer, bytes));
    }
  }
}

v8::Handle<v8::Value> FileStream::Read(size_t count) {
  char* buffer = static_cast<char*>(::malloc(count));
  if (!buffer) {
    return v8::ThrowException(Module::ErrnoException::New(errno));
  }
  v8::Handle<v8::Value> value = Read(buffer, 0, count);
  if (value->IsUndefined()) {
    ::free(buffer);
    return value;
  }
  v8::Local<v8::String> string =
    v8::String::New(buffer, value->ToUint32()->Value());
  ::free(buffer);
  return string;
}

v8::Handle<v8::Value> FileStream::Write(Buffer* buffer) {
  return Write(buffer, 0, buffer->GetLength());
}

v8::Handle<v8::Value> FileStream::Write(v8::String::Utf8Value& string) {
  return Write(string, 0, string.length());
}

v8::Handle<v8::Value> FileStream::Write(Buffer* buffer, size_t count) {
  return Write(buffer, 0, count);
}

v8::Handle<v8::Value> FileStream::Write(v8::String::Utf8Value& string,
    size_t count) {
  return Write(string, 0, count);
}

v8::Handle<v8::Value> FileStream::Write(Buffer* buffer, size_t offset,
    size_t count) {
  if (!buffer->GetLength()) {
    return v8::Uint32::New(0);
  }
  if (count > buffer->GetLength() - offset) {
    count = buffer->GetLength() - offset;
  }
  return Write(buffer->GetBuffer(), offset, count);
}

v8::Handle<v8::Value> FileStream::Write(v8::String::Utf8Value& string,
    size_t offset, size_t count) {
  if (!string.length()) {
    return v8::Uint32::New(0);
  }
  if (count > string.length() - offset) {
    count = string.length() - offset;
  }
  return Write(*string, offset, count);
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

v8::Handle<v8::Value> FileStream::Close(const v8::Arguments& arguments) {
  switch (arguments.Length()) {
  case 0:
    {
      FileStream* self = static_cast<FileStream*>(
          arguments.This()->GetPointerFromInternalField(0));
      v8::Handle<v8::Value> value = self->Close();
      if (value->IsUndefined()) {
        return value;
      }
      return v8::True();
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero arguments allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Read(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("read: File is closed"));
  }
  if (!self->readable_) {
    return v8::ThrowException(Error::New("read: File is not readable"));
  }
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->IsUint32()) {
      if (arguments[1]->IsUint32()) {
        if (arguments[0]->IsObject()) {
          v8::Handle<v8::Object> object = arguments[0]->ToObject();
          if (Buffer::GetTemplate()->HasInstance(object)) {
            Buffer* buffer = static_cast<Buffer*>(
                object->GetPointerFromInternalField(0));
            return self->Read(buffer, arguments[1]->ToUint32()->Value(),
                arguments[2]->ToUint32()->Value());
          }
        }
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be of type Buffer")));
      }
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument two must be an unsigned integer")));
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument three must be an unsigned integer")));
  case 2:
    if (arguments[1]->IsUint32()) {
      if (arguments[0]->IsObject()) {
        v8::Handle<v8::Object> object = arguments[0]->ToObject();
        if (Buffer::GetTemplate()->HasInstance(object)) {
          Buffer* buffer = static_cast<Buffer*>(
              object->GetPointerFromInternalField(0));
          return self->Read(buffer, arguments[1]->ToUint32()->Value());
        }
      }
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be of type Buffer")));
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument two must be an unsigned integer")));
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (Buffer::GetTemplate()->HasInstance(object)) {
        Buffer* buffer = static_cast<Buffer*>(
            object->GetPointerFromInternalField(0));
        return self->Read(buffer);
      }
    } else if (arguments[0]->IsUint32()) {
      return self->Read(arguments[0]->ToUint32()->Value());
    }
    return v8::ThrowException(v8::Exception::TypeError(
        v8::String::New("Argument one must be an unsigned integer or "
          "of type Buffer")));
  case 0:
    return self->Read();
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two, or three argument(s) allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Write(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("write: File is closed"));
  }
  if (!self->writable_) {
    return v8::ThrowException(Error::New("write: File is not writable"));
  }
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->IsUint32()) {
      if (arguments[1]->IsUint32()) {
        if (arguments[0]->IsObject()) {
          v8::Handle<v8::Object> object = arguments[0]->ToObject();
          if (Buffer::GetTemplate()->HasInstance(object)) {
            Buffer* buffer = static_cast<Buffer*>(
                object->GetPointerFromInternalField(0));
            return self->Write(buffer, arguments[1]->ToUint32()->Value(),
                arguments[2]->ToUint32()->Value());
          }
        } else {
          v8::String::Utf8Value string(arguments[0]->ToString());
          return self->Write(string, arguments[1]->ToUint32()->Value(),
              arguments[2]->ToUint32()->Value());
        }
      }
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument two must be an unsigned integer")));
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument three must be an unsigned integer")));
  case 2:
    if (arguments[1]->IsUint32()) {
      if (arguments[0]->IsObject()) {
        v8::Handle<v8::Object> object = arguments[0]->ToObject();
        if (Buffer::GetTemplate()->HasInstance(object)) {
          Buffer* buffer = static_cast<Buffer*>(
              object->GetPointerFromInternalField(0));
          return self->Write(buffer, arguments[1]->ToUint32()->Value());
        }
      } else {
        v8::String::Utf8Value string(arguments[0]->ToString());
        return self->Write(string, arguments[1]->ToUint32()->Value());
      }
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument two must be an unsigned integer")));
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (Buffer::GetTemplate()->HasInstance(object)) {
        Buffer* buffer = static_cast<Buffer*>(
            object->GetPointerFromInternalField(0));
        return self->Write(buffer);
      }
    } else {
      v8::String::Utf8Value string(arguments[0]->ToString());
      return self->Write(string);
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two, or three argument(s) allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Fileno(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("fileno: File is closed"));
  }
  switch (arguments.Length()) {
  case 0:
    return self->Fileno();
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero arguments allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Isatty(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("isatty: File is closed"));
  }
  switch (arguments.Length()) {
  case 0:
    return self->Isatty();
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero arguments allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Tell(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("tell: File is closed"));
  }
  if (!self->seekable_) {
    return v8::ThrowException(Error::New("tell: File is not seekable"));
  }
  switch (arguments.Length()) {
  case 0:
    return self->Tell();
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero arguments allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Seek(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("seek: File is closed"));
  }
  if (!self->seekable_) {
    return v8::ThrowException(Error::New("seek: File is not seekable"));
  }
  int whence = SEEK_SET;
  switch (arguments.Length()) {
  case 2:
    if (arguments[1]->IsInt32()) {
      whence = arguments[1]->ToInt32()->Value();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be one of "
              "SEEK_SET/SEEK_CUR/SEEK_END")));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsInt32()) {
      return self->Seek(arguments[0]->ToInt32()->Value(), whence);
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument one must be an integer")));
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One or two argument(s) allowed")));
  }
}

v8::Handle<v8::Value> FileStream::Truncate(const v8::Arguments& arguments) {
  FileStream* self = static_cast<FileStream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (-1 == self->fileno_) {
    return v8::ThrowException(Error::New("truncate: File is closed"));
  }
  if (!self->seekable_) {
    return v8::ThrowException(Error::New("truncate: File is not seekable"));
  }
  off_t length = 0;
  switch (arguments.Length()) {
  case 1:
    if (arguments[1]->IsInt32()) {
      length = arguments[1]->ToInt32()->Value();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an integer")));
    }
    // Fall through
  case 0:
    return self->Truncate(length);
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero or one argument(s) allowed")));
  }
}

v8::Handle<v8::Value> FileStream::ClosedGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  FileStream* self = static_cast<FileStream*>(
      info.This()->GetPointerFromInternalField(0));
  return -1 == self->fileno_ ? v8::True() : v8::False();
}

v8::Handle<v8::Value> FileStream::ReadableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  FileStream* self = static_cast<FileStream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->readable_ ? v8::True() : v8::False();
}

v8::Handle<v8::Value> FileStream::WritableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  FileStream* self = static_cast<FileStream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->writable_ ? v8::True() : v8::False();
}

v8::Handle<v8::Value> FileStream::SeekableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  FileStream* self = static_cast<FileStream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->seekable_ ? v8::True() : v8::False();
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

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
