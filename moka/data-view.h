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
#include "moka/bytes.h"

namespace moka {

class DataView;

} // namespace moka

class moka::DataView: public moka::ArrayBufferView {
public:
  static v8::Handle<v8::FunctionTemplate> GetTemplate();

private: // V8 interface
  static v8::Handle<v8::Value> New(const v8::Arguments& arguments);

  static void Delete(v8::Persistent<v8::Value> object, void* parameters);

  template<typename T>
  static v8::Handle<v8::Value> GetByte(const v8::Arguments& arguments) {
    switch (arguments.Length()) {
    case 1:
      if (arguments[0]->IsUint32()) {
        uint32_t byte_offset = arguments[0]->ToUint32()->Value();
        ArrayBufferView* self = static_cast<ArrayBufferView*>(
            arguments.This()->GetPointerFromInternalField(0));
        if (byte_offset >= self->GetByteLength()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Attempt to read beyond the end of the view")));
        }
        return v8::Number::New(
            static_cast<T*>(self->GetBuffer())[byte_offset]);
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned long")));
      }
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument required")));
    }
  }

  template<typename T>
  static v8::Handle<v8::Value> Get(const v8::Arguments& arguments) {
    bool little_endian = false;
    switch (arguments.Length()) {
    case 2:
      little_endian = arguments[1]->ToBoolean()->Value();
      // Fall through
    case 1:
      if (arguments[0]->IsUint32()) {
        uint32_t byte_offset = arguments[0]->ToUint32()->Value();
        ArrayBufferView* self = static_cast<ArrayBufferView*>(
            arguments.This()->GetPointerFromInternalField(0));
        if (byte_offset + sizeof(T) > self->GetByteLength()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Attempt to read beyond the end of the view")));
        }
        T value;
        int8_t* buffer = static_cast<int8_t*>(self->GetBuffer()) + byte_offset;
        if (little_endian) {
          value = moka::bytes::Get<T, LITTLE_ENDIAN>()(buffer);
        } else {
          value = moka::bytes::Get<T, BIG_ENDIAN>()(buffer);
        }
        return v8::Number::New(value);
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned long")));
      }
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument required")));
    }
  }

  template<typename T>
  static v8::Handle<v8::Value> SetByte(const v8::Arguments& arguments) {
    if (2 == arguments.Length()) {
      if (!arguments[1]->IsInt32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be a byte")));
      }
      if (!arguments[0]->IsUint32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned long")));
      }
      uint32_t byte_offset = arguments[0]->ToUint32()->Value();
      ArrayBufferView* self = static_cast<ArrayBufferView*>(
          arguments.This()->GetPointerFromInternalField(0));
      if (byte_offset >= self->GetByteLength()) {
        return v8::ThrowException(v8::Exception::RangeError(
              v8::String::New("Attempt to write beyond the end of the view")));
      }
      static_cast<T*>(self->GetBuffer())[byte_offset] =
        static_cast<T>(arguments[1]->ToInt32()->Value());
      return v8::Null();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two arguments required")));
    }
  }

  template<typename T>
  static v8::Handle<v8::Value> SetSigned(const v8::Arguments& arguments) {
    int8_t* buffer;
    uint32_t byte_offset;
    bool little_endian = false;
    ArrayBufferView* self = static_cast<ArrayBufferView*>(
        arguments.This()->GetPointerFromInternalField(0));
    switch (arguments.Length()) {
    case 3:
      little_endian = arguments[2]->ToBoolean()->Value();
      // Fall Through
    case 2:
      if (!arguments[1]->IsInt32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an integer")));
      }
      if (!arguments[0]->IsUint32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned long")));
      }
      byte_offset = arguments[0]->ToUint32()->Value();
      if (byte_offset + sizeof(T) >= self->GetByteLength()) {
        return v8::ThrowException(v8::Exception::RangeError(
              v8::String::New("Attempt to write beyond the end of the view")));
      }
      buffer = static_cast<int8_t*>(self->GetBuffer()) + byte_offset;
      if (little_endian) {
        moka::bytes::Set<T, LITTLE_ENDIAN>()(
            arguments[1]->ToInt32()->Value(), buffer);
      } else {
        moka::bytes::Set<T, BIG_ENDIAN>()(
            arguments[1]->ToInt32()->Value(), buffer);
      }
      return v8::Null();
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments required")));
    }
  }

  template<typename T>
  static v8::Handle<v8::Value> SetUnsigned(const v8::Arguments& arguments) {
    int8_t* buffer;
    uint32_t byte_offset;
    bool little_endian = false;
    ArrayBufferView* self = static_cast<ArrayBufferView*>(
        arguments.This()->GetPointerFromInternalField(0));
    switch (arguments.Length()) {
    case 3:
      little_endian = arguments[2]->ToBoolean()->Value();
      // Fall Through
    case 2:
      if (!arguments[1]->IsUint32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an unsigned integer")));
      }
      if (!arguments[0]->IsUint32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned long")));
      }
      byte_offset = arguments[0]->ToUint32()->Value();
      if (byte_offset + sizeof(T) >= self->GetByteLength()) {
        return v8::ThrowException(v8::Exception::RangeError(
              v8::String::New("Attempt to write beyond the end of the view")));
      }
      buffer = static_cast<int8_t*>(self->GetBuffer()) + byte_offset;
      if (little_endian) {
        moka::bytes::Set<T, LITTLE_ENDIAN>()(
            arguments[1]->ToUint32()->Value(), buffer);
      } else {
        moka::bytes::Set<T, BIG_ENDIAN>()(
            arguments[1]->ToUint32()->Value(), buffer);
      }
      return v8::Null();
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments required")));
    }
  }

  template<typename T>
  static v8::Handle<v8::Value> SetRational(const v8::Arguments& arguments) {
    int8_t* buffer;
    uint32_t byte_offset;
    bool little_endian = false;
    ArrayBufferView* self = static_cast<ArrayBufferView*>(
        arguments.This()->GetPointerFromInternalField(0));
    switch (arguments.Length()) {
    case 3:
      little_endian = arguments[2]->ToBoolean()->Value();
      // Fall Through
    case 2:
      if (!arguments[1]->IsUint32()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an unsigned integer")));
      }
      if (!arguments[0]->IsNumber()) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be a rational number")));
      }
      byte_offset = arguments[0]->ToUint32()->Value();
      if (byte_offset + sizeof(T) >= self->GetByteLength()) {
        return v8::ThrowException(v8::Exception::RangeError(
              v8::String::New("Attempt to write beyond the end of the view")));
      }
      buffer = static_cast<int8_t*>(self->GetBuffer()) + byte_offset;
      if (little_endian) {
        moka::bytes::Set<T, LITTLE_ENDIAN>()(
            arguments[1]->ToNumber()->Value(), buffer);
      } else {
        moka::bytes::Set<T, BIG_ENDIAN>()(
            arguments[1]->ToNumber()->Value(), buffer);
      }
      return v8::Null();
    default:
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments required")));
    }
  }

private: // Private methods
  DataView() {}

  virtual ~DataView() {}
};

#endif // MOKA_DATA_VIEW_H

// vim: tabstop=2:sw=2:expandtab
