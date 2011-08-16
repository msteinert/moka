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
#include "dlfcn.h"
#include <libgen.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "v8-commonjs/internal/module.h"

namespace commonjs {

namespace internal {

Module::Module(bool secure, v8::Handle<v8::Object> require,
    v8::Handle<v8::Context> context)
  : secure_(secure)
  , context_owner_(false)
  , context_(context)
  , require_(require)
  , handle_(NULL) {}

Module::Module(bool secure, v8::Handle<v8::Object> require)
  : secure_(secure)
  , context_owner_(true)
  , context_(v8::Context::New())
  , require_(require)
  , handle_(NULL) {}

Module::~Module() {
  exception_.Dispose();
  module_.Dispose();
  exports_.Dispose();
  if (context_owner_) {
    context_.Dispose();
  }
  if (handle_) {
    dlclose(handle_);
  }
}

// XXX ---------------- XXX //
static v8::Handle<v8::Value> Print(const v8::Arguments& args)
{
  for (int i = 0; i < args.Length(); ++i) {
    v8::HandleScope handle_scope;
    if (i != 0) {
      fputc(' ', stdout);
    }
    v8::String::Utf8Value s(args[i]->ToString());
    fwrite(*s, sizeof(**s), s.length(), stdout);
  }
  fputc('\n', stdout);
  fflush(stdout);
  return v8::Undefined();
}
// XXX ---------------- XXX //

bool Module::Initialize(const char* id, const char* file_name)
{
  v8::HandleScope handle_scope;
  // XXX ---------------- XXX //
  v8::Local<v8::FunctionTemplate> print_templ =
    v8::FunctionTemplate::New(Print);
  context_->Global()->Set(v8::String::New("print"), print_templ->GetFunction());
  // XXX ---------------- XXX //
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
  id_.assign(id);
  module->Set(v8::String::NewSymbol("id"), v8::String::New(id),
      static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  char resolved_path[PATH_MAX];
  if (!realpath(file_name, resolved_path)) {
    return false;
  }
  dirname_.assign(dirname(resolved_path));
  if (!secure_) {
    // Create the 'uri' property
    std::string uri("file://");
    uri.append(resolved_path);
    module->Set(v8::String::NewSymbol("uri"), v8::String::New(uri.c_str()),
        static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete));
  }
  // Store 'module' as a persistent object
  module_ = v8::Persistent<v8::Object>::New(module);
  // Create 'module' object
  context_->Global()->Set(v8::String::NewSymbol("module"), module_);
  return true;
}

bool Module::RequireScript(const char* id, const char* path) {
  v8::HandleScope handle_scope;
  std::string full_path(path);
  full_path.append("/");
  full_path.append(id);
  full_path.append(".js");
  FILE* file = fopen(full_path.c_str(), "rb");
  if (!file) {
    return false;
  }
  struct stat buf;
  if (fstat(fileno(file), &buf)) {
    fclose(file);
    return false;
  }
  if (!S_ISREG(buf.st_mode)) {
    fclose(file);
    return false;
  }
  char* characters = new char[buf.st_size + 1];
  if (!characters) {
    fclose(file);
    char error[BUFSIZ];
    strerror_r(errno, error, BUFSIZ);
    v8::Local<v8::Value> exception =
      v8::Exception::Error(v8::String::New(error));
    exception_ = v8::Persistent<v8::Value>::New(exception);
    return false;
  }
  size_t size = fread(characters, 1, buf.st_size, file);
  if (static_cast<off_t>(size) < buf.st_size) {
    if (ferror(file)) {
      fclose(file);
      delete[] characters;
      char error[BUFSIZ];
      strerror_r(errno, error, BUFSIZ);
      v8::Local<v8::Value> exception =
        v8::Exception::Error(v8::String::New(error));
      exception_ = v8::Persistent<v8::Value>::New(exception);
      return false;
    }
    clearerr(file);
  }
  fclose(file);
  v8::Context::Scope scope(context_);
  if (!Initialize(id, full_path.c_str())) {
    v8::Local<v8::Value> exception =
      v8::Exception::Error(v8::String::New("Failed to initialize module"));
    exception_ = v8::Persistent<v8::Value>::New(exception);
    return false;
  }
  v8::Local<v8::String> source = v8::String::New(characters, size);
  delete[] characters;
  v8::TryCatch try_catch;
  v8::Local<v8::Script> script = v8::Script::Compile(source,
      v8::String::New(full_path.c_str()));
  if (script.IsEmpty()) {
    exception_ = v8::Persistent<v8::Value>::New(try_catch.Exception());
    return false;
  }
  v8::Local<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    exception_ = v8::Persistent<v8::Value>::New(try_catch.Exception());
    return false;
  }
  return true;
}

} // namespace internal

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
