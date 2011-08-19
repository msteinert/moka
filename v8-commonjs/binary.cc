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

#include <cerrno>
#include <cstring>
#include <iconv.h>
#include "v8-commonjs/binary.h"

namespace commonjs {

// Binary
Binary::Binary()
  : length_(0)
  , data_(NULL) {}

Binary::~Binary() {
  if (data_) {
    delete[] data_;
  }
}

v8::Handle<v8::Value> Binary::Initialize(int length) {
  v8::HandleScope handle_scope;
  if (data_) {
    length_ = 0;
    delete[] data_;
    data_ = NULL;
  }
  if (length) {
    data_ = new unsigned char[length];
    if (!data_) {
      return handle_scope.Close(v8::Exception::Error(
            v8::String::New("No memory")));
    }
    length_ = length;
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::FunctionTemplate> Binary::GetTemplate() {
  v8::HandleScope handle_scope;
  v8::Local<v8::FunctionTemplate> binary_templ =
    v8::FunctionTemplate::New(Construct);
  binary_templ->SetClassName(v8::String::NewSymbol("Binary"));
  return handle_scope.Close(binary_templ);
}

v8::Handle<v8::Value> Binary::Construct(const v8::Arguments& /* arguments */) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Value> exception = v8::Exception::TypeError(
      v8::String::New("Binary cannot be constructed"));
  return handle_scope.Close(v8::ThrowException(exception));
}

// ByteString
v8::Handle<v8::Value> ByteString::Initialize(Binary* that) {
  v8::HandleScope handle_scope;
  uint32_t length = that->GetLength();
  v8::Handle<v8::Value> exception = Binary::Initialize(length);
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  unsigned char* data = GetData();
  unsigned char* that_data = that->GetData();
  for (uint32_t index = 0; index < length; ++index) {
    data[index] = that_data[index];
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> ByteString::Initialize(v8::Handle<v8::Object> /* object */) {
  v8::HandleScope handle_scope;
  // TODO
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> ByteString::Initialize(v8::Handle<v8::Array> numbers) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Array> properties = numbers->GetPropertyNames();
  if (properties.IsEmpty()) {
    return handle_scope.Close(v8::Exception::Error(
          v8::String::New("Array has no properties")));
  }
  uint32_t length = properties->Length();
  v8::Handle<v8::Value> exception = Binary::Initialize(length);
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  unsigned char* data = GetData();
  uint32_t index = 0;
  while (index < length) {
    v8::Local<v8::Value> index_value = properties->Get(index);
    if (index_value->IsUint32()) {
      v8::Local<v8::Value> number_value =
        numbers->Get(index_value->Uint32Value());
      if (number_value.IsEmpty()) {
        return handle_scope.Close(v8::Exception::Error(
              v8::String::New("Array has empty property")));
      }
      if (!number_value->IsUint32()) {
        return handle_scope.Close(v8::Exception::TypeError(
              v8::String::New("Array values must be unsigned integers")));
      }
      uint32_t number = number_value->Uint32Value();
      if (number > 255) {
        return handle_scope.Close(v8::Exception::RangeError(
              v8::String::New("Array values must be in the range [0..255]")));
      }
      data[index] = number;
      ++index;
    }
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> ByteString::Initialize(v8::Handle<v8::String> /* string */,
    v8::Handle<v8::String> charset) {
  v8::HandleScope handle_scope;
  iconv_t cd = ::iconv_open(*v8::String::Utf8Value(charset), "ASCII");
  if ((iconv_t *)-1 == cd) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  // TODO
  if (-1 == ::iconv_close(cd)) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::FunctionTemplate> ByteString::GetTemplate() {
  v8::HandleScope handle_scope;
  v8::Local<v8::FunctionTemplate> byte_string_templ =
    v8::FunctionTemplate::New(Construct);
  byte_string_templ->SetClassName(v8::String::NewSymbol("ByteString"));
  byte_string_templ->Inherit(Binary::GetTemplate());
  return handle_scope.Close(byte_string_templ);
}

v8::Handle<v8::Value> ByteString::Construct(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  ByteString* self = NULL;
  if (arguments.Length() == 0) {
    self = new ByteString;
  } else if (arguments.Length() == 1) {
    if (arguments[0]->IsObject()) {
      if (arguments[0]->IsArray()) {
        self = new ByteString;
        if (self) {
          v8::Handle<v8::Value> exception = 
            self->Initialize(v8::Handle<v8::Array>::Cast(arguments[0]));
          if (!exception.IsEmpty()) {
            delete self;
            return handle_scope.Close(v8::ThrowException(exception));
          }
        }
      } else {
        self = new ByteString;
        if (self) {
          v8::Handle<v8::Value> exception = 
            self->Initialize(v8::Handle<v8::Object>::Cast(arguments[0]));
          if (!exception.IsEmpty()) {
            delete self;
            return handle_scope.Close(v8::ThrowException(exception));
          }
        }
      }
    } else {
      return handle_scope.Close(v8::ThrowException(v8::String::New(
              "Argument must of type Array, ByteString, or ByteArray")));
    }
  } else if (arguments.Length() == 2) {
    if (arguments[0]->IsString() && arguments[1]->IsString()) {
      self = new ByteString;
      if (self) {
        v8::Handle<v8::Value> exception = 
          self->Initialize(v8::Handle<v8::String>::Cast(arguments[0]),
              v8::Handle<v8::String>::Cast(arguments[1]));
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
  v8::Local<v8::ObjectTemplate> byte_string_templ = v8::ObjectTemplate::New();
  byte_string_templ->SetInternalFieldCount(1);
  // TODO add methods
  v8::Persistent<v8::Object> byte_string =
    v8::Persistent<v8::Object>::New(byte_string_templ->NewInstance());
  byte_string->SetInternalField(0, v8::External::New(self));
  byte_string.MakeWeak(static_cast<void*>(self), Delete);
  return byte_string;
}

void ByteString::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  delete static_cast<ByteString*>(parameters);
  object.Dispose();
  object.Clear();
}

static bool binary_initialize(Module& module,
    int* /* argc */, char*** /* argv */)
{
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("Binary"),
      Binary::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("ByteString"),
      ByteString::GetTemplate()->GetFunction());
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::binary_initialize)

// vim: tabstop=2:sw=2:expandtab
