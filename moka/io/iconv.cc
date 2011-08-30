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
#include <cstdlib>
#include "moka/io/iconv.h"

namespace moka {

namespace io {

Iconv::Iconv()
  : length_(0)
  , buffer_(NULL) {}

Iconv::~Iconv() {
  if (buffer_) {
    ::free(buffer_);
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-length_);
  }
  if (cd_) {
    iconv_close(cd_);
  }
}

v8::Handle<v8::Value> Iconv::Convert(Buffer* in) {
  v8::HandleScope handle_scope;
  size_t size = in->GetLength() * sizeof(wchar_t);
  if (!size) {
    return handle_scope.Close(Buffer::New(0));
  }
  v8::Handle<v8::Value> value = EnsureBuffer(size);
  if (value->IsUndefined()) {
    return handle_scope.Close(value);
  }
  size_t inbytesleft = in->GetLength();
  char* inbuf = in->GetBuffer();
  size_t outbytesleft = length_;
  char* outbuf = buffer_;
  // Flush the iconv handle
  int converted = ::iconv(cd_, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  while (-1 == converted) {
    if (EINVAL == errno) {
      // Junk at the end of the buffer, ignore it
      break;
    } else if (E2BIG == errno) {
      // Double the size of the output buffer
      size *= 2;
      value = EnsureBuffer(size);
      if (value->IsUndefined()) {
        ::iconv(cd_, NULL, NULL, &outbuf, &outbytesleft);
        return handle_scope.Close(value);
      }
      inbytesleft = in->GetLength();
      inbuf = in->GetBuffer();
      outbytesleft = size;
      outbuf = buffer_;
      converted = ::iconv(cd_, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    } else {
      // Unrecoverable error
      ::iconv(cd_, NULL, NULL, &outbuf, &outbytesleft);
      return handle_scope.Close(v8::ThrowException(
            Module::ErrnoException::New(errno)));
    }
  }
  ::iconv(cd_, NULL, NULL, &outbuf, &outbytesleft);
  value = PruneBuffer();
  if (value->IsUndefined()) {
    return handle_scope.Close(value);
  }
  return handle_scope.Close(Buffer::New(buffer_, outbuf - buffer_));
}

v8::Handle<v8::Value> Iconv::New(const char* to, const char* from) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> argv[2] = {
    v8::String::New(to),
    v8::String::New(from)
  };
  return GetTemplate()->GetFunction()->NewInstance(2, argv);
}

v8::Handle<v8::FunctionTemplate> Iconv::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->SetClassName(v8::String::NewSymbol("Iconv"));
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("to"), ToGet);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("from"),
      FromGet);
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("convert"),
      v8::FunctionTemplate::New(Convert)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Iconv::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  Iconv* self = NULL;
  v8::Handle<v8::Value> from;
  switch (arguments.Length()) {
  case 2:
    if (!arguments[1]->IsString()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be a string"))));
    }
    from = arguments[1]->ToString();
    // Fall through
  case 1:
    if (!arguments[0]->IsString()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be a string"))));
    }
    self = new Iconv;
    if (self) {
      v8::Handle<v8::Value> value;
      if (from.IsEmpty()) {
        value = self->Construct(
            *v8::String::AsciiValue(arguments[0]->ToString()), "UTF-8");
      } else {
        value = self->Construct(
            *v8::String::AsciiValue(arguments[0]->ToString()),
            *v8::String::AsciiValue(from));
      }
      if (value->IsUndefined()) {
        delete self;
        return handle_scope.Close(value);
      }
    }
    break;
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two arguments allowed"))));
  }
  if (!self) {
    delete self;
    return handle_scope.Close(v8::ThrowException(
          Module::ErrnoException::New(ENOMEM)));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(Iconv));
  v8::Persistent<v8::Object> cd =
    v8::Persistent<v8::Object>::New(arguments.This());
  cd->SetInternalField(0, v8::External::New(self));
  cd.MakeWeak(static_cast<void*>(self), Delete);
  return cd;
}

void Iconv::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<Iconv*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(
      -static_cast<int>(sizeof(Iconv)));
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> Iconv::ToGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Iconv* self = static_cast<Iconv*>(
      info.This()->GetPointerFromInternalField(0));
  return handle_scope.Close(v8::String::New(self->to_.c_str()));
}

v8::Handle<v8::Value> Iconv::FromGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Iconv* self = static_cast<Iconv*>(
      info.This()->GetPointerFromInternalField(0));
  return handle_scope.Close(v8::String::New(self->from_.c_str()));
}

v8::Handle<v8::Value> Iconv::Convert(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  switch (arguments.Length()) {
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (Buffer::GetTemplate()->HasInstance(object)) {
        Iconv* self = static_cast<Iconv*>(
            arguments.This()->GetPointerFromInternalField(0));
        Buffer* buffer = static_cast<Buffer*>(
            object->GetPointerFromInternalField(0));
        return self->Convert(buffer);
      }
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument must be of type Buffer"))));
    }
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument must be an object"))));
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument allowed"))));
  }
}

v8::Handle<v8::Value> Iconv::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Iconv* self = static_cast<Iconv*>(
      arguments.This()->GetPointerFromInternalField(0));
  std::string string(*v8::String::Utf8Value(
        arguments.This()->GetConstructorName()));
  string.append("('");
  string.append(self->to_);
  string.append("', '");
  string.append(self->from_);
  string.append("')");
  return handle_scope.Close(v8::String::New(string.c_str()));
}

v8::Handle<v8::Value> Iconv::Construct(const char* to, const char* from) {
  v8::HandleScope handle_scope;
  cd_ = iconv_open(to, from);
  if (cd_ == (iconv_t)-1) {
    if (EINVAL == errno) {
      std::string message("Conversion from ");
      message.append(from);
      message.append(" to ");
      message.append(to);
      message.append(" is not available");
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New(message.c_str()))));
    }
    return handle_scope.Close(v8::ThrowException(
          Module::ErrnoException::New(errno)));
  }
  to_.assign(to);
  from_.assign(from);
  return handle_scope.Close(v8::Boolean::New(true));
}

v8::Handle<v8::Value> Iconv::EnsureBuffer(size_t length) {
  v8::HandleScope handle_scope;
  if (length_ < length) {
    if (length < BUFSIZ) {
      length = BUFSIZ;
    }
    char* buffer = static_cast<char*>(
        ::realloc(static_cast<void*>(buffer_), length));
    if (!buffer) {
      return handle_scope.Close(v8::ThrowException(
            Module::ErrnoException::New(errno)));
    }
    v8::V8::AdjustAmountOfExternalAllocatedMemory(length - length_);
    buffer_ = buffer;
    length_ = length;
  }
  return handle_scope.Close(v8::Boolean::New(true));
}

v8::Handle<v8::Value> Iconv::PruneBuffer() {
  v8::HandleScope handle_scope;
  if (BUFSIZ < length_) {
    char* buffer = static_cast<char*>(
        ::realloc(static_cast<void*>(buffer_), BUFSIZ));
    if (!buffer) {
      return handle_scope.Close(v8::ThrowException(
            Module::ErrnoException::New(errno)));
    }
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-(length_ - BUFSIZ));
    buffer_ = buffer;
    length_ = BUFSIZ;
  }
  return handle_scope.Close(v8::Boolean::New(true));
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
