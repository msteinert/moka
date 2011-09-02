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

#ifndef MOKA_INT8_ARRAY_H
#define MOKA_INT8_ARRAY_H

#include "moka/array-buffer-view.h"
#include <v8.h>

namespace moka {

class ArrayBuffer;
class Int8Array;

} // namespace moka

class moka::Int8Array: public moka::ArrayBufferView {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

private: // V8 interface
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

protected: // Protected methods
  Int8Array();

  virtual ~Int8Array();

private: // Private methods
  Int8Array(Int8Array const& that);

  void operator=(Int8Array const& that);

  virtual uint32_t BytesPerElement() const {
    return 1;
  }

  virtual void Set(uint32_t index, v8::Handle<v8::Value> value) {
    array_[index] = static_cast<int8_t>(value->ToInt32()->Value());
  }

  virtual v8::Handle<v8::Value> Get(uint32_t index) const {
    return v8::Int32::New(array_[index]);
  }

  virtual void Set(ArrayBufferView* that, uint32_t offset);

  virtual v8::Handle<v8::Value> SubArray(int32_t begin, int32_t end) const;

protected: // Protected data
  int8_t* array_;
};

#endif // MOKA_INT8_ARRAY_H

// vim: tabstop=2:sw=2:expandtab
