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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libgen.h>
#include "v8-commonjs/module.h"

namespace commonjs {

namespace internal {

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
    directory_name_ = NULL;
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
  // Create 'module' object
  context_->Global()->Set(v8::String::NewSymbol("module"), module_);
  initialized_ = true;
  return true;
}

v8::Handle<v8::Value> Module::Print(const v8::Arguments& args) {
  for (int i = 0; i < args.Length(); ++i) {
    v8::HandleScope handle_scope;
    if (i != 0) {
      ::fputc(' ', stdout);
    }
    v8::String::Utf8Value s(args[i]->ToString());
    ::fwrite(*s, sizeof(**s), s.length(), stdout);
  }
  ::fputc('\n', stdout);
  ::fflush(stdout);
  return v8::Undefined();
}

const char* Module::GetDirectoryName() {
  if (!directory_name_) {
    char* file_name = static_cast<char*>(
        ::malloc(strlen(file_name_.c_str()) + 1));
    if (!file_name) {
      return NULL;
    }
    strcpy(file_name, file_name_.c_str());
    char* directory_name = ::dirname(file_name);
    if (!directory_name) {
      ::free(file_name);
      return NULL;
    }
    directory_name_ = static_cast<char*>(::malloc(strlen(directory_name) + 1));
    ::strcpy(directory_name_, directory_name);
    ::free(file_name);
  }
  return directory_name_;
}

} // namespace internal

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
