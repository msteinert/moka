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

/// \brief Implements the API found in moka/module.h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include "moka/module.h"

namespace moka {

Module::Module(const char* id, const char* file_name, bool secure,
    v8::Handle<v8::Object> require, v8::Handle<v8::Context> context)
  : id_(id)
  , file_name_(file_name)
  , directory_name_(NULL)
  , secure_(secure)
  , initialized_(false)
  , context_owner_(false)
  , context_(context)
  , require_(require) {}

Module::Module(const char* id, const char* file_name, bool secure,
    v8::Handle<v8::Object> require)
  : id_(id)
  , file_name_(file_name)
  , directory_name_(NULL)
  , secure_(secure)
  , initialized_(false)
  , context_owner_(true)
  , context_(v8::Context::New())
  , require_(require) {}

Module::~Module() {
  if (directory_name_) {
    ::free(directory_name_);
  }
  module_.Dispose();
  exports_.Dispose();
  if (context_owner_) {
    context_.Dispose();
  }
}

bool Module::Initialize() {
  v8::HandleScope handle_scope;
  if (initialized_) {
    return true;
  }
  v8::Context::Scope scope(context_);
  if (!secure_) {
    // Add the print function for testing
    v8::Local<v8::FunctionTemplate> print_templ =
      v8::FunctionTemplate::New(Print);
    context_->Global()->Set(v8::String::New("print"),
        print_templ->GetFunction());
  }
  context_->Global()->Set(v8::String::NewSymbol("require"), require_);
  // Initialize exports
  v8::Local<v8::Object> exports = v8::Object::New();
  // Store 'paths' as a persistent object
  exports_ = v8::Persistent<v8::Object>::New(exports);
  // Create 'exports' object
  context_->Global()->Set(v8::String::NewSymbol("exports"), exports_);
  // Initialize module
  v8::Local<v8::Object> module = v8::Object::New();
  // Create the 'id' property
  module->Set(v8::String::NewSymbol("id"), v8::String::New(id_.c_str()),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  if (!secure_) {
    // Create the 'uri' property
    std::string uri("file://");
    uri.append(file_name_);
    module->Set(v8::String::NewSymbol("uri"), v8::String::New(uri.c_str()),
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
  // Store 'module' as a persistent object
  module_ = v8::Persistent<v8::Object>::New(module);
  // Create exceptions
  module_->Set(v8::String::NewSymbol("Exception"),
      Module::Exception::GetTemplate()->GetFunction());
  module_->Set(v8::String::NewSymbol("ErrnoException"),
      Module::ErrnoException::GetTemplate()->GetFunction());
  // Create 'module' object
  context_->Global()->Set(v8::String::NewSymbol("module"), module_);
  initialized_ = true;
  return true;
}

v8::Handle<v8::Value> Module::Print(const v8::Arguments& args) {
  for (int i = 0; i < args.Length(); ++i) {
    if (i != 0) {
      ::fputc(' ', stdout);
    }
    ::fprintf(stdout, "%s", *v8::String::Utf8Value(args[i]->ToString()));
  }
  ::fputc('\n', stdout);
  ::fflush(stdout);
  return v8::Handle<v8::Value>();
}

const char* Module::GetDirectoryName() {
  if (!directory_name_) {
    // Copy the file name for the call to dirname
    char* file_name = static_cast<char*>(
        ::malloc(strlen(file_name_.c_str()) + 1));
    if (!file_name) {
      return NULL;
    }
    strcpy(file_name, file_name_.c_str());
    // Get the directory name
    char* directory_name = ::dirname(file_name);
    if (!directory_name) {
      ::free(file_name);
      return NULL;
    }
    // Copy the directory name to its permanent location
    directory_name_ = static_cast<char*>(::malloc(strlen(directory_name) + 1));
    ::strcpy(directory_name_, directory_name);
    ::free(file_name);
  }
  return directory_name_;
}

v8::Handle<v8::Value> Module::Require(v8::Handle<v8::String> name) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> require = v8::Context::GetCurrent()->Global()->
    Get(v8::String::NewSymbol("require"));
  if (require.IsEmpty()) {
    // Should be unreachable
    return handle_scope.Close(v8::ThrowException(v8::Exception::Error(
          v8::String::New("Require is empty"))));
  }
  if (!require->IsFunction()) {
    // Should be unreachable
    return handle_scope.Close(v8::ThrowException(v8::Exception::Error(
          v8::String::New("Require is not a function"))));
  }
  v8::Handle<v8::Value> argv[1] = { name };
  return handle_scope.Close(
      v8::Function::Cast(*require)->Call(require->ToObject(), 1, argv));
}

v8::Handle<v8::Value> Module::Exports() {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> exports = v8::Context::GetCurrent()->Global()->
    Get(v8::String::NewSymbol("exports"));
  if (exports.IsEmpty()) {
    // Should be unreachable
    return handle_scope.Close(v8::ThrowException(v8::Exception::Error(
          v8::String::New("Exports is empty"))));
  }
  return handle_scope.Close(exports);
}

v8::Handle<v8::Value> Module::ConstructCall(
    v8::Handle<v8::FunctionTemplate> function_templ,
    const v8::Arguments& arguments) {
  int argc = arguments.Length();
  v8::Local<v8::Value>* argv = static_cast<v8::Local<v8::Value>*>(
      ::malloc(argc * sizeof(v8::Local<v8::Value>*)));
  if (!argv) {
    return v8::ThrowException(Module::ErrnoException::New(errno));
  }
  for (int index = 0; index < argc; ++index) {
    argv[index] = arguments[index];
  }
  v8::Local<v8::Object> instance =
    function_templ->GetFunction()->NewInstance(argc, argv);
  ::free(argv);
  return instance;
}

v8::Handle<v8::Value> Module::Exception::New(v8::Handle<v8::Value> message) {
  v8::Handle<v8::Object> object;
  if (message.IsEmpty()) {
    return GetTemplate()->GetFunction()->NewInstance();
  } else {
    v8::Handle<v8::Value> argv[1] = { message };
    return GetTemplate()->GetFunction()->NewInstance(1, argv);
  }
}

v8::Handle<v8::FunctionTemplate> Module::Exception::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("Exception"));
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Module::Exception::New(const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  v8::Handle<v8::Object> self = arguments.This();
  self->Set(v8::String::NewSymbol("name"), v8::String::NewSymbol("Exception"));
  if (arguments.Length()) {
    if (!arguments[0].IsEmpty()) {
      self->Set(v8::String::NewSymbol("message"), arguments[0]);
    }
  }
  return self;
}

v8::Handle<v8::Value> Module::Exception::ToString(
    const v8::Arguments& arguments) {
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
  return v8::String::New(string.c_str());
}

v8::Handle<v8::Value> Module::ErrnoException::New(int error) {
  v8::Handle<v8::Value> argv[1] = { v8::Integer::New(error) };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

v8::Handle<v8::Value> Module::ErrnoException::New(const char* message,
    int error) {
  v8::Handle<v8::Value> argv[2] = {
    v8::String::New(message),
    v8::Integer::New(error)
  };
  return GetTemplate()->GetFunction()->NewInstance(2, argv);
}

v8::Handle<v8::FunctionTemplate> Module::ErrnoException::GetTemplate() {
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->Inherit(Module::Exception::GetTemplate());
  templ->SetClassName(v8::String::NewSymbol("ErrnoException"));
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Module::ErrnoException::New(
    const v8::Arguments& arguments) {
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  v8::Handle<v8::Object> self = arguments.This();
  self->Set(v8::String::NewSymbol("name"),
      v8::String::NewSymbol("ErrnoException"));
  switch (arguments.Length()) {
  case 2:
    if (arguments[0]->IsString()) {
      std::string message(*v8::String::Utf8Value(arguments[0]->ToString()));
      if (arguments[1]->IsInt32()) {
        message.append(": ");
        message.append(::strerror(arguments[1]->ToInteger()->Value()));
        self->Set(v8::String::NewSymbol("errno"), arguments[1]);
      }
      self->Set(v8::String::NewSymbol("message"),
          v8::String::New(message.c_str()));
    } else if (arguments[0]->IsInt32()) {
      self->Set(v8::String::NewSymbol("message"),
          v8::String::New(::strerror(arguments[0]->ToInteger()->Value())));
      self->Set(v8::String::NewSymbol("errno"), arguments[0]);

    } else {
      self->Set(v8::String::NewSymbol("message"), arguments[0]);
    }
    break;
  case 1:
    if (arguments[0]->IsInt32()) {
      self->Set(v8::String::NewSymbol("message"),
          v8::String::New(::strerror(arguments[0]->ToInteger()->Value())));
      self->Set(v8::String::NewSymbol("errno"), arguments[0]);
    } else {
      self->Set(v8::String::NewSymbol("message"), arguments[0]);
    }
    break;
  default:
    break;
  }
  return self;
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
