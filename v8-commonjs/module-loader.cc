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
#include <climits>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "v8-commonjs/module-factory.h"
#include "v8-commonjs/module-loader.h"

namespace commonjs {

ModuleLoader::ModuleLoader()
  : error_("None")
  , initialized_(false)
  , secure_(false) {}

ModuleLoader::ModuleLoader(bool secure)
  : error_("None")
  , initialized_(false)
  , secure_(secure) {}

ModuleLoader::~ModuleLoader() {
  require_.Dispose();
  paths_.Dispose();
}

bool ModuleLoader::Initialize(const char* file_name) {
  return Initialize(file_name, NULL, NULL);
}

static char* NewId(const char* file_name) {
  char* file_name_copy = static_cast<char*>(::malloc(strlen(file_name) + 1));
  if (!file_name_copy) {
    return NULL;
  }
  strcpy(file_name_copy, file_name);
  char* base_name = ::basename(file_name_copy);
  if (!base_name) {
    ::free(file_name_copy);
    return NULL;
  }
  char* dot = strchr(base_name, '.');
  if (dot) {
    *dot = '\0';
  }
  char *base_name_copy = static_cast<char*>(::malloc(strlen(base_name) + 1));
  if (!base_name_copy) {
    ::free(file_name_copy);
    return NULL;
  }
  strcpy(base_name_copy, base_name);
  ::free(file_name_copy);
  return base_name_copy;
}

bool ModuleLoader::Initialize(const char* file_name, int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  if (initialized_) {
    return true;
  }
  context_ = v8::Context::GetEntered();
  if (context_.IsEmpty()) {
    error_.assign("No currently entered context");
    return false;
  }
  // Create 'require' object
  v8::Handle<v8::ObjectTemplate> require_templ = v8::ObjectTemplate::New();
  require_templ->SetInternalFieldCount(1);
  require_templ->SetCallAsFunctionHandler(Require);
  // Create 'paths' object
  v8::Local<v8::Array> paths = v8::Array::New();
  uint32_t index = 0;
  if (!secure_) {
    // Add directories from environment
    const char* env = ::getenv("COMMONJSPATH");
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
    env = ::getenv("HOME");
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
  char resolved_path[PATH_MAX];
  if (!realpath(file_name, resolved_path)) {
    char error[BUFSIZ];
    ::strerror_r(errno, error, BUFSIZ);
    error_.assign(error);
    return false;
  }
  ModulePointer module;
  char* id = NewId(resolved_path);
  if (id) {
    module.reset(new Module(id, resolved_path, secure_, require_,
          context_));
    ::free(id);
  } else {
    module.reset(new Module(".", resolved_path, secure_, require_,
          context_));
  }
  if (!module.get()) {
    error_.assign("No memory");
    return false;
  }
  if (!module->Initialize()) {
    error_.assign("Failed to initialize main module");
    return false;
  }
  module_stack_.push(module);
  module_factory_.reset(new internal::ModuleFactory(secure_, require_, module,
        argc, argv));
  if (!module_factory_.get()) {
    error_.assign("No Memory");
    return false;
  }
  // Create 'main' object
  require_->Set(v8::String::NewSymbol("main"), module->GetModule(),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  initialized_ = true;
  return true;
}

v8::Handle<v8::Value> ModuleLoader::Require(const v8::Arguments& arguments) {
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
  ModuleLoader* module_loader = static_cast<ModuleLoader*>(external->Value());
  if (!module_loader->initialized_) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Module loader is not initialized")));
  }
  ModulePointer module;
  if ('.' != id[0]) {
    v8::Local<v8::Array> properties = module_loader->paths_->GetPropertyNames();
    if (!properties.IsEmpty()) {
      uint32_t index = 0, length = properties->Length();
      while ((index < length) && !module.get()) {
        v8::Local<v8::Value> index_value = properties->Get(index++);
        if (index_value->IsUint32()) {
          v8::Local<v8::Value> path_value =
            module_loader->paths_->Get(index_value->Uint32Value());
          if (!path_value.IsEmpty()) {
            if (path_value->IsString()) {
              module = module_loader->module_factory_->NewModule(id.c_str(),
                  *v8::String::Utf8Value(path_value));
            }
          }
        }
      }
    }
  } else {
    // Relative load
    if (!module_loader->secure_) {
      ModulePointer previous_module = module_loader->module_stack_.top();
      if (!previous_module.get()) {
        return handle_scope.Close(v8::ThrowException(
              v8::String::New("Internal module loader error")));
      }
      const char* directory_name = previous_module->GetDirectoryName();
      if (!directory_name) {
        return handle_scope.Close(v8::ThrowException(
              v8::String::New("No memory")));
      }
      module = module_loader->module_factory_->NewModule(id.c_str(),
          directory_name);

    }
  }
  if (module.get()) {
    module_loader->module_stack_.push(module);
    bool status = module->Load();
    module_loader->module_stack_.pop();
    if (status) {
      return handle_scope.Close(module->GetExports());
    } else {
      module_loader->module_factory_->RemoveModule(module);
      v8::Handle<v8::Value> exception = module->GetException();
      if (exception.IsEmpty()) {
        std::string error("Failed to load module ");
        error.append(id);
        return handle_scope.Close(v8::ThrowException(
              v8::String::New(error.c_str())));
      } else {
        return handle_scope.Close(v8::ThrowException(exception));
      }
    }
  }
  std::string error("No module named ");
  error.append(id);
  return handle_scope.Close(v8::ThrowException(v8::String::New(error.c_str())));
}

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
