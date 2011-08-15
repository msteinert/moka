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

#include "v8-commonjs/module.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace commonjs {

namespace internal {

class Module {
public:
  Module(v8::Persistent<v8::Object> exports, void* handle = NULL)
    : exports_(exports)
    , handle_(handle) {}

  ~Module() {
    exports_.Dispose();
    if (handle_) {
      dlclose(handle_);
    }
  }

  v8::Persistent<v8::Object> GetExports() {
    return exports_;
  }

private: // private data
  v8::Persistent<v8::Object> exports_;
  void* handle_;
  Module(Module const& that);
  void operator=(Module const& that);
};

} // namespace internal

Module::Module()
  : error_("None")
  , initialized_(false)
  , secure_(false) {}

Module::Module(bool secure)
  : error_("None")
  , initialized_(false)
  , secure_(secure) {}

Module::~Module() {
  paths_.Dispose();
  for (ModuleMap::iterator iter = modules_.begin(); iter != modules_.end();
      ++iter) {
    delete (*iter).second;
  }
  while (!exports_.empty()) {
    exports_.top().Dispose();
    exports_.pop();
  }
}

bool Module::Initialize(const char* id) {
  return Initialize(id, NULL, NULL);
}

bool Module::Initialize(const char* id, int* argc, char*** argv) {
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
  InitializeRequire();
  InitializeExports();
  InitializeModule(".", id);
  InitializeMain();
  initialized_ = true;
  return true;
}

void Module::InitializeRequire() {
  v8::HandleScope handle_scope;
  // Create 'require' object
  v8::Handle<v8::ObjectTemplate> require_templ = v8::ObjectTemplate::New();
  require_templ->SetInternalFieldCount(1);
  require_templ->SetCallAsFunctionHandler(Require);
  // Create 'paths' object
  InitializePaths(require_templ);
  // Export 'require' object
  v8::Local<v8::Object> require = require_templ->NewInstance();
  require->SetInternalField(0, v8::External::New(this));
  context_->Global()->Set(v8::String::NewSymbol("require"), require);
}

void Module::InitializeExports() {
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> exports = v8::Object::New();
  // Store 'paths' as a persistent object
  exports_.push(v8::Persistent<v8::Object>::New(exports));
  // Create 'exports' object
  context_->Global()->Set(v8::String::NewSymbol("exports"), exports_.top());
}

void Module::InitializeModule(const std::string& id, const std::string& uri)
{
  v8::HandleScope handle_scope;
  v8::Local<v8::Object> module = v8::Object::New();
  // Create the 'id' property
  module->Set(v8::String::NewSymbol("id"), v8::String::New(id.c_str()),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  // Create the 'uri' property
  module->Set(v8::String::NewSymbol("uri"), v8::String::New(uri.c_str()),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  // Store 'module' as a persistent object
  module_.push(v8::Persistent<v8::Object>::New(module));
  // Create 'module' object
  context_->Global()->Set(v8::String::NewSymbol("module"), module_.top());
}

void Module::InitializeMain()
{
  v8::HandleScope handle_scope;
  // Create 'main' object
  context_->Global()->Set(v8::String::NewSymbol("main"), module_.top());
}

void Module::InitializePaths(v8::Handle<v8::ObjectTemplate> templ) {
  v8::HandleScope handle_scope;
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
    templ->Set(v8::String::NewSymbol("paths"), paths_,
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
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
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  Module* module = static_cast<Module*>(external->Value());
  if (!module->initialized_) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("Module loader is not initialized")));
  }
  std::string name(*v8::String::Utf8Value(arguments[0]));
  ModuleMap::iterator iter = module->modules_.find(name);
  if (module->modules_.end() != iter) {
    return (*iter).second->GetExports();
  }
  v8::Local<v8::Array> properties = module->paths_->GetPropertyNames();
  if (properties.IsEmpty()) {
    return handle_scope.Close(v8::Undefined());
  }
  uint32_t index = 0;
  while (index < properties->Length()) {
    v8::Local<v8::Value> index_value = properties->Get(index++);
    if (!index_value->IsUint32()) {
      module->exports_.top().Dispose();
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Index error")));
    }
    v8::Local<v8::Value> path_value =
      module->paths_->Get(index_value->Uint32Value());
    if (path_value.IsEmpty()) {
      module->exports_.top().Dispose();
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("Index error")));
    }
    if (path_value->IsString()) {
      std::string path(*v8::String::Utf8Value(path_value));
      v8::Handle<v8::Value> object = module->RequireScript(name, path);
      if (!object.IsEmpty()) {
        return handle_scope.Close(object);
      }
#if 0
      object = module->RequireSharedObject(name, path);
      if (!object.IsEmpty()) {
        return handle_scope.Close(object);
      }
#endif
#if 0
      object = module->RequireModule(name, path);
      if (!object.IsEmpty()) {
        return handle_scope.Close(object);
      }
#endif
    }
  }
  std::string error("No module named ");
  error.append(*v8::String::Utf8Value(arguments[0]));
  return handle_scope.Close(v8::ThrowException(v8::String::New(error.c_str())));
}

