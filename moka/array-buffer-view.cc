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
  // Methods
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("get"),
      v8::FunctionTemplate::New(Get)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("set"),
      v8::FunctionTemplate::New(Set)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("subarray"),
      v8::FunctionTemplate::New(SubArray)->GetFunction());
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

v8::Handle<v8::Value> ArrayBufferView::Get(const v8::Arguments& arguments) {
  switch (arguments.Length()) {
  case 1:
    if (arguments[0]->IsUint32()) {
      ArrayBufferView* self = static_cast<ArrayBufferView*>(
          arguments.This()->GetPointerFromInternalField(0));
      uint32_t index = arguments[0]->ToUint32()->Value();
      if (index < self->Length()) {
        return arguments.This()->Get(index);
      }
      return v8::Undefined();
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One argument is required")));
  }
}

v8::Handle<v8::Value> ArrayBufferView::Set(const v8::Arguments& arguments) {
  ArrayBufferView* self = static_cast<ArrayBufferView*>(
      arguments.This()->GetPointerFromInternalField(0));
  uint32_t offset = 0;
  switch (arguments.Length()) {
  case 2:
    if (arguments[0]->IsUint32()) {
      uint32_t index = arguments[0]->ToUint32()->Value();
      if (index < self->Length()) {
        arguments.This()->Set(index, arguments[1]);
      }
      return v8::Undefined();
    } else {
      if (arguments[1]->IsUint32()) {
        offset = arguments[1]->ToUint32()->Value();
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an unsigned long")));
      }
    }
    // Fall through
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (object->IsArray()) {
        v8::Array* array = v8::Array::Cast(*arguments[0]);
        if (offset + array->Length() > self->Length()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Offset is out of range")));
        }
        for (uint32_t index = 0; index < self->Length(); ++index) {
          arguments.This()->Set(index + offset, array->Get(index));
        }
      } else if (GetTemplate()->HasInstance(object)) {
        ArrayBufferView* that = static_cast<ArrayBufferView*>(
            object->GetPointerFromInternalField(0));
        if (offset + that->Length() > self->Length()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Offset is out of range")));
        }
        for (uint32_t index = 0; index < self->Length(); ++index) {
          arguments.This()->Set(index + offset, object->Get(index));
        }
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an array")));
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an object")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One argument is required")));
  }
  return v8::Undefined();
}

