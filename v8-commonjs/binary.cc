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
#include <cstdlib>
#include <cstring>
#include <iconv.h>
#include <sstream>
#include "v8-commonjs/binary.h"

namespace commonjs {

// Binary
Binary::Binary()
  : size_(0)
  , length_(0)
  , data_(NULL) {}

Binary::~Binary() {
  if (data_) {
    ::free(data_);
  }
}

v8::Handle<v8::Value> Binary::Resize(uint32_t length) {
  v8::HandleScope handle_scope;
  if (size_ >= length) {
    if (length > length_) {
      ::memset(data_ + length_, 0, length - length_);
    }
    length_ = length;
  } else {
    if (length) {
      char* data = static_cast<char*>(
          ::realloc(static_cast<void*>(data_), length));
      if (!data) {
        char message[BUFSIZ];
        ::strerror_r(errno, message, BUFSIZ);
        return handle_scope.Close(v8::Exception::Error(
              v8::String::New(message)));
      }
      ::memcpy(data, data_, length_);
      if (length > length_) {
        ::memset(data + length_, 0, length - length_);
      }
      size_ = length;
      length_ = length;
      ::free(data_);
      data_ = data;
    }
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> Binary::Join(v8::Handle<v8::Array> array,
    char number) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Array> properties = array->GetPropertyNames();
  if (properties.IsEmpty()) {
    return handle_scope.Close(v8::Exception::Error(
          v8::String::New("Array has no properties")));
  }
  uint32_t length = properties->Length();
  v8::Handle<v8::Value> exception = Resize((length * 2) - 1);
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  uint32_t other = 0;
  for (uint32_t index = 0; index < length_; ++index) {
    if (index % 2) {
      data_[index] = number;
    } else {
      v8::Local<v8::Value> index_value = properties->Get(other++);
      if (index_value->IsUint32()) {
        v8::Local<v8::Value> number_value =
          array->Get(index_value->Uint32Value());
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
        data_[index] = number;
      }
    }
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> Binary::Construct(int length) {
  v8::HandleScope handle_scope;
  if (data_) {
    size_ = 0;
    length_ = 0;
    ::free(data_);
    data_ = NULL;
  }
  if (length) {
    data_ = static_cast<char*>(::calloc(length, sizeof(char)));
    if (!data_) {
      char message[BUFSIZ];
      ::strerror_r(errno, message, BUFSIZ);
      return handle_scope.Close(v8::Exception::Error(
            v8::String::New(message)));
    }
    size_ = length;
    length_ = length;
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> Binary::Construct(v8::Handle<v8::Object> object) {
  v8::HandleScope handle_scope;
  if (Binary::GetTemplate()->HasInstance(object)) {
    Binary* that = static_cast<Binary*>(object->GetPointerFromInternalField(0));
    uint32_t length = that->GetLength();
    v8::Handle<v8::Value> exception = Construct(length);
    if (!exception.IsEmpty()) {
      return handle_scope.Close(exception);
    }
    char* data = GetData();
    char* that_data = that->GetData();
    for (uint32_t index = 0; index < length; ++index) {
      data[index] = that_data[index];
    }
    return handle_scope.Close(v8::Handle<v8::Value>());
  } else {
    return handle_scope.Close(v8::Exception::TypeError(v8::String::New(
            "Argument must of type Array, ByteString, or ByteArray")));
  }
}

v8::Handle<v8::Value> Binary::Construct(v8::Handle<v8::Array> numbers) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Array> properties = numbers->GetPropertyNames();
  if (properties.IsEmpty()) {
    return handle_scope.Close(v8::Exception::Error(
          v8::String::New("Array has no properties")));
  }
  uint32_t length = properties->Length();
  v8::Handle<v8::Value> exception = Construct(length);
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  char* data = GetData();
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

v8::Handle<v8::Value> Binary::Construct(v8::Handle<v8::String> string,
    v8::Handle<v8::String> charset) {
  v8::HandleScope handle_scope;
  std::string fromcode;
  if (charset.IsEmpty()) {
    fromcode.assign("UTF-8");
  } else {
    fromcode.assign(*v8::String::Utf8Value(charset));
  }
  Iconv cd;
  v8::Handle<v8::Value> value = cd.Convert(*v8::String::Utf8Value(string),
      string->Length(), "UTF-8", fromcode.c_str());
  if (!value.IsEmpty()) {
    return handle_scope.Close(value);
  }
  v8::Handle<v8::Value> exception = Construct(cd.GetLength());
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  memcpy(GetData(), cd.GetData(), cd.GetLength());
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> Binary::Construct(v8::Handle<v8::Uint32> length) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> exception = Construct(length->Value());
  if (!exception.IsEmpty()) {
    return handle_scope.Close(exception);
  }
  memset(GetData(), 0, GetLength());
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::FunctionTemplate> Binary::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("Binary"));
  templ->PrototypeTemplate()->SetAccessor(v8::String::NewSymbol("length"),
      LengthGet);
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toArray"),
      v8::FunctionTemplate::New(ToArray)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("decodeToString"),
      v8::FunctionTemplate::New(DecodeToString)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Binary::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Value> exception = v8::Exception::TypeError(
      v8::String::New("Binary cannot be constructed"));
  return handle_scope.Close(v8::ThrowException(exception));
}

v8::Handle<v8::Value> Binary::LengthGet(v8::Local<v8::String> property,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  return handle_scope.Close(v8::Uint32::New(self->length_));
}

v8::Handle<v8::Value> Binary::ToArray(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (0 == arguments.Length()) {
    v8::Local<v8::Array> array = v8::Array::New(self->GetLength());
    for (uint32_t index = 0; index < self->GetLength(); ++index) {
      array->Set(index, v8::Uint32::New(self->Get(index)));
    }
    return handle_scope.Close(array);
  } else if (1 == arguments.Length()) {
    if (!arguments[0]->IsString()) {
      return handle_scope.Close(v8::ThrowException(
              v8::String::New("Argument one must be a string")));
    }
    Iconv cd;
    v8::Handle<v8::Value> value = cd.Convert(self->GetData(),
        self->GetLength(), "UTF-8",
        *v8::String::Utf8Value(arguments[0]->ToString()));
    if (!value.IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(value));
    }
    v8::Local<v8::Array> array = v8::Array::New(cd.GetLength());
    for (uint32_t index = 0; index < cd.GetLength(); ++index) {
      array->Set(index, v8::Uint32::New(cd.GetData()[index]));
    }
    return handle_scope.Close(array);
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Zero or one argument allowed"))));
  }
}

v8::Handle<v8::Value> Binary::DecodeToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (0 == arguments.Length()) {
    return handle_scope.Close(v8::String::New(self->GetData(),
          self->GetLength()));
  } else if (1 == arguments.Length()) {
    if (!arguments[0]->IsString()) {
      return handle_scope.Close(v8::ThrowException(
              v8::String::New("Argument one must be a string")));
    }
    Iconv cd;
    v8::Handle<v8::Value> value = cd.Convert(self->GetData(),
        self->GetLength(), "UTF-8",
        *v8::String::Utf8Value(arguments[0]->ToString()));
    if (!value.IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(value));
    }
    return handle_scope.Close(v8::String::New(cd.GetData(), cd.GetLength()));
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Zero or one argument allowed"))));
  }
}

// ByteString
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
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toSource"),
      v8::FunctionTemplate::New(ToSource)->GetFunction());
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
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (index >= self->GetLength()) {
    return handle_scope.Close(v8::Exception::RangeError(
          v8::String::New("Index is out of range")));
  }
  return handle_scope.Close(v8::Integer::New(self->Get(index)));
}

