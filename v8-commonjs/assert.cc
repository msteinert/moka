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

#include "v8-commonjs/assert.h"

namespace commonjs {

v8::Handle<v8::Value> AssertionError::New(v8::Handle<v8::Value> actual,
    v8::Handle<v8::Value> expected, v8::Handle<v8::Value> message) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> object;
  if (!actual.IsEmpty()) {
    if (object.IsEmpty()) {
      object = v8::Object::New();
    }
    object->Set(v8::String::NewSymbol("actual"), actual);
  }
  if (!expected.IsEmpty()) {
    if (object.IsEmpty()) {
      object = v8::Object::New();
    }
    object->Set(v8::String::NewSymbol("expected"), expected);
  }
  if (!message.IsEmpty()) {
    if (object.IsEmpty()) {
      object = v8::Object::New();
    }
    object->Set(v8::String::NewSymbol("message"), message);
  }
  if (object.IsEmpty()) {
    return GetTemplate()->GetFunction()->NewInstance();
  } else {
    v8::Handle<v8::Value> argv[1] = { object };
    return GetTemplate()->GetFunction()->NewInstance(1, argv);
  }
}

v8::Handle<v8::FunctionTemplate> AssertionError::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("AssertionError"));
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> AssertionError::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    int argc = arguments.Length();
    v8::Local<v8::Value>* argv = new v8::Local<v8::Value>[argc];
    if (!argv) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("No memory")));
    }
    for (int index = 0; index < argc; ++index) {
      argv[index] = arguments[index];
    }
    v8::Local<v8::Object> instance =
      GetTemplate()->GetFunction()->NewInstance(argc, argv);
    delete[] argv;
    return handle_scope.Close(instance);
  }
  v8::Handle<v8::Object> self = arguments.This();
  self->Set(v8::String::NewSymbol("name"), v8::String::New("AssertionError"));
  if (arguments.Length()) {
    v8::Local<v8::String> message_string = v8::String::NewSymbol("message");
    if (arguments[0]->IsObject()) {
      v8::Local<v8::Object> options = arguments[0]->ToObject();
      v8::Local<v8::String> actual_string = v8::String::NewSymbol("actual");
      v8::Handle<v8::Value> actual = options->Get(actual_string);
      if (!actual.IsEmpty()) {
        self->Set(actual_string, actual);
      }
      v8::Local<v8::String> expected_string = v8::String::NewSymbol("expected");
      v8::Handle<v8::Value> expected = options->Get(expected_string);
      if (!expected.IsEmpty()) {
        self->Set(expected_string, expected);
      }
      v8::Handle<v8::Value> message = options->Get(message_string);
      if (!message_string.IsEmpty()) {
        self->Set(message_string, message);
      }
    } else {
      if (!arguments[0].IsEmpty()) {
        self->Set(message_string, arguments[0]);
      }
    }
  }
  return self;
}

v8::Handle<v8::Value> AssertionError::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  std::string string;
  v8::Local<v8::Object> self = arguments.This();
  v8::Local<v8::Value> name = self->Get(v8::String::NewSymbol("name"));
  if (!name->Equals(v8::Undefined())) {
    string.append(*v8::String::Utf8Value(name->ToString()));
  }
  v8::Local<v8::Value> message = self->Get(v8::String::NewSymbol("message"));
  if (!message->Equals(v8::Undefined())) {
    if (!string.empty()) {
      string.append(": ");
    }
    string.append(*v8::String::Utf8Value(message->ToString()));
  }
  if (string.empty()) {
    return handle_scope.Close(v8::Local<v8::String>());
  }
  return handle_scope.Close(v8::String::New(string.c_str()));
}

