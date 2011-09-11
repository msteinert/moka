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

#include <cstring>
#include "moka/array-buffer-view.h"
#include "moka/module.h"
#include <sstream>

namespace moka {

ArrayBufferView::ArrayBufferView()
  : byte_offset_(0)
  , byte_length_(0) {}

ArrayBufferView::~ArrayBufferView() {
  if (!array_buffer_.IsEmpty()) {
    array_buffer_.Dispose();
  }
}

// Public interface
v8::Handle<v8::FunctionTemplate> ArrayBufferView::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New();
  templ->SetClassName(v8::String::NewSymbol("ArrayBufferView"));
  // Properties
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("arrayBuffer"),
      ArrayBuffer);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("byteOffset"),
      ByteOffset);
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("byteLength"),
      ByteLength);
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

// Private V8 interface
v8::Handle<v8::Value> ArrayBufferView::ArrayBuffer(
    v8::Local<v8::String> property, const v8::AccessorInfo &info) {
  ArrayBufferView* self = static_cast<ArrayBufferView*>(
      info.This()->GetPointerFromInternalField(0));
  return self->array_buffer_;
}

v8::Handle<v8::Value> ArrayBufferView::ByteOffset(
    v8::Local<v8::String> property, const v8::AccessorInfo &info) {
  ArrayBufferView* self = static_cast<ArrayBufferView*>(
      info.This()->GetPointerFromInternalField(0));
  return v8::Uint32::New(self->byte_offset_);
}

v8::Handle<v8::Value> ArrayBufferView::ByteLength(
    v8::Local<v8::String> property, const v8::AccessorInfo &info) {
  ArrayBufferView* self = static_cast<ArrayBufferView*>(
      info.This()->GetPointerFromInternalField(0));
  return v8::Uint32::New(self->byte_length_);
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
