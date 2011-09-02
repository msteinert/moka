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
#include "moka/io/buffer.h"
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

v8::Handle<v8::Value> Stream::Close() {
  return v8::Undefined();
}

v8::Handle<v8::Value> Stream::Read(v8::Handle<v8::Object> buffer) {
  return Read(buffer, Buffer::Length(buffer));
}

v8::Handle<v8::Value> Stream::Read(v8::Handle<v8::Object> buffer,
        size_t count) {
  if (!count) {
    return v8::Uint32::New(0);
  }
  return Read(buffer, 0, count);
}

v8::Handle<v8::Value> Stream::Read(v8::Handle<v8::Object> buffer,
        size_t offset, size_t count) {
  return v8::ThrowException(Error::New("read: Unsupported"));
}

v8::Handle<v8::Value> Stream::Write(Buffer* buffer) {
  return Write(buffer, buffer->GetLength());
}

v8::Handle<v8::Value> Stream::Write(v8::String::Utf8Value& string) {
  return Write(string, string.length());
}

v8::Handle<v8::Value> Stream::Write(Buffer* buffer, size_t count) {
  return Write(buffer, 0, buffer->GetLength());
}

v8::Handle<v8::Value> Stream::Write(v8::String::Utf8Value& string,
    size_t count) {
  return Write(string, 0, count);
}

v8::Handle<v8::Value> Stream::Write(Buffer* buffer, size_t offset,
    size_t count) {
  if (!buffer->GetLength()) {
    return v8::Uint32::New(0);
  }
  if (count > buffer->GetLength() - offset) {
    count = buffer->GetLength() - offset;
  }
  return Write(buffer->GetBuffer(), offset, count);
}

v8::Handle<v8::Value> Stream::Write(v8::String::Utf8Value& string,
    size_t offset, size_t count) {
  if (!string.length()) {
    return v8::Uint32::New(0);
  }
  if (count > string.length() - offset) {
    count = string.length() - offset;
  }
  return Write(*string, offset, count);
}

v8::Handle<v8::Value> Stream::Write(const char* buffer, size_t offset,
    size_t count) {
  return v8::ThrowException(Error::New("write: Unsupported"));
}

v8::Handle<v8::Value> Stream::Fileno() {
  return v8::ThrowException(Error::New("fileno: Unsupported"));
}

v8::Handle<v8::Value> Stream::Isatty() {
  return v8::ThrowException(Error::New("isatty: Unsupported"));
}

v8::Handle<v8::Value> Stream::Tell() {
  return v8::ThrowException(Error::New("tell: Unsupported"));
}

v8::Handle<v8::Value> Stream::Seek(off_t offset, int whence = SEEK_SET) {
  return v8::ThrowException(Error::New("seek: Unsupported"));
}

v8::Handle<v8::Value> Stream::Truncate(off_t length = 0) {
  return v8::ThrowException(Error::New("truncate: Unsupported"));
}

v8::Handle<v8::Value> Stream::Flush() {
  return v8::True();
}

v8::Handle<v8::Value> Stream::New(const v8::Arguments& arguments) {
  std::string message("Cannot instantiate the type ");
  message.append(*v8::String::AsciiValue(
        arguments.This()->GetConstructorName()));
  return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New(message.c_str())));
}

v8::Handle<v8::Value> Stream::Close(const v8::Arguments& arguments) {
  switch (arguments.Length()) {
  case 0:
    {
      Stream* self = static_cast<Stream*>(
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

v8::Handle<v8::Value> Stream::Read(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("read: File is closed"));
  }
  if (!self->Readable()) {
    return v8::ThrowException(Error::New("read: File is not readable"));
  }
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->IsUint32()) {
      if (arguments[1]->IsUint32()) {
        if (arguments[0]->IsObject()) {
          return self->Read(arguments[0]->ToObject(),
              arguments[1]->ToUint32()->Value(),
              arguments[2]->ToUint32()->Value());
        }
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an array-like object")));
      }
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument two must be an unsigned integer")));
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument three must be an unsigned integer")));
  case 2:
    if (arguments[1]->IsUint32()) {
      if (arguments[0]->IsObject()) {
        return self->Read(arguments[0]->ToObject(),
            arguments[1]->ToUint32()->Value());
      }
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an array-like object")));
    }
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Argument two must be an unsigned integer")));
  case 1:
    if (arguments[0]->IsObject()) {
      return self->Read(arguments[0]->ToObject());
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an array-like object")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two, or three argument(s) allowed")));
  }

}

v8::Handle<v8::Value> Stream::Write(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("write: File is closed"));
  }
  if (!self->Writable()) {
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

v8::Handle<v8::Value> Stream::Flush(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("flush: File is closed"));
  }
  switch (arguments.Length()) {
  case 0:
    return self->Flush();
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("Zero arguments allowed")));
  }
}

v8::Handle<v8::Value> Stream::Fileno(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
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

v8::Handle<v8::Value> Stream::Isatty(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
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

v8::Handle<v8::Value> Stream::Tell(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("tell: File is closed"));
  }
  if (!self->Seekable()) {
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

v8::Handle<v8::Value> Stream::Seek(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("seek: File is closed"));
  }
  if (!self->Seekable()) {
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

v8::Handle<v8::Value> Stream::Truncate(const v8::Arguments& arguments) {
  Stream* self = static_cast<Stream*>(
      arguments.This()->GetPointerFromInternalField(0));
  if (self->Closed()) {
    return v8::ThrowException(Error::New("truncate: File is closed"));
  }
  if (!self->Seekable()) {
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

v8::Handle<v8::Value> Stream::Write(v8::Handle<v8::Object> stream,
    v8::Handle<v8::Object> buffer, size_t offset, size_t count) {
  v8::Handle<v8::Value> argv[3] = {
    buffer,
    v8::Uint32::New(offset),
    v8::Uint32::New(count)
  };
  return Module::CallMethod(stream, "write", 3, argv);
}

bool Stream::Readable(v8::Handle<v8::Object> stream) {
  v8::Handle<v8::Value> value = stream->Get(v8::String::New("readable"));
  v8::TryCatch try_catch;
  if (value.IsEmpty()) {
    return false;
  }
  return value->ToBoolean()->Value();
}

bool Stream::Writable(v8::Handle<v8::Object> stream) {
  v8::Handle<v8::Value> value = stream->Get(v8::String::New("writable"));
  v8::TryCatch try_catch;
  if (value.IsEmpty()) {
    return false;
  }
  return value->ToBoolean()->Value();
}

v8::Handle<v8::Value> Stream::Fileno(v8::Handle<v8::Object> stream) {
  return Module::CallMethod(stream, "fileno", 0, NULL);
}

v8::Handle<v8::Value> Stream::ClosedGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  Stream* self = static_cast<Stream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->Closed() ? v8::True() : v8::False();
}

v8::Handle<v8::Value> Stream::ReadableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  Stream* self = static_cast<Stream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->Readable() ? v8::True() : v8::False();
}

v8::Handle<v8::Value> Stream::WritableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  Stream* self = static_cast<Stream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->Writable() ? v8::True() : v8::False();
}

v8::Handle<v8::Value> Stream::SeekableGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  Stream* self = static_cast<Stream*>(
      info.This()->GetPointerFromInternalField(0));
  return self->Seekable() ? v8::True() : v8::False();
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
