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
#include "moka/array-buffer.h"
#include "moka/int8-array.h"
#include "moka/module.h"

namespace moka {

Int8Array::Int8Array()
  : array_(NULL) {}

Int8Array::~Int8Array() {
  if (!array_buffer_.IsEmpty()) {
    array_buffer_.Dispose();
  }
}

// Public interface
v8::Handle<v8::FunctionTemplate> Int8Array::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->Inherit(ArrayBufferView::GetTemplate());
  templ->SetClassName(v8::String::NewSymbol("Int8Array"));
  return templ_;
}

// Private V8 interface
v8::Handle<v8::Value> Int8Array::New(const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  Int8Array* self = NULL;
  uint32_t byte_offset = 0, length = 0;
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->IsUint32()) {
      length = arguments[2]->ToUint32()->Value();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument three must be an unsigned long")));
    }
    // Fall through
  case 2:
    if (arguments[1]->IsUint32()) {
      byte_offset = arguments[1]->ToUint32()->Value();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument two must be an unsigned long")));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsUint32()) {
      uint32_t length = arguments[0]->ToUint32()->Value();
      v8::Handle<v8::Value> buffer = ArrayBuffer::New(length);
      if (buffer->IsUndefined()) {
        return buffer;
      }
      self = new Int8Array;
      if (self) {
        self->Construct(buffer->ToObject(), 0, length);
      }
    } else if (arguments[0]->IsObject()) {
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long"
              " or an object")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two or three arguments required")));
  }
  if (!self) {
    return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(Int8Array));
  v8::Persistent<v8::Object> int8_array =
    v8::Persistent<v8::Object>::New(arguments.This());
  int8_array->SetInternalField(0, v8::External::New(self));
  int8_array.MakeWeak(static_cast<void*>(self), Delete);
  return int8_array;
}

void Int8Array::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<Int8Array*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(Int8Array));
  object.Dispose();
  object.Clear();
}

// Private methods
void Int8Array::Set(ArrayBufferView* that, uint32_t offset) {
}

v8::Handle<v8::Value> Int8Array::SubArray(int32_t begin, int32_t end) const {
  return v8::Undefined();
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
