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

#ifndef MOKA_TYPED_ARRAY_H
#define MOKA_TYPED_ARRAY_H

#include "moka/array-buffer-view.h"
#include "moka/module.h"
#include <v8.h>

namespace moka {

template <typename T, v8::ExternalArrayType A> class TypedArray;

} // namespace moka

template <typename T, v8::ExternalArrayType A>
class moka::TypedArray: public moka::ArrayBufferView {
public:
  TypedArray() {};

  virtual ~TypedArray() {};

  static v8::Handle<v8::FunctionTemplate> GetTemplate(const char* name = NULL) {
    static v8::Persistent<v8::FunctionTemplate> templ_;
    if (!templ_.IsEmpty()) {
      return templ_;
    }
    v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
    templ->SetClassName(v8::String::NewSymbol(name));
    templ->Inherit(ArrayBufferView::GetTemplate());
    templ->InstanceTemplate()->SetInternalFieldCount(1);
    templ->Set(v8::String::NewSymbol("BYTES_PER_ELEMENT"),
        v8::Uint32::New(sizeof(T)),
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
    templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
    return templ_;
  }

private: // V8 interface
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments) {
    if (!arguments.IsConstructCall()) {
      return Module::ConstructCall(GetTemplate(), arguments);
    }
    TypedArray* self = new TypedArray;
    if (!self) {
      return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
    }
    v8::Handle<v8::Value> value = self->Construct(arguments, A);
    if (value->IsUndefined()) {
      delete self;
      return value;
    }
    v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(TypedArray));
    v8::Persistent<v8::Object> typed_array =
      v8::Persistent<v8::Object>::New(arguments.This());
    typed_array->SetInternalField(0, v8::External::New(self));
    typed_array.MakeWeak(static_cast<void*>(self), Delete);
    return typed_array;
  }

  static void Delete(v8::Persistent<v8::Value> object, void* parameters) {
    delete static_cast<TypedArray*>(parameters);
    v8::V8::AdjustAmountOfExternalAllocatedMemory(-sizeof(TypedArray));
    object.Dispose();
    object.Clear();
  }

private: // Private methods
  TypedArray(TypedArray const& that);

  void operator=(TypedArray const& that);

  virtual uint32_t BytesPerElement() const {
    return sizeof(T);
  }

  virtual v8::Handle<v8::Value> New(v8::Handle<v8::Value> buffer,
      uint32_t byte_offset, uint32_t length) const {
    v8::Handle<v8::Value> argv[3] = {
      buffer,
      v8::Uint32::New(byte_offset),
      v8::Uint32::New(length)
    };
    return GetTemplate()->GetFunction()->NewInstance(3, argv);
  }
};

#endif // MOKA_TYPED_ARRAY_H

// vim: tabstop=2:sw=2:expandtab