v8::Handle<v8::Value> ArrayBufferView::SubArray(
    const v8::Arguments& arguments) {
  ArrayBufferView* self = static_cast<ArrayBufferView*>(
      arguments.This()->GetPointerFromInternalField(0));
  int32_t end = self->GetByteLength();
  switch (arguments.Length()) {
  case 2:
    if (arguments[1]->IsInt32()) {
      end = arguments[1]->ToUint32()->Value();
      if (0 > end) {
        end = self->Length() + end;
        if (0 > end) {
          end = 0;
        }
      } else {
        if (static_cast<uint32_t>(end) > self->Length()) {
          end = self->Length() + 1;
        }
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument two must be a long")));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsInt32()) {
      int32_t start =
        arguments[0]->ToInt32()->Value();
      if (0 > start) {
        start = self->Length() + start;
        if (0 > start) {
          start = 0;
        }
      } else {
        if (static_cast<uint32_t>(start) > self->Length()) {
          start = self->Length() + 1;
        }
      }
      int32_t length = end - start - 1;
      if (0 > length) {
        length = 0;
      }
      v8::TryCatch try_catch;
      v8::Handle<v8::Value> argv[3] = {
        self->GetArrayBuffer(),
        v8::Uint32::New(start * self->BytesPerElement()),
        v8::Uint32::New(length)
      };
      v8::Handle<v8::Value> value = self->NewInstance(3, argv);
      if (value.IsEmpty()) {
        return try_catch.ReThrow();
      }
      return value;
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be a long")));
    }
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One or two arguments are required")));
  }
}

// Protected
v8::Handle<v8::Value> ArrayBufferView::Construct(
    const v8::Arguments& arguments, v8::ExternalArrayType type) {
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
    if (arguments[0]->IsObject()) {
      if (!ArrayBuffer::GetTemplate()->HasInstance(arguments[0]->ToObject())) {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an ArrayBuffer")));
      }
    } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an object")));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsUint32()) {
      // TypedArray(unsigned long length)
      byte_length_ = arguments[0]->ToUint32()->Value() * BytesPerElement();
      v8::Handle<v8::Value> array_buffer = ArrayBuffer::New(byte_length_);
      if (array_buffer->IsUndefined()) {
        return array_buffer;
      }
      array_buffer_ = v8::Persistent<v8::Object>::New(array_buffer->ToObject());
      byte_offset_ = 0;
      arguments.This()->SetIndexedPropertiesToExternalArrayData(GetBuffer(),
          type, Length());
    } else if (arguments[0]->IsArray()) {
      // TypedArray(type[] array)
      v8::Array* array = v8::Array::Cast(*arguments[0]);
      byte_length_ = array->Length() * BytesPerElement();
      v8::Handle<v8::Value> array_buffer = ArrayBuffer::New(byte_length_);
      if (array_buffer->IsUndefined()) {
        return array_buffer;
      }
      array_buffer_ = v8::Persistent<v8::Object>::New(array_buffer->ToObject());
      byte_offset_ = 0;
      arguments.This()->SetIndexedPropertiesToExternalArrayData(GetBuffer(),
          type, Length());
      for (uint32_t index = 0; index < array->Length(); ++index) {
        arguments.This()->Set(index, array->Get(index));
      }
    } else if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (GetTemplate()->HasInstance(object)) {
        // TypedArray(TypedArray array)
        ArrayBufferView* that = static_cast<ArrayBufferView*>(
            object->GetPointerFromInternalField(0));
        v8::Handle<v8::Value> array_buffer =
          ArrayBuffer::New(that->GetByteLength());
        if (array_buffer->IsUndefined()) {
          return array_buffer;
        }
        array_buffer_ =
          v8::Persistent<v8::Object>::New(array_buffer->ToObject());
        byte_offset_ = 0;
        byte_length_ = that->GetByteLength();
        ::memcpy(GetBuffer(), that->GetBuffer(), that->GetByteLength());
        arguments.This()->SetIndexedPropertiesToExternalArrayData(GetBuffer(),
            type, Length());
      } else if (ArrayBuffer::GetTemplate()->HasInstance(object)) {
        // TypedArray(ArrayBuffer buffer,
        //            optional unsigned long byteOffset,
        //            option unsigned long length)
        array_buffer_ = v8::Persistent<v8::Object>::New(object);
        byte_offset_ = byte_offset;
        moka::ArrayBuffer* buffer = static_cast<moka::ArrayBuffer*>(
            object->GetPointerFromInternalField(0));
        if (arguments.Length() == 3) {
          byte_length_ = length * BytesPerElement();
        } else {
          byte_length_ = buffer->GetByteLength() - byte_offset_;
        }
        if (byte_offset_ >= buffer->GetByteLength()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Offset is out of range")));
        }
        if (byte_offset_ + byte_length_ > buffer->GetByteLength()) {
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New("Length is out of range")));
        }
        if (byte_length_ % BytesPerElement()) {
          std::stringstream message;
          message << "Length minus offset must be a multiple of ";
          message << BytesPerElement();
          return v8::ThrowException(v8::Exception::RangeError(
                v8::String::New(message.str().c_str())));
        }
        arguments.This()->SetIndexedPropertiesToExternalArrayData(
            static_cast<int8_t*>(GetBuffer()) + byte_offset_, type, Length());
      } else {
        return v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an ArrayBuffer"
                " or an ArrayBufferView")));
      }
    } else {
      return v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be an unsigned long,"
              " and array or an object")));
    }
    break;
  default:
    return v8::ThrowException(v8::Exception::TypeError(
          v8::String::New("One, two or three arguments required")));
  }
  return v8::True();
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
