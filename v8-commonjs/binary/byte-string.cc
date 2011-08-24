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

#include "v8-commonjs/binary/byte-array.h"
#include "v8-commonjs/binary/byte-string.h"

namespace commonjs {

v8::Handle<v8::FunctionTemplate> ByteString::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->InstanceTemplate()->SetInternalFieldCount(1);
  templ->SetClassName(v8::String::NewSymbol("ByteString"));
  templ->Inherit(Binary::GetTemplate());
  templ->InstanceTemplate()->SetIndexedPropertyHandler(GetIndex, SetIndex,
      QueryIndex);
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("join"),
      v8::FunctionTemplate::New(Join)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("get"),
      v8::FunctionTemplate::New(Get)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("byteAt"),
      v8::FunctionTemplate::New(Get)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("valueAt"),
      v8::FunctionTemplate::New(Get)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("copy"),
      v8::FunctionTemplate::New(ByteArray::Copy)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> ByteString::New(const v8::Arguments& arguments) {
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
  ByteString* self = NULL;
  if (arguments.Length() == 0) {
    self = new ByteString;
  } else if (arguments.Length() == 1) {
    if (arguments[0]->IsString()) {
      self = new ByteString;
      if (self) {
        v8::Handle<v8::Value> exception =
          self->Construct(arguments[0]->ToString());
        if (!exception.IsEmpty()) {
          delete self;
          return handle_scope.Close(v8::ThrowException(exception));
        }
      }
    } else if (arguments[0]->IsObject()) {
      if (arguments[0]->IsArray()) {
        self = new ByteString;
        if (self) {
          v8::Handle<v8::Value> exception = 
            self->Construct(v8::Handle<v8::Array>::Cast(arguments[0]));
          if (!exception.IsEmpty()) {
            delete self;
            return handle_scope.Close(v8::ThrowException(exception));
          }
        }
      } else {
        self = new ByteString;
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
      self = new ByteString;
      if (self) {
        v8::Handle<v8::Value> exception = 
          self->Construct(arguments[0]->ToString(), arguments[1]->ToString());
        if (!exception.IsEmpty()) {
          delete self;
          return handle_scope.Close(v8::ThrowException(exception));
        }
      }
    } else {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one and two must be of type String"))));
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

void ByteString::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<ByteString*>(parameters);
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> ByteString::GetIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      info.This()->GetPointerFromInternalField(0));
  if (index >= self->GetLength()) {
    return handle_scope.Close(v8::Undefined());
  }
  v8::Local<v8::Array> array = v8::Array::New(1);
  array->Set(0, v8::Uint32::New(self->Get(index)));
  v8::Handle<v8::Value> argv[1] = { array };
  return handle_scope.Close(GetTemplate()->GetFunction()->NewInstance(1, argv));
}

v8::Handle<v8::Value> ByteString::SetIndex(uint32_t index,
    v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
  return value;
}

v8::Handle<v8::Integer> ByteString::QueryIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      info.This()->GetPointerFromInternalField(0));
  if (index < self->GetLength()) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

v8::Handle<v8::Value> ByteString::Join(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  ByteString* that = NULL;
  if (1 == arguments.Length()) {
    if (!arguments[0]->IsArray()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be of type Array"))));
    }
    that = new ByteString;
    if (that) {
      v8::Handle<v8::Value> exception = 
        that->Construct(v8::Handle<v8::Array>::Cast(arguments[0]));
      if (!exception.IsEmpty()) {
        delete that;
        return handle_scope.Close(v8::ThrowException(exception));
      }
    }
  } else if (2 == arguments.Length()) {
    if (!arguments[0]->IsArray()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be of type Array"))));
    }
    if (!arguments[1]->IsUint32()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument must be an unsigned integer"))));
    }
    uint32_t number = arguments[1]->ToUint32()->Value();
    if (number > 255) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument must be in the range [0..255]"))));
    }
    that = new ByteString;
    if (that) {
      v8::Handle<v8::Value> exception =
        that->Binary::Join(v8::Handle<v8::Array>::Cast(arguments[0]), number);
      if (!exception.IsEmpty()) {
        delete that;
        return handle_scope.Close(v8::ThrowException(exception));
      }
    }
  } else {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("One or two arguments allowed")));
  }
  if (!that) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("No memory")));
  }
  v8::Persistent<v8::Object> byte_string =
    v8::Persistent<v8::Object>::New(arguments.This());
  byte_string->SetInternalField(0, v8::External::New(that));
  byte_string.MakeWeak(static_cast<void*>(that), Delete);
  return byte_string;
}

v8::Handle<v8::Value> ByteString::Get(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  switch (arguments.Length()) {
  case 1:
    if (!arguments[0]->ToUint32().IsEmpty()) {
      Binary* self = static_cast<Binary*>(
          arguments.This()->GetPointerFromInternalField(0));
      uint32_t index = arguments[0]->ToUint32()->Value();
      if (index >= self->GetLength()) {
        return handle_scope.Close(v8::Undefined());
      }
      v8::Local<v8::Array> array = v8::Array::New(1);
      array->Set(0, v8::Uint32::New(self->Get(index)));
      v8::Handle<v8::Value> argv[1] = { array };
      return handle_scope.Close(GetTemplate()->GetFunction()->
          NewInstance(1, argv));
    }
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Argument one must be an unsigned integer")));
  default:
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("One argument allowed")));
  }
}

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
