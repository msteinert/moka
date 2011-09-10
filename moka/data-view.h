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

#ifndef MOKA_DATA_VIEW_H
#define MOKA_DATA_VIEW_H

#include "moka/array-buffer-view.h"

namespace moka {

class DataView;

} // namespace moka

class moka::DataView: public moka::ArrayBufferView {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

private: // V8 interface
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

  static v8::Handle<v8::Value> GetInt8(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetUint8(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetInt16(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetUint16(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetInt32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetUint32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetFloat32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> GetDouble64(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetInt8(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetUint8(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetInt16(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetUint16(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetInt32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetUint32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetFloat32(const v8::Arguments& arguments);

  static v8::Handle<v8::Value> SetDouble64(const v8::Arguments& arguments);

private: // Private methods
  DataView() {}

  virtual ~DataView() {}
};

#endif // MOKA_DATA_VIEW_H

// vim: tabstop=2:sw=2:expandtab
