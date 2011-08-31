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

/// \brief Implements the API found in moka/so-module.h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <dlfcn.h>
#include <moka/so-module.h>

namespace moka {

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

v8::Handle<v8::Value> SoModule::Load() {
  if (loaded_) {
    // Already loaded
    return GetExports();
  }
  if (!Initialize()) {
    // Initialize the base module
    std::string message("Failed to initialize module ");
    message.append(GetId());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Verify the shared object handle is valid
  if (!handle_) {
    std::string message("Shared object handle is NULL for module ");
    message.append(GetId());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Resolve the symbol for the initialization structure
  const struct module* init =
    static_cast<const struct module*>(::dlsym(handle_, "moka_module__"));
  if (!init) {
    std::string message("Loading module ");
    message.append(GetId());
    message.append(": ");
    message.append(dlerror());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Check the major version number, must be equal
  if (init->version_major != MOKA_MODULE_VERSION_MAJOR) {
    std::string message("Major version must match for module ");
    message.append(GetId());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Check the minor version number, must be equal or greater
  if (init->version_minor < MOKA_MODULE_VERSION_MINOR) {
    std::string message("Minor version not supported for module ");
    message.append(GetId());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Enter the context for this module
  v8::Context::Scope scope(GetContext());
  if (!init->initialize) {
    std::string message("Initialize function is NULL for module ");
    message.append(GetId());
    return v8::ThrowException(v8::String::New(message.c_str()));
  }
  // Call the module initialization function passing a reference to this module
  v8::Handle<v8::Value> exports = init->initialize(argc_, argv_);
  if (exports.IsEmpty()) {
    std::string message("Module ");
    message.append(GetId());
    message.append(" returned an empty value");
    return v8::ThrowException(v8::String::New(message.c_str()));
  } else if (exports->IsUndefined()) {
    return exports;
  }
  loaded_ = true;
  return exports;
}

} // namespace internal

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
