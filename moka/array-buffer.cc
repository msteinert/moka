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
#include "moka/array-buffer.h"
#include "moka/module.h"

namespace moka {

ArrayBuffer::ArrayBuffer()
  : buffer_(NULL)
  , byte_length_(0) {}

ArrayBuffer::~ArrayBuffer() {
  if (buffer_) {
    ::free(buffer_);
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-byte_length_);
  }
}

// Public interface
v8::Handle<v8::FunctionTemplate> ArrayBuffer::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->SetClassName(v8::String::NewSymbol("ArrayBuffer"));
  // Functions
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("slice"),
      v8::FunctionTemplate::New(Slice)->GetFunction());
  // Properties
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("byteLength"),
      ByteLength);
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> ArrayBuffer::New(uint32_t length) {
  v8::Handle<v8::Value> argv[1] = { v8::Uint32::New(length) };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

// Private V8 interface
v8::Handle<v8::Value> ArrayBuffer::New(const v8::Arguments& arguments) {
  ArrayBuffer* self = NULL;
  switch (arguments.Length()) {
  case 1:
    if (arguments[0]->IsUint32()) {
      self = new ArrayBuffer;
      if (self) {
        self->byte_length_ = arguments[0]->ToUint32()->Value();
        if (self->byte_length_) {
          self->buffer_ = ::calloc(self->byte_length_, sizeof(char));
          if (!self->buffer_) {
            delete self;
            return v8::ThrowException(Module::ErrnoException::New(errno));
          }
        }
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned integer")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("A single argument is required")));
  }
  if (!self) {
    return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(ArrayBuffer));
  v8::Persistent<v8::Object> array_buffer =
    v8::Persistent<v8::Object>::New(arguments.This());
  array_buffer->SetInternalField(0, v8::External::New(self));
  array_buffer.MakeWeak(static_cast<void*>(self), Delete);
  return array_buffer;
}

void ArrayBuffer::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<ArrayBuffer*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(ArrayBuffer));
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> ArrayBuffer::Slice(const v8::Arguments& arguments) {
  ArrayBuffer* self = static_cast<ArrayBuffer*>(
      arguments.This()->GetPointerFromInternalField(0));
  int32_t begin = 0, end = self->byte_length_;
  switch (arguments.Length()) {
    case 2:
      if (arguments[1]->IsInt32()) {
        int32_t index = arguments[1]->ToInt32()->Value();
        if (index < end) {
          if (index > 0) {
            end = index;
          } else {
            end = 0;
          }
        }
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an integer")));
      }
      // Fall through
    case 1:
      if (arguments[0]->IsInt32()) {
        int32_t index = arguments[0]->ToInt32()->Value();
        if (index > 0) {
          if (index < end) {
            begin = index;
          } else {
            begin = end;
          }
        }
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an integer")));
      }
      break;
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One or two arguments allowed")));
  }
  uint32_t length;
  if (end > begin) {
    length = end - begin;
  } else {
    length = 0;
  }
  v8::TryCatch try_catch;
  v8::Handle<v8::Value> byte_array = New(length);
  if (byte_array.IsEmpty()) {
    return try_catch.ReThrow();
  }
  if (byte_array->IsUndefined()) {
    return byte_array;
  }
  if (length) {
    ArrayBuffer* that = static_cast<ArrayBuffer*>(
        byte_array->ToObject()->GetPointerFromInternalField(0));
    for (uint32_t index = 0; index < length; ++index) {
      static_cast<char*>(that->buffer_)[index] =
        static_cast<char*>(self->buffer_)[index + begin];
    }
  }
  return byte_array;
}

v8::Handle<v8::Value> ArrayBuffer::ByteLength(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  return v8::Uint32::New(static_cast<ArrayBuffer*>(
        info.This()->GetPointerFromInternalField(0))->byte_length_);
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
