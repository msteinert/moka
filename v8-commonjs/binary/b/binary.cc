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
#include <sstream>
#include "v8-commonjs/binary/b/binary.h"
#include "v8-commonjs/iconv.h"

namespace commonjs {

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

v8::Handle<v8::Value> Binary::Split(char value,
    v8::Handle<v8::Function> construct, uint32_t count,
    bool include_delimiter) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Array> split = v8::Array::New();
  char* current = data_;
  for (uint32_t index = 0; index < length_; ++index) {
    if (data_[index] == value) {
      v8::Handle<v8::Value> binary = construct->NewInstance();
      Binary* that = static_cast<Binary*>(
          binary->ToObject()->GetPointerFromInternalField(0));
      v8::Handle<v8::Value> value = that->Resize(index - (current - data_));
      if (!value.IsEmpty()) {
        return handle_scope.Close(v8::ThrowException(value));
      }
      ::memcpy(that->data_, current, that->length_);
      split->Set(split->Length(), binary);
      if (split->Length() == count - 1) {
        ++current;
        break;
      }
      current += that->length_;
      if (include_delimiter) {
        v8::Handle<v8::Value> binary = construct->NewInstance();
        Binary* that = static_cast<Binary*>(
            binary->ToObject()->GetPointerFromInternalField(0));
        v8::Handle<v8::Value> value = that->Resize(1);
        if (!value.IsEmpty()) {
          return handle_scope.Close(v8::ThrowException(value));
        }
        that->data_[0] = *current;
        split->Set(split->Length(), binary);
        if (split->Length() == count - 1) {
          ++current;
          break;
        }
      }
      ++current;
    }
  }
  if (split->Length() < count) {
    if (current < data_ + length_) {
      v8::Handle<v8::Value> binary = construct->NewInstance();
      Binary* that = static_cast<Binary*>(
          binary->ToObject()->GetPointerFromInternalField(0));
      v8::Handle<v8::Value> value = that->Resize((data_ + length_) - current);
      if (!value.IsEmpty()) {
        return handle_scope.Close(v8::ThrowException(value));
      }
      ::memcpy(that->data_, current, that->length_);
      split->Set(split->Length(), binary);
    }
  }
  return handle_scope.Close(split);
}

v8::Handle<v8::Value> Binary::Split(Binary* that,
    v8::Handle<v8::Function> construct, uint32_t count,
    bool include_delimiter) {
  v8::HandleScope handle_scope;
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> Binary::Split(v8::Array* array,
    v8::Handle<v8::Function> construct, uint32_t count,
    bool include_delimiter) {
  v8::HandleScope handle_scope;
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
  if (GetTemplate()->HasInstance(object)) {
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
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toSource"),
      v8::FunctionTemplate::New(ToSource)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("decodeToString"),
      v8::FunctionTemplate::New(DecodeToString)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("indexOf"),
      v8::FunctionTemplate::New(IndexOf)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("lastIndexOf"),
      v8::FunctionTemplate::New(LastIndexOf)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("byteAt"),
      v8::FunctionTemplate::New(ByteAt)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("valueAt"),
      v8::FunctionTemplate::New(ByteAt)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("codeAt"),
      v8::FunctionTemplate::New(CodeAt)->GetFunction());
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("get"),
      v8::FunctionTemplate::New(CodeAt)->GetFunction());
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
  Binary* self = static_cast<Binary*>(
      info.This()->GetPointerFromInternalField(0));
  return handle_scope.Close(v8::Uint32::New(self->length_));
}

