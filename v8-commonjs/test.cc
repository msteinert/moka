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
#include "v8-commonjs/commonjs.h"

namespace commonjs {

static v8::Handle<v8::Value> RealRun(v8::Handle<v8::Value> argument) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> tests;
  v8::TryCatch try_catch;
  if (argument->IsString()) {
    // Load argument as a module
    v8::Handle<v8::Value> value = Require(argument->ToString());
    if (value.IsEmpty()) {
      if (try_catch.HasCaught()) {
        return handle_scope.Close(try_catch.ReThrow());
      }
    }
    tests = value->ToObject();
  } else {
    tests = argument->ToObject();
  }
  uint32_t failures = 0;
  if (!tests.IsEmpty()) {
    v8::Local<v8::Array> properties = tests->GetPropertyNames();
    uint32_t length = properties->Length();
    // Scan object for test cases
    for (uint32_t index = 0; index < length; ++index) {
      v8::Local<v8::Value> property = properties->Get(index);
      if (!property->IsString()) {
        continue;
      }
      // Properties starting with 'test' (case insensitive) are tests
      if (::strncasecmp(*v8::String::Utf8Value(property->ToString()),
            "test", 4)) {
        continue;
      }
      v8::Local<v8::Value> test = tests->Get(property);
      if (test->IsFunction()) {
        // Run the test function
        v8::Handle<v8::Value> value =
          v8::Function::Cast(*test)->Call(test->ToObject(), 0, NULL);
        if (value.IsEmpty()) {
          if (try_catch.HasCaught()) {
            ++failures;
            if (!try_catch.CanContinue()) {
              return handle_scope.Close(try_catch.ReThrow());
            }
            try_catch.Reset();
          }
        }
      } else {
        // Run sub-tests
        v8::Handle<v8::Value> value = RealRun(test);
        if (value->IsInt32()) {
          failures += value->ToNumber()->Value();
        } else {
          return value;
        }
      }
    }
  }
  // Return the number of failures
  return handle_scope.Close(v8::Integer::New(failures));
}

static v8::Handle<v8::Value> Run(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (0 == arguments.Length()) {
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument required"))));
  }
  v8::Handle<v8::Value> value = RealRun(arguments[0]);
  if (value->IsInt32()) {
    return handle_scope.Close(value);
  } else {
    return handle_scope.Close(v8::ThrowException(value));
  }
}

static bool TestInitialize(Module& module, int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("run"),
      v8::FunctionTemplate::New(Run)->GetFunction());
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::TestInitialize)

// vim: tabstop=2:sw=2:expandtab
