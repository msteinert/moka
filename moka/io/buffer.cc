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
#include <cstring>
#include "moka/io/buffer.h"
#include <sstream>

namespace moka {

namespace io {

Buffer::Buffer()
  : length_(0)
  , buffer_(NULL) {}

Buffer::~Buffer() {
  if (buffer_) {
    ::free(buffer_);
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-length_);
  }
}

v8::Handle<v8::Value> Buffer::Resize(size_t length) {
  v8::HandleScope handle_scope;
  if (length == length_) {
    return handle_scope.Close(v8::Boolean::New(true));
  }
  if (length) {
    char* buffer = static_cast<char*>(
        ::realloc(static_cast<void*>(buffer_), length));
    if (!buffer) {
      return handle_scope.Close(Module::ErrnoException::New(errno));
    }
    if (length_ > length) {
      ::memset(buffer + length_, 0, length - length_);
    }
    length_ = length;
    buffer_ = buffer;
  } else {
    if (buffer_) {
      ::free(buffer_);
      length_ = 0;
      buffer_ = NULL;
    }
  }
  return handle_scope.Close(v8::Boolean::New(true));
}

v8::Handle<v8::Value> Buffer::New(size_t length) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> argv[1] = { v8::Integer::New(length) };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

v8::Handle<v8::FunctionTemplate> Buffer::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->SetClassName(v8::String::NewSymbol("Buffer"));
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("length"),
            LengthGet);
  templ->InstanceTemplate()->SetIndexedPropertyHandler(GetIndex, SetIndex);
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("resize"),
      v8::FunctionTemplate::New(Resize)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Buffer::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  Buffer* self = NULL;
  switch (arguments.Length()) {
  case 1:
    if (arguments[0]->IsUint32()) {
      self = new Buffer;
      if (self) {
        v8::TryCatch try_catch;
        v8::Handle<v8::Value> value = self->Construct(
            arguments[0]->ToUint32()->Value());
        if (value->IsUndefined()) {
          return handle_scope.Close(v8::ThrowException(try_catch.ReThrow()));
        }
      }
    } else if (arguments[0]->IsArray()) {
      self = new Buffer;
      if (self) {
        v8::Handle<v8::Object> array = arguments[0]->ToObject();
        v8::Handle<v8::Array> properties = array->GetPropertyNames();
        if (!properties.IsEmpty()) {
          size_t length = properties->Length();
          v8::Handle<v8::Value> value = self->Construct(length);
          if (value->IsUndefined()) {
            return handle_scope.Close(v8::ThrowException(value));
          }
          for (size_t index = 0; index < length; ++index) {
            self->SetIndex(index, array->Get(properties->Get(index))->
                ToInteger()->Value() & 0xff);
          }
        }
      }
    } else {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Argument one must be of type Integer or Array")));
    }
    break;
  case 0:
    self = new Buffer;
    break;
  default:
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Zero or one argument(s) allowed")));
  }
  if (!self) {
    return handle_scope.Close(v8::ThrowException(
          Module::ErrnoException::New(ENOMEM)));
  }
  v8::Persistent<v8::Object> buffer =
    v8::Persistent<v8::Object>::New(arguments.This());
  buffer->SetInternalField(0, v8::External::New(self));
  buffer.MakeWeak(static_cast<void*>(self), Delete);
  return buffer;
}

void Buffer::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<Buffer*>(parameters);
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> Buffer::LengthGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Buffer* self = static_cast<Buffer*>(
      info.This()->GetPointerFromInternalField(0));
  return handle_scope.Close(v8::Uint32::New(self->length_));
}

v8::Handle<v8::Value> Buffer::GetIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Buffer* self = static_cast<Buffer*>(
      info.This()->GetPointerFromInternalField(0));
  if (index >= self->GetLength()) {
    return handle_scope.Close(v8::ThrowException(v8::Exception::RangeError(
            v8::String::New("Index is out of range"))));
  }
  return handle_scope.Close(v8::Uint32::New(self->GetIndex(index)));
}

v8::Handle<v8::Value> Buffer::SetIndex(uint32_t index,
    v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Buffer* self = static_cast<Buffer*>(
      info.This()->GetPointerFromInternalField(0));
  if (index < self->GetLength()) {
    self->SetIndex(index, value->ToUint32()->Value() & 0xff);
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::RangeError(
            v8::String::New("Index is out of range"))));
  }
  return handle_scope.Close(value);
}

v8::Handle<v8::Value> Buffer::Resize(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Buffer* self = static_cast<Buffer*>(
      arguments.This()->GetPointerFromInternalField(0));
  switch (arguments.Length()) {
  case 1:
    if (!arguments[0]->IsUint32()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument must be an unsigned integer"))));
    }
    return self->Resize(arguments[0]->ToUint32()->Value());
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument allowed"))));
  }
}

v8::Handle<v8::Value> Buffer::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Buffer* self = static_cast<Buffer*>(
      arguments.This()->GetPointerFromInternalField(0));
  std::stringstream stream;
  stream << *v8::String::Utf8Value(arguments.This()->GetConstructorName());
  stream << "([";
  for (size_t index = 0; index < self->GetLength(); ++index) {
    if (index) {
      stream << ", ";
    }
    stream << static_cast<uint32_t>(self->GetIndex(index));
  }
  stream << "])";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

v8::Handle<v8::Value> Buffer::Construct(size_t length) {
  v8::HandleScope handle_scope;
  if (!length) {
    return handle_scope.Close(v8::Boolean::New(true));
  }
  buffer_ = static_cast<char*>(::calloc(1, length));
  if (!buffer_) {
    return handle_scope.Close(v8::ThrowException(
          Module::ErrnoException::New(errno)));
  }
  length_ = length;
  v8::V8::AdjustAmountOfExternalAllocatedMemory(length_);
  return handle_scope.Close(v8::Boolean::New(true));
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