static v8::Handle<v8::Value> Ok(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (1 == arguments.Length()) {
    if (!arguments[0]->ToBoolean()->Equals(v8::True())) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], v8::True())));
    }
  } else if (2 <= arguments.Length()) {
    if (!arguments[0]->ToBoolean()->Equals(v8::True())) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], v8::True(), arguments[1])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One or two arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> Equal(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (!arguments[0]->Equals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1])));
    }
  } else if (3 <= arguments.Length()) {
    if (!arguments[0]->Equals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1], arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> NotEqual(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (arguments[0]->Equals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1])));
    }
  } else if (3 <= arguments.Length()) {
    if (arguments[0]->Equals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1], arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

bool RealDeepEqual(v8::Handle<v8::Value> a, v8::Handle<v8::Value> b) {
  if (a->StrictEquals(b)) {
    return true;
  } else if (a->IsDate() && b->IsDate()) {
    return a->NumberValue() == b->NumberValue();
  } else if (!a->IsObject() && !b->IsObject()) {
    return a->Equals(b);
  } else if (a->IsString() || b->IsString()) {
    return a->StrictEquals(b);
  } else {
    v8::Local<v8::Object> a_object = a->ToObject();
    v8::Local<v8::Object> b_object = b->ToObject();
    v8::Local<v8::String> prototype = v8::String::NewSymbol("prototype");
    if (!a_object->Get(prototype)->Equals(b_object->Get(prototype))) {
      return false;
    }
    v8::Local<v8::Array> a_properties = a_object->GetPropertyNames();
    for (uint32_t index = 0; index < a_properties->Length(); ++index) {
      v8::Local<v8::Value> value = a_properties->Get(index);
      if (!RealDeepEqual(a_object->Get(value), b_object->Get(value))) {
        return false;
      }
    }
    v8::Local<v8::Array> b_properties = b_object->GetPropertyNames();
    for (uint32_t index = 0; index < b_properties->Length(); ++index) {
      v8::Local<v8::Value> value = b_properties->Get(index);
      if (!RealDeepEqual(b_object->Get(value), a_object->Get(value))) {
        return false;
      }
    }
    return true;
  }
}

static v8::Handle<v8::Value> DeepEqual(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (!RealDeepEqual(arguments[0], arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              v8::Handle<v8::Value>(), v8::Handle<v8::Value>())));
    }
  } else if (3 <= arguments.Length()) {
    if (!RealDeepEqual(arguments[0], arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              v8::Handle<v8::Value>(), v8::Handle<v8::Value>(), arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> NotDeepEqual(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (RealDeepEqual(arguments[0], arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1])));
    }
  } else if (3 <= arguments.Length()) {
    if (RealDeepEqual(arguments[0], arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1], arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> StrictEqual(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (!arguments[0]->StrictEquals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1])));
    }
  } else if (3 <= arguments.Length()) {
    if (!arguments[0]->StrictEquals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1], arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> NotStrictEqual(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (2 == arguments.Length()) {
    if (arguments[0]->StrictEquals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1])));
    }
  } else if (3 <= arguments.Length()) {
    if (arguments[0]->StrictEquals(arguments[1])) {
      return handle_scope.Close(v8::ThrowException(AssertionError::New(
              arguments[0], arguments[1], arguments[2])));
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Two or three arguments allowed"))));
  }
  return handle_scope.Close(v8::True());
}

static v8::Handle<v8::Value> Error(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> block;
  v8::Handle<v8::Value> error;
  v8::Handle<v8::Value> message;
  if (1 == arguments.Length()) {
    block = arguments[0];
  } else if (2 == arguments.Length()) {
    block = arguments[0];
    if (arguments[1]->IsString()) {
      message = arguments[1];
    } else {
      error= arguments[1];
    }
  } else if (3 <= arguments.Length()) {
    block = arguments[0];
    error = arguments[1];
    message = arguments[2];
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One, two or three arguments allowed"))));
  }
  v8::Local<v8::Value> caught;
  if (block->IsFunction()) {
    v8::TryCatch try_catch;
    v8::Local<v8::Value> value = v8::Function::Cast(*block)->Call(
        block->ToObject(), 0, NULL);
    if (value.IsEmpty()) {
      if (try_catch.HasCaught()) {
        if (error.IsEmpty()) {
          return handle_scope.Close(v8::True());
        }
        caught = try_catch.Exception();
        if (error->IsFunction() && caught->IsObject()) {
          if (caught->ToObject()->GetConstructorName()->Equals(
                v8::Function::Cast(*error)->GetName())) {
            return handle_scope.Close(v8::True());
          }
        } else if (error->IsFunction() && caught->IsObject()) {
          if (error->ToObject()->GetPrototype()->Equals(
                caught->ToObject()->GetPrototype())) {
            return handle_scope.Close(v8::True());
          }
        } else {
          if (error->Equals(caught)) {
            return handle_scope.Close(v8::True());
          }
        }
        return handle_scope.Close(v8::ThrowException(try_catch.ReThrow()));
      }
    }
  } else {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Argument one must be a code block"))));
  }
  return handle_scope.Close(v8::ThrowException(
        AssertionError::New(caught, error, message)));
}

static bool AssertInitialize(Module& module, int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("AssertionError"),
      AssertionError::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("ok"),
      v8::FunctionTemplate::New(Ok)->GetFunction());
  exports->Set(v8::String::NewSymbol("equal"),
      v8::FunctionTemplate::New(Equal)->GetFunction());
  exports->Set(v8::String::NewSymbol("notEqual"),
      v8::FunctionTemplate::New(NotEqual)->GetFunction());
  exports->Set(v8::String::NewSymbol("deepEqual"),
      v8::FunctionTemplate::New(DeepEqual)->GetFunction());
  exports->Set(v8::String::NewSymbol("notDeepEqual"),
      v8::FunctionTemplate::New(NotDeepEqual)->GetFunction());
  exports->Set(v8::String::NewSymbol("strictEqual"),
      v8::FunctionTemplate::New(StrictEqual)->GetFunction());
  exports->Set(v8::String::NewSymbol("notStrictEqual"),
      v8::FunctionTemplate::New(NotStrictEqual)->GetFunction());
  v8::Local<v8::Function> error =
    v8::FunctionTemplate::New(Error)->GetFunction();
  exports->Set(v8::String::NewSymbol("error"), error);
  exports->Set(v8::String::NewSymbol("throws"), error);
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::AssertInitialize)

// vim: tabstop=2:sw=2:expandtab