v8::Handle<v8::Value> Module::RequireScript(const std::string& name,
    const std::string& path) {
  v8::HandleScope handle_scope;
  std::string full_path(path + '/' + name + ".js");
  FILE* file = fopen(full_path.c_str(), "rb");
  if (!file) {
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  struct stat buf;
  if (fstat(fileno(file), &buf)) {
    fclose(file);
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  if (!S_ISREG(buf.st_mode)) {
    fclose(file);
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  char* characters = new char[buf.st_size + 1];
  if (!characters) {
    fclose(file);
    char error[BUFSIZ];
    strerror_r(errno, error, BUFSIZ);
    return handle_scope.Close(v8::ThrowException(v8::String::New(error)));
  }
  size_t size = fread(characters, 1, buf.st_size, file);
  if (static_cast<off_t>(size) < buf.st_size) {
    if (ferror(file)) {
      fclose(file);
      delete[] characters;
      char error[BUFSIZ];
      strerror_r(errno, error, BUFSIZ);
      return handle_scope.Close(v8::ThrowException(v8::String::New(error)));
    }
    clearerr(file);
  }
  fclose(file);
  v8::Local<v8::String> source = v8::String::New(characters, size);
  delete[] characters;
  if (source.IsEmpty()) {
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  InitializeExports();
  v8::TryCatch try_catch;
  v8::Local<v8::Script> script = v8::Script::Compile(source,
      v8::String::New(full_path.c_str()));
  if (script.IsEmpty()) {
    exports_.top().Dispose();
    PopExports();
    return handle_scope.Close(try_catch.Exception());
  }
  internal::Module* module = new internal::Module(exports_.top());
  modules_.insert(std::pair<std::string, internal::Module*>(name, module));
  v8::Local<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    modules_.erase(name);
    delete module;
    PopExports();
    return handle_scope.Close(try_catch.Exception());
  }
  PopExports();
  return module->GetExports();
}

#if 0
v8::Handle<v8::Value> Module::RequireSharedObject(const std::string& name,
    const std::string& path) {
  v8::HandleScope handle_scope;
     string full_path(string(path) + '/' + name + ".so");
     void* handle = dlopen(full_path.c_str(), RTLD_LAZY);
     if (!handle) {
     Handle<Value> value;
     return handle_scope.Close(Handle<Value>());
     }
     string module_name(string(name) + "_module");
     struct module* init = static_cast<struct module*>(dlsym(handle,
     module_name.c_str()));
     if (!init) {
     dlclose(handle);
     return handle_scope.Close(Handle<Value>());
     }
     if (!init->initialize) {
     dlclose(handle);
     return handle_scope.Close(Handle<Value>());
     }
     if (init->version_major != COMMONJS_MODULE_VERSION_MAJOR) {
     dlclose(handle);
     return handle_scope.Close(Handle<Value>());
     }
     Handle<Object> object = init->initialize(argc_, argv_);
     if (object.IsEmpty()) {
     dlclose(handle);
     return handle_scope.Close(Handle<Value>());
     }
     init->object = Persistent<Object>::New(object);
     init->handle = handle;
     modules_.insert(pair<string, struct module*>(string(name), init));
     return init->object;
  return handle_scope.Close(v8::Handle<v8::Value>());
}
#endif

#if 0
v8::Handle<v8::Value> Module::RequireModule(const std::string& name,
    const std::string& path) {
  v8::HandleScope handle_scope;
  return handle_scope.Close(v8::Handle<v8::Value>());
}
#endif

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