v8::Handle<v8::Value> Binary::ToArray(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
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

v8::Handle<v8::Value> Binary::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
  std::stringstream stream;
  stream << '[';
  stream << *v8::String::Utf8Value(arguments.This()->GetConstructorName());
  stream << ' ';
  stream << self->GetLength();
  stream << ']';
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

v8::Handle<v8::Value> Binary::ToSource(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
  std::stringstream stream;
  stream << *v8::String::Utf8Value(arguments.This()->GetConstructorName());
  stream << "([";
  for (uint32_t index = 0; index < self->GetLength(); ++index) {
    if (index) {
      stream << ", ";
    }
    stream << static_cast<uint32_t>(self->Get(index));
  }
  stream << "])";
  return handle_scope.Close(v8::String::New(stream.str().c_str()));
}

v8::Handle<v8::Value> Binary::DecodeToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
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

v8::Handle<v8::Value> Binary::IndexOf(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
  uint32_t start = 0, end = self->GetLength(), byte;
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument three must be an unsigned integer"))));
    }
    end = arguments[2]->ToUint32()->Value() + 1;
    if (end > self->GetLength()) {
      end = self->GetLength() - 1;
    }
    // Fall through
  case 2:
    if (arguments[1]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an unsigned integer"))));
    }
    start = arguments[1]->ToUint32()->Value();
    if (start >= self->GetLength()) {
      return handle_scope.Close(v8::Integer::New(-1));
    }
    // Fall through
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (!GetTemplate()->HasInstance(object)) {
        return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
                v8::String::New("Argument one must be an unsigned integer, "
                  "a ByteString or a ByteArray"))));
      }
      Binary* that = static_cast<Binary*>(
          object->GetPointerFromInternalField(0));
      for (uint32_t index = start; index < end; ++index) {
        for (uint32_t j = 0; j < that->GetLength(); ++j) {
          if (end < (index + j)) {
            return handle_scope.Close(v8::Number::New(-1));
          }
          if (self->Get(index + j) != that->Get(j)) {
            break;
          }
          if (that->GetLength() == (j + 1)) {
            return handle_scope.Close(v8::Number::New(index));
          }
        }
      }
      return handle_scope.Close(v8::Number::New(-1));
    } else if (!arguments[0]->ToUint32().IsEmpty()) {
      byte = arguments[0]->ToUint32()->Value();
      for (uint32_t index = start; index < end; ++index) {
        uint32_t value = self->Get(index);
        if (value == byte) {
          return handle_scope.Close(v8::Number::New(index));
        }
      }
      return handle_scope.Close(v8::Number::New(-1));
    } else {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned integer, "
                "a ByteString or a ByteArray"))));
    }
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One, two or three argument allowed"))));
  }
}

v8::Handle<v8::Value> Binary::LastIndexOf(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  Binary* self = static_cast<Binary*>(
      arguments.This()->GetPointerFromInternalField(0));
  uint32_t start = self->GetLength() - 1, end = 0, byte;
  switch (arguments.Length()) {
  case 3:
    if (arguments[2]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument three must be an unsigned integer"))));
    }
    end = arguments[2]->ToUint32()->Value();
    if (end >= self->GetLength()) {
      return handle_scope.Close(v8::Number::New(-1));
    }
    // Fall through
  case 2:
    if (arguments[1]->ToUint32().IsEmpty()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument two must be an unsigned integer"))));
    }
    start = arguments[1]->ToUint32()->Value();
    if (start >= self->GetLength()) {
      start = self->GetLength() - 1;
    }
    // Fall through
  case 1:
    if (arguments[0]->IsObject()) {
      v8::Handle<v8::Object> object = arguments[0]->ToObject();
      if (!GetTemplate()->HasInstance(object)) {
        return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
                v8::String::New("Argument one must be an unsigned integer, "
                  "a ByteString or a ByteArray"))));
      }
      Binary* that = static_cast<Binary*>(
          object->GetPointerFromInternalField(0));
      for (uint32_t index = start; index >= end; --index) {
        for (uint32_t j = 0; j < that->GetLength(); ++j) {
          if (that->GetLength() > index) {
            return handle_scope.Close(v8::Number::New(-1));
          }
          if (self->Get(index - that->GetLength() + j) != that->Get(j)) {
            break;
          }
          if (that->GetLength() == (j + 1)) {
            return handle_scope.Close(v8::Number::New(
                  index - that->GetLength()));
          }
        }
      }
      return handle_scope.Close(v8::Number::New(-1));
    } else if (!arguments[0]->ToUint32().IsEmpty()) {
      byte = arguments[0]->ToUint32()->Value();
      for (uint32_t index = start; index >= end; --index) {
        if (static_cast<uint32_t>(self->Get(index)) == byte) {
          return handle_scope.Close(v8::Number::New(index));
        }
      }
      return handle_scope.Close(v8::Number::New(-1));
    } else {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned integer, "
                "a ByteString or a ByteArray"))));
    }
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One, two or three argument allowed"))));
  }
}

v8::Handle<v8::Value> Binary::CodeAt(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  switch (arguments.Length()) {
  case 1:
    if (!arguments[0]->ToUint32().IsEmpty()) {
      Binary* self = static_cast<Binary*>(
          arguments.This()->GetPointerFromInternalField(0));
      uint32_t index = arguments[0]->ToUint32()->Value();
      if (index < self->GetLength()) {
        return handle_scope.Close(v8::Number::New(self->Get(index)));
      }
    } else {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an unsigned integer"))));
    }
    return handle_scope.Close(v8::Undefined());
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument allowed"))));
  }
}

v8::Handle<v8::Value> Binary::ByteAt(const v8::Arguments& arguments) {
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