v8::Handle<v8::Value> ByteString::SetIndex(uint32_t index,
    v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Integer> ByteString::QueryIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (index < self->GetLength()) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

v8::Handle<v8::Value> ByteString::Join(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  ByteString* that = NULL;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
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

v8::Handle<v8::Value> ByteString::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  std::stringstream stream;
  stream << "[ByteString ";
  stream << self->GetLength();
  stream << "]";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

v8::Handle<v8::Value> ByteString::ToSource(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  std::stringstream stream;
  stream << "ByteString([";
  for (uint32_t index = 0; index < self->GetLength(); ++index) {
    if (index) {
      stream << ", ";
    }
    stream << static_cast<uint32_t>(self->Get(index));
  }
  stream << "])";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

// ByteArray
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
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toSource"),
      v8::FunctionTemplate::New(ToSource)->GetFunction());
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
    v8::Local<v8::Object> object = info.This();
    v8::Local<v8::External> external =
      v8::Local<v8::External>::Cast(object->GetInternalField(0));
    Binary* self = static_cast<Binary*>(external->Value());
    self->Resize(value->ToUint32()->Value());
  }
}

v8::Handle<v8::Value> ByteArray::GetIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (index >= self->GetLength()) {
    return handle_scope.Close(v8::Undefined());
  }
  return handle_scope.Close(v8::Integer::New(self->Get(index)));
}

