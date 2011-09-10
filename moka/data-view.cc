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

#include <arpa/inet.h>
#include <cerrno>
#include "moka/data-view.h"
#include "moka/module.h"

namespace moka {

// Public interface
v8::Handle<v8::FunctionTemplate> DataView::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("DataView"));
  templ->Inherit(ArrayBufferView::GetTemplate());
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  // Methods
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getInt8"),
      v8::FunctionTemplate::New(GetInt8)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getUint8"),
      v8::FunctionTemplate::New(GetUint8)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getInt16"),
      v8::FunctionTemplate::New(GetInt16)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getUint16"),
      v8::FunctionTemplate::New(GetUint16)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getInt32"),
      v8::FunctionTemplate::New(GetInt32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getUint32"),
      v8::FunctionTemplate::New(GetUint32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getFloat32"),
      v8::FunctionTemplate::New(GetFloat32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("getDouble64"),
      v8::FunctionTemplate::New(GetDouble64)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setInt8"),
      v8::FunctionTemplate::New(SetInt8)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setUint8"),
      v8::FunctionTemplate::New(SetUint8)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setInt16"),
      v8::FunctionTemplate::New(SetInt16)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setUint16"),
      v8::FunctionTemplate::New(SetUint16)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setInt32"),
      v8::FunctionTemplate::New(SetInt32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setUint32"),
      v8::FunctionTemplate::New(SetUint32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setFloat32"),
      v8::FunctionTemplate::New(SetFloat32)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("setDouble64"),
      v8::FunctionTemplate::New(SetDouble64)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> DataView::New(const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  DataView* self = NULL;
  uint32_t byte_offset = 0, byte_length = 0;
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->IsUint32()) {
      byte_length = arguments[2]->ToUint32()->Value();
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
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (ArrayBuffer::GetTemplate()->HasInstance(object)) {
        moka::ArrayBuffer* buffer = static_cast<moka::ArrayBuffer*>(
            object->GetPointerFromInternalField(0));
        if (arguments.Length() < 3) {
          byte_length = buffer->GetByteLength() - byte_offset;
        } else {
          if (byte_length + byte_offset > buffer->GetByteLength()) {
            return v8::ThrowException(v8::Exception::RangeError(
                  v8::String::New("Byte length is out of range")));
          }
        }
        self = new DataView;
        if (self) {
          v8::Handle<v8::Value> value =
            self->Construct(object, byte_offset, byte_length);
          if (value->IsUndefined()) {
            return value;
          }
        }
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an ArrayBuffer")));
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an object")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two or three arguments required")));
  }
  if (!self) {
    return v8::ThrowException(Module::ErrnoException::New(ENOMEM));
  }
  v8::V8::AdjustAmountOfExternalAllocatedMemory(sizeof(DataView));
  v8::Persistent<v8::Object> data_view =
    v8::Persistent<v8::Object>::New(arguments.This());
  data_view->SetInternalField(0, v8::External::New(self));
  data_view.MakeWeak(static_cast<void*>(self), Delete);
  return data_view;
}

void DataView::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<DataView*>(parameters);
  v8::V8::AdjustAmountOfExternalAllocatedMemory(
      static_cast<int>(-sizeof(DataView)));
  object.Dispose();
  object.Clear();
} 

v8::Handle<v8::Value> DataView::GetInt8(const v8::Arguments& arguments) {
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
      return v8::Int32::New(
          static_cast<int8_t*>(self->GetBuffer())[byte_offset]);
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One argument required")));
  }
}

v8::Handle<v8::Value> DataView::GetUint8(const v8::Arguments& arguments) {
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
      return v8::Uint32::New(
          static_cast<uint8_t*>(self->GetBuffer())[byte_offset]);
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One argument required")));
  }
}

v8::Handle<v8::Value> DataView::GetInt16(const v8::Arguments& arguments) {
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
      if (byte_offset + sizeof(int16_t) > self->GetByteLength()) {
        return v8::ThrowException(v8::Exception::RangeError(
              v8::String::New("Attempt to read beyond the end of the view")));
      }
      int16_t value;
      int8_t* buffer = static_cast<int8_t*>(self->GetBuffer()) + byte_offset;
      if (little_endian) {
        value = MOKA_INT16_LITTLE_ENDIAN(buffer);
      } else {
        value = MOKA_INT16_BIG_ENDIAN(buffer);
      }
      return v8::Int32::New(value);
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One argument required")));
  }
}

v8::Handle<v8::Value> DataView::GetUint16(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::GetInt32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::GetUint32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::GetFloat32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::GetDouble64(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetInt8(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetUint8(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetInt16(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetUint16(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetInt32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetUint32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetFloat32(const v8::Arguments& arguments) {
  return v8::Undefined();
}

v8::Handle<v8::Value> DataView::SetDouble64(const v8::Arguments& arguments) {
  return v8::Undefined();
}

// Private V8 interface

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
