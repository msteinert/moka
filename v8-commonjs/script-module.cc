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

/// \brief Implements the API found in v8-commonjs/script-module.h

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <v8-commonjs/script-module.h>

namespace commonjs {

namespace internal {

ScriptModule::ScriptModule(const char* id, const char* file_name, bool secure,
    v8::Handle<v8::Object> require, FILE* file, size_t size)
  : Module(id, file_name, secure, require)
  , file_(file)
  , size_(size)
  , loaded_(false) {}

ScriptModule::~ScriptModule() {
  if (file_) {
    ::fclose(file_);
  }
}

bool ScriptModule::Load() {
  v8::HandleScope handle_scope;
  if (loaded_) {
    // Already loaded
    return true;
  }
  if (!Initialize()) {
    // Initialize the base module
    std::string message("Failed to initialize module ");
    message.append(GetId());
    SetException(message);
    return false;
  }
  // Reset file pointer
  ::rewind(file_);
  // Create a buffer to read the script into
  char* characters = static_cast<char*>(::malloc(size_ + 1));
  if (!characters) {
    char error[BUFSIZ];
    ::strerror_r(errno, error, BUFSIZ);
    std::string message("Loading module ");
    message.append(GetId());
    message.append(": ");
    message.append(error);
    SetException(message);
    return false;
  }
  // Read the script into the buffer
  size_t size = ::fread(characters, 1, size_, file_);
  if (static_cast<off_t>(size) < size_) {
    if (ferror(file_)) {
      ::free(characters);
      char error[BUFSIZ];
      ::strerror_r(errno, error, BUFSIZ);
      std::string message("Loading module ");
      message.append(GetId());
      message.append(": ");
      message.append(error);
      SetException(message);
      return false;
    }
    ::clearerr(file_);
  }
  ::fclose(file_);
  file_ = NULL;
  // Enter the context for this module
  v8::Context::Scope scope(GetContext());
  // Create a script object from the source buffer
  v8::Local<v8::String> source = v8::String::New(characters, size);
  ::free(characters);
  v8::TryCatch try_catch;
  // Compile the script
  v8::Local<v8::Script> script = v8::Script::Compile(source,
      v8::String::New(GetFileName()));
  if (script.IsEmpty()) {
    SetException(handle_scope.Close(try_catch.ReThrow()));
    return false;
  }
  // Run the script
  v8::Local<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    SetException(handle_scope.Close(try_catch.ReThrow()));
    return false;
  }
  loaded_ = true;
  return true;
}

} // namespace internal

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