v8::Handle<v8::Value> ByteArray::SetIndex(uint32_t index,
    v8::Local<v8::Value> value, const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (index >= self->GetLength()) {
    v8::Handle<v8::Value> exception = self->Resize(index + 1);
    if (!exception.IsEmpty()) {
      return handle_scope.Close(v8::Undefined());
    }
  }
  if (!value->IsUint32()) {
    return handle_scope.Close(v8::Undefined());
  }
  uint32_t number = value->Uint32Value();
  if (number > 255) {
    return handle_scope.Close(v8::Undefined());
  }
  self->Set(index, number);
  return handle_scope.Close(value);;
}

v8::Handle<v8::Integer> ByteArray::QueryIndex(uint32_t index,
    const v8::AccessorInfo &info) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = info.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  if (index < self->GetLength()) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

v8::Handle<v8::Value> ByteArray::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  std::stringstream stream;
  stream << "[ByteArray ";
  stream << self->GetLength();
  stream << "]";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

v8::Handle<v8::Value> ByteArray::ToSource(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> object = arguments.This();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Binary* self = static_cast<Binary*>(external->Value());
  std::stringstream stream;
  stream << "ByteArray([";
  for (uint32_t index = 0; index < self->GetLength(); ++index) {
    if (index) {
      stream << ", ";
    }
    stream << static_cast<uint32_t>(self->Get(index));
  }
  stream << "])";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

// Iconv
Iconv::Iconv()
  : length_(0)
  , data_(NULL) {}

Iconv::~Iconv() {
  if (data_) {
    ::free(data_);
  }
}

v8::Handle<v8::Value> Iconv::Convert(const char* data, uint32_t length,
    const char* tocode, const char* fromcode) {
  v8::HandleScope handle_scope;
  iconv_t cd = ::iconv_open(tocode, fromcode);
  if ((iconv_t *)-1 == cd) {
    if (EINVAL == errno) {
      std::string message("Conversion from ");
      message.append(fromcode);
      message.append(" to ");
      message.append(tocode);
      message.append(" is not available");
      return handle_scope.Close(v8::Exception::TypeError(
            v8::String::New(message.c_str())));
    }
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  size_t size = length * sizeof(wchar_t);
  size_t outbytesleft = size;
  data_ = static_cast<char*>(::malloc(size));
  if (!data_) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  char* outbuf = data_;
  size_t inbytesleft = length;
  char* in_buffer = static_cast<char*>(::malloc(inbytesleft + 1));
  if (!in_buffer) {
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  memcpy(in_buffer, data, length);
  char* inbuf = in_buffer;
  int converted = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  while (-1 == converted) {
    if (EINVAL == errno) {
      // Junk at the end of the buffer, ignore it
      break;
    } else if (E2BIG == errno) {
      // Double the size of the output buffer
      size *= 2;
      char* new_buffer = static_cast<char*>(::realloc(data_, size));
      if (!new_buffer) {
        ::free(in_buffer);
        char message[BUFSIZ];
        ::strerror_r(errno, message, BUFSIZ);
        return handle_scope.Close(v8::Exception::Error(
              v8::String::New(message)));
      }
      outbytesleft = size;
      data_ = outbuf = new_buffer;
    } else {
      // Unrecoverable error
      ::free(in_buffer);
      char message[BUFSIZ];
      ::strerror_r(errno, message, BUFSIZ);
      return handle_scope.Close(v8::Exception::Error(
            v8::String::New(message)));
    }
    inbytesleft = length;
    inbuf = in_buffer;
    converted = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
  }
  if (-1 == ::iconv_close(cd)) {
    ::free(in_buffer);
    char message[BUFSIZ];
    ::strerror_r(errno, message, BUFSIZ);
    return handle_scope.Close(v8::Exception::Error(v8::String::New(message)));
  }
  length_ = outbuf - data_;
  ::free(in_buffer);
  return handle_scope.Close(v8::Handle<v8::Value>());
}

// Initialize module
static bool BinaryInitialize(Module& module, int* argc, char*** argv)
{
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("Binary"),
      Binary::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("ByteString"),
      ByteString::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("ByteArray"),
      ByteArray::GetTemplate()->GetFunction());
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::BinaryInitialize)

// vim: tabstop=2:sw=2:expandtab
