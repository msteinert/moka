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

#include "v8-commonjs/internal/module.h"
#include "v8-commonjs/module.h"
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace commonjs {

Module::Module()
  : error_("None")
  , initialized_(false)
  , secure_(false) {}

Module::Module(bool secure)
  : error_("None")
  , initialized_(false)
  , secure_(secure) {}

Module::~Module() {
  require_.Dispose();
  paths_.Dispose();
}

bool Module::Initialize(const char* id) {
  return Initialize(id, NULL, NULL);
}

bool Module::Initialize(const char* file_name, int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  if (initialized_) {
    error_.assign("Already initialized");
    return false;
  }
  context_ = v8::Context::GetEntered();
  if (context_.IsEmpty()) {
    error_.assign("No currently entered context");
    return false;
  }
  argc_ = argc;
  argv_ = argv;
  // Create 'require' object
  v8::Handle<v8::ObjectTemplate> require_templ = v8::ObjectTemplate::New();
  require_templ->SetInternalFieldCount(1);
  require_templ->SetCallAsFunctionHandler(Require);
  // Create 'paths' object
  v8::Local<v8::Array> paths = v8::Array::New();
  uint32_t index = 0;
  if (!secure_) {
    // Add directories from environment
    const char* env = getenv("COMMONJSPATH");
    if (env) {
      std::string path;
      std::stringstream commonjspath(env);
      while (std::getline(commonjspath, path, ':')) {
        paths->Set(index++, v8::String::New(path.c_str()));
      }
    }
    // Add current working directory
    paths->Set(index++, v8::String::New("."));
    // Add $HOME/lib/commonjs
    env = getenv("HOME");
    if (env) {
      std::string home(env);
      home.append("/lib/commonjs");
      paths->Set(index++, v8::String::New(home.c_str()));
    }
  }
  // Add $libdir/commonjs
  paths->Set(index++, v8::String::New(LIBDIR "/commonjs"));
  // Store 'paths' as a persistent object
  paths_ = v8::Persistent<v8::Array>::New(paths);
  if (!secure_) {
    require_templ->Set(v8::String::NewSymbol("paths"), paths_,
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
  // Export 'require' object
  v8::Local<v8::Object> require = require_templ->NewInstance();
  require->SetInternalField(0, v8::External::New(this));
  require_ = v8::Persistent<v8::Object>::New(require);
  // Create 'main' module
  ModulePointer module(new internal::Module(secure_, require_, context_));
  if (!module.get()) {
    error_.assign("Out of memory");
    return false;
  }
  std::string id(basename(file_name));
  size_t dot = id.find_first_of(".");
  if (std::string::npos != dot) {
    id.erase(dot);
  }
  if (!module->Initialize(id.c_str(), file_name)) {
    error_.assign("Failed to initialize main module");
    return false;
  }
  modules_.insert(ModulePair(id, module));
  // Create 'main' object
  require_->Set(v8::String::NewSymbol("main"), module->GetModule(),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  initialized_ = true;
  return true;
}

v8::Handle<v8::Value> Module::Require(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (arguments.Length() != 1) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("A single argument is required")));
  }
  if (!arguments[0]->IsString()) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Argument one must be a string")));
  }
  std::string id(*v8::String::Utf8Value(arguments[0]));
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Module* module = static_cast<Module*>(external->Value());
  if (!module->initialized_) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Module loader is not initialized")));
  }
  ModuleMap::iterator iter = module->modules_.find(id);
  if (module->modules_.end() != iter) {
    return (*iter).second->GetExports();
  }
  v8::Local<v8::Array> properties = module->paths_->GetPropertyNames();
  if (properties.IsEmpty()) {
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  uint32_t index = 0;
  if (id[0] != '.') {
    ModulePointer internal_module(
        new internal::Module(module->secure_, module->require_));
    if (!internal_module.get()) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Out of memory")));
    }
    module->modules_.insert(ModulePair(id, internal_module));
    module->current_ = internal_module;
    while (index < properties->Length()) {
      v8::Local<v8::Value> index_value = properties->Get(index++);
      if (!index_value->IsUint32()) {
        module->current_.reset();
        module->modules_.erase(id);
        return handle_scope.Close(v8::ThrowException(
              v8::String::New("Index error")));
      }
      v8::Local<v8::Value> path_value =
        module->paths_->Get(index_value->Uint32Value());
      if (path_value.IsEmpty()) {
        module->current_.reset();
        module->modules_.erase(id);
        return handle_scope.Close(v8::ThrowException(
              v8::String::New("Index error")));
      }
      if (path_value->IsString()) {
        bool success = internal_module->RequireScript(id.c_str(),
            *v8::String::Utf8Value(path_value));
        if (success) {
          module->current_.reset();
          return internal_module->GetExports();
        } else {
          v8::Handle<v8::Value> exception = internal_module->Exception();
          if (!exception.IsEmpty()) {
            module->current_.reset();
            module->modules_.erase(id);
            return exception;
          }
        }
      }
    }
    module->modules_.erase(id);
  } else {
    // relative
    if (module->current_.get()) {
      std::string file_name(module->current_->GetDirname());
      file_name.append("/");
      file_name.append(id);
      file_name.append(".js");
      char resolved_path[PATH_MAX];
      if (!realpath(file_name.c_str(), resolved_path)) {
        char error[BUFSIZ];
        strerror_r(errno, error, BUFSIZ);
        module->current_.reset();
        return handle_scope.Close(v8::ThrowException(v8::String::New(error)));
      }
      char* slash = strrchr(resolved_path, '/');
      *slash = '\0';
      ModulePointer internal_module(
          new internal::Module(module->secure_, module->require_));
      if (!internal_module.get()) {
        return handle_scope.Close(v8::ThrowException(
              v8::String::New("Out of memory")));
      }
      slash = strrchr(id.c_str(), '/');
      ++slash;
      module->modules_.insert(ModulePair(slash, internal_module));
      bool success = internal_module->RequireScript(slash, resolved_path);
      if (success) {
        module->current_.reset();
        return internal_module->GetExports();
      } else {
        v8::Handle<v8::Value> exception = internal_module->Exception();
        if (!exception.IsEmpty()) {
          module->current_.reset();
          module->modules_.erase(slash);
          return exception;
        }
      }
      module->modules_.erase(slash);
    }
  }
  module->current_.reset();
  std::string error("No module named ");
  error.append(id);
  return handle_scope.Close(v8::ThrowException(v8::String::New(error.c_str())));
}

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
