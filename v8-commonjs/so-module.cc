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

#include <dlfcn.h>
#include <v8-commonjs/so-module.h>

namespace commonjs {

namespace internal {

SoModule::SoModule(const char* id, const char* file_name, bool secure,
    v8::Handle<v8::Object> require, void* handle, int* argc, char*** argv)
  : Module(id, file_name, secure, require)
  , handle_(handle)
  , argc_(argc)
  , argv_(argv)
  , loaded_(false) {}

SoModule::~SoModule() {
  if (handle_) {
    ::dlclose(handle_);
  }
}

bool SoModule::Load() {
  v8::HandleScope handle_scope;
  if (!Initialize()) {
    SetException("Module initialization failed");
    return false;
  }
  if (loaded_) {
    return true;
  }
  if (!handle_) {
    SetException("Shared object handle is NULL");
    return false;
  }
  const struct module* init =
    static_cast<const struct module*>(::dlsym(handle_, "commonjs_initialize"));
  if (!init) {
    SetException(dlerror());
    return false;
  }
  if (init->version_major != COMMONJS_MODULE_VERSION_MAJOR) {
    SetException("Module major version must match");
    return false;
  }
  if (init->version_minor < COMMONJS_MODULE_VERSION_MINOR) {
    SetException("Module minor version not supported");
    return false;
  }
  v8::Context::Scope scope(GetContext());
  if (!init->initialize) {
    SetException("Module initialization function is NULL");
    return false;
  }
  if (!init->initialize(*this, argc_, argv_)) {
    v8::Handle<v8::Value> exception = GetException();
    if (exception.IsEmpty()) {
      SetException("Module initialization failed");
    }
    return false;
  }
  return true;
}

} // namespace internal

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
