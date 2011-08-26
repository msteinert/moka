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

#include "v8-commonjs/binary/b/byte-array.h"

namespace commonjs {

v8::Handle<v8::FunctionTemplate> ByteArray::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->SetClassName(v8::String::NewSymbol("ByteArray"));
  templ->Inherit(Binary::GetTemplate());
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("length"),
      Binary::LengthGet, LengthSet);
  templ->InstanceTemplate()->SetIndexedPropertyHandler(GetIndex, SetIndex,
      QueryIndex);
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("copy"),
      v8::FunctionTemplate::New(Copy)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> ByteArray::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    int argc = arguments.Length();
    v8::Local<v8::Value>* argv = new v8::Local<v8::Value>[argc];
    for (int i = 0; i < argc; ++i) {
      argv[i] = arguments[i];
    }
    v8::Local<v8::Object> instance =
      GetTemplate()->GetFunction()->NewInstance(argc, argv);
    delete[] argv;
    return handle_scope.Close(instance);
  }
  ByteArray* self = NULL;
  if (arguments.Length() == 0) {
    self = new ByteArray;
  } else if (arguments.Length() == 1) {
    if (arguments[0]->IsUint32()) {
      self = new ByteArray;
      if (self) {
        v8::Handle<v8::Value> exception =
          self->Construct(arguments[0]->ToUint32());
      }
    } else if (arguments[0]->IsObject()) {
      if (arguments[0]->IsArray()) {
        self = new ByteArray;
        if (self) {
          v8::Handle<v8::Value> exception = 
            self->Construct(v8::Handle<v8::Array>::Cast(arguments[0]));
          if (!exception.IsEmpty()) {
            delete self;
            return handle_scope.Close(v8::ThrowException(exception));
          }
        }
      } else {
        self = new ByteArray;
        if (self) {
          v8::Handle<v8::Value> exception = 
            self->Construct(v8::Handle<v8::Object>::Cast(arguments[0]));
          if (!exception.IsEmpty()) {
            delete self;
            return handle_scope.Close(v8::ThrowException(exception));
          }
        }
      }
    }
  } else if (arguments.Length() == 2) {
    if (arguments[0]->IsString() && arguments[1]->IsString()) {
      self = new ByteArray;
      if (self) {
        v8::Handle<v8::Value> exception = 
          self->Construct(arguments[0]->ToString(), arguments[1]->ToString());
        if (!exception.IsEmpty()) {
          delete self;
          return handle_scope.Close(v8::ThrowException(exception));
        }
      }
    } else {
      return handle_scope.Close(v8::ThrowException(v8::String::New(
              "Argument one and two must be of type String")));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Zero, one, or two arguments allowed")));
  }
  if (!self) {
    return handle_scope.Close(v8::ThrowException(v8::String::New("No memory")));
  }
  v8::Persistent<v8::Object> byte_string =
    v8::Persistent<v8::Object>::New(arguments.This());
  byte_string->SetInternalField(0, v8::External::New(self));
  byte_string.MakeWeak(static_cast<void*>(self), Delete);
  return byte_string;
}

void ByteArray::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<ByteArray*>(parameters);
  object.Dispose();
  object.Clear();
}

void ByteArray::LengthSet(v8::Local<v8::String> property,
    v8::Local<v8::Value> value, const v8::AccessorInfo& info) {
  v8::HandleScope handle_scope;
  if (value->Uint32Value()) {
    ByteArray* self = static_cast<ByteArray*>(
        info.This()->GetPointerFromInternalField(0));
    self->Resize(value->ToUint32()->Value());
  }
}

v8::Handle<v8::Value> ByteArray::GetIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  ByteArray* self = static_cast<ByteArray*>(
      info.This()->GetPointerFromInternalField(0));
  if (index >= self->GetLength()) {
    return handle_scope.Close(v8::Undefined());
  }
  return handle_scope.Close(v8::Number::New(self->Get(index)));
}

v8::Handle<v8::Value> ByteArray::SetIndex(uint32_t index,
    v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  ByteArray* self = static_cast<ByteArray*>(
      info.This()->GetPointerFromInternalField(0));
  if (index < self->GetLength()) {
    if (!value->ToUint32().IsEmpty()) {
      uint32_t byte = value->ToUint32()->Value();
      if (255 >= byte) {
        self->Set(index, byte);
      }
    }
  }
  return handle_scope.Close(value);;
}

v8::Handle<v8::Integer> ByteArray::QueryIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  ByteArray* self = static_cast<ByteArray*>(
      info.This()->GetPointerFromInternalField(0));
  if (index < self->GetLength()) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

v8::Handle<v8::Value> ByteArray::Copy(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  ByteArray* self = static_cast<ByteArray*>(
      arguments.This()->GetPointerFromInternalField(0));
  uint32_t start = 0, stop = self->GetLength(), target_start = 0;
  switch (arguments.Length()) {
  case 4:
    if (arguments[3]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Argument four must be an unsigned integer")));
    }
    target_start = arguments[3]->ToUint32()->Value();
    // Fall through
  case 3:
    if (arguments[2]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Argument three must be an unsigned integer")));
    }
    stop = arguments[2]->ToUint32()->Value() + 1;
    // Fall through
  case 2:
    if (arguments[1]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Argument two must be an unsigned integer")));
    }
    start = arguments[1]->ToUint32()->Value();
    // Fall through
  case 1:
    if (stop > self->GetLength()) {
      stop = self->GetLength();
    }
    if (arguments[0]->IsArray()) {
      v8::Array* array = v8::Array::Cast(*arguments[0]);
      for (uint32_t index = start; index < stop; ++index) {
        array->Set(target_start++, v8::Uint32::New(self->Get(index)));
      }
      return handle_scope.Close(v8::Undefined());
    } else if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (GetTemplate()->HasInstance(object)) {
        ByteArray* that = static_cast<ByteArray*>(
            object->GetPointerFromInternalField(0));
        uint32_t that_length = stop - start + target_start;
        if (that_length > that->GetLength()) {
          that->Resize(that_length);
        }
        uint32_t j = 0;
        for (uint32_t index = start; index < stop; ++index) {
          that->Set(target_start + j++, self->Get(index));
        }
        return handle_scope.Close(v8::Undefined());
      }
    }
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Argument one must be of type Array or ByteArray")));
  default:
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("One, two, three or four argument allowed")));
  }
}

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
