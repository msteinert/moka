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

#ifndef MOKA_ARRAY_BUFFER_VIEW_H
#define MOKA_ARRAY_BUFFER_VIEW_H

#include "moka/array-buffer.h"
#include <v8.h>

namespace moka {

class ArrayBufferView;

} // namespace moka

class moka::ArrayBufferView {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

  v8::Handle<v8::Object> GetArrayBuffer() const {
    return array_buffer_;
  }

  void* GetBuffer() const {
    return static_cast<moka::ArrayBuffer*>(
        array_buffer_->GetPointerFromInternalField(0))->GetBuffer();
  }

  uint32_t GetByteOffset() const {
    return byte_offset_;
  }

  uint32_t GetByteLength() const {
    return byte_length_;
  }

private: // V8 interface
  static v8::Handle<v8::Value> ArrayBuffer(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> ByteOffset(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

  static v8::Handle<v8::Value> ByteLength(v8::Local<v8::String> property,
      const v8::AccessorInfo &info);

protected: // Protected methods
  ArrayBufferView();

  virtual ~ArrayBufferView();

  v8::Handle<v8::Value> Construct(v8::Handle<v8::Object> array_buffer,
      uint32_t byte_offset, uint32_t byte_length) {
    array_buffer_ = v8::Persistent<v8::Object>::New(array_buffer);
    byte_offset_ = byte_offset;
    byte_length_ = byte_length;
    return v8::True();
  }

private: // Private methods
  ArrayBufferView(ArrayBufferView const& that);

  void operator=(ArrayBufferView const& that);

protected: // Protected data
  v8::Persistent<v8::Object> array_buffer_;
  uint32_t byte_offset_;
  uint32_t byte_length_;
};

#endif // MOKA_ARRAY_BUFFER_VIEW_H

// vim: tabstop=2:sw=2:expandtab
