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
#include "v8-commonjs/system.h"

// Needed to enumerate the environment
extern char** environ;

namespace commonjs {

// Stdin
bool Stdin::Initialize(Module& /* module */) {
  v8::HandleScope handle_scope;
  return true;
}

// Stdout
bool Stdout::Initialize(Module& /* module */) {
  v8::HandleScope handle_scope;
  return true;
}

// Stderr
bool Stderr::Initialize(Module& /* module */) {
  v8::HandleScope handle_scope;
  return true;
}

// Env
static v8::Handle<v8::Value> EnvGet(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  const char* value = ::getenv(*v8::String::Utf8Value(property));
  if (value) {
    return handle_scope.Close(v8::String::New(value));
  }
  return handle_scope.Close(v8::Local<v8::Value>());
}

static v8::Handle<v8::Value> EnvSet(v8::Local<v8::String> property,
    v8::Local<v8::Value> value, const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (-1 == ::setenv(*v8::String::Utf8Value(property),
        *v8::String::Utf8Value(value), 1)) {
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  return handle_scope.Close(value);
}

static v8::Handle<v8::Integer> EnvQuery(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (::getenv(*v8::String::Utf8Value(property))) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

static v8::Handle<v8::Boolean> EnvDelete(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (::getenv(*v8::String::Utf8Value(property))) {
    if (-1 != ::unsetenv(*v8::String::Utf8Value(property))) {
      return handle_scope.Close(v8::True());
    }
  }
  return handle_scope.Close(v8::False());
}

v8::Handle<v8::Array> EnvEnumerate(const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  int size = 0;
  while (environ[++size])
    ;
  v8::Local<v8::Array> env = v8::Array::New(size);
  for (int index = 0; index < size; ++index) {
    const char* equal = strchr(environ[index], '=');
    if (equal) {
      env->Set(index, v8::String::New(environ[index], equal - environ[index]));
    } else {
      env->Set(index, v8::String::New(environ[index]));
    }
  }
  return handle_scope.Close(env);
}

bool Env::Initialize(Module& module) {
  v8::HandleScope handle_scope;
  v8::Local<v8::ObjectTemplate> env_templ = v8::ObjectTemplate::New();
  env_templ->SetNamedPropertyHandler(EnvGet, EnvSet, EnvQuery, EnvDelete,
      EnvEnumerate);
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("env"), env_templ->NewInstance());
  return true;
}

// Args
bool Args::Initialize(Module& module, int argc, char** argv) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Array> args = v8::Array::New(argc);
  for (int index = 0; index < argc; ++index) {
    args->Set(index, v8::String::New(argv[index]));
  }
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("args"), args);
  return true;
}

static bool system_initialize(Module& module, int* argc, char*** argv)
{
  v8::HandleScope handle_scope;
  if (!Stdin::Initialize(module)) {
    return false;
  }
  if (!Stdout::Initialize(module)) {
    return false;
  }
  if (!Stderr::Initialize(module)) {
    return false;
  }
  if (!Env::Initialize(module)) {
    return false;
  }
  if (!Args::Initialize(module, argc ? *argc : 0, argv ? *argv : NULL)) {
    return false;
  }
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::system_initialize)

// vim: tabstop=2:sw=2:expandtab
