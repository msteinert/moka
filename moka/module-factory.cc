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

/// \brief Implements the API found in moka/module-factory.h

#include <cstdlib>
#include <dlfcn.h>
#include "moka/module-factory.h"
#include "moka/script-module.h"
#include "moka/so-module.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace moka {

namespace internal {

ModuleFactory::ModuleFactory(bool secure, v8::Handle<v8::Object> require,
    ModulePointer module, int* argc, char*** argv)
  : secure_(secure)
  , require_(require)
  , argc_(argc)
  , argv_(argv) {
  // Insert the main module into the module store
  modules_.insert(ModulePair(module->GetFileName(), module));
}

ModulePointer ModuleFactory::NewScriptModule(const char* id, const char* path) {
  ModulePointer module;
  // Construct a full script filename
  std::string file_name(path);
  file_name.append("/");
  file_name.append(id);
  file_name.append(".js");
  // Resolve links/relative references
  if (!realpath(file_name.c_str(), resolved_path_)) {
    return module;
  }
  // Check for a previously loaded module
  ModuleMap::iterator iter = modules_.find(resolved_path_);
  if (modules_.end() != iter) {
    return (*iter).second;
  }
  // Open the script
  FILE* file = ::fopen(resolved_path_, "rb");
  if (!file) {
    return module;
  }
  // Get file statistics
  struct stat buf;
  if (::fstat(fileno(file), &buf)) {
    ::fclose(file);
    return module;
  }
  // Ensure this file is a regular file, i.e., not a directory
  if (!S_ISREG(buf.st_mode)) {
    ::fclose(file);
    return module;
  }
  // Create a new script module
  module.reset(new ScriptModule(id, resolved_path_, secure_, require_, file,
        buf.st_size));
  if (!module.get()) {
    ::fclose(file);
    return module;
  }
  // Insert the module into the module store
  modules_.insert(ModulePair(module->GetFileName(), module));
  return module;
}

ModulePointer ModuleFactory::NewSoModule(const char* id, const char* path) {
  ModulePointer module;
  // Construct a shared object name
  std::string file_name(path);
  file_name.append("/");
  file_name.append(id);
  file_name.append(".so");
  // Resolve links/relative references
  if (!realpath(file_name.c_str(), resolved_path_)) {
    return module;
  }
  // Check for a previously loaded module
  ModuleMap::iterator iter = modules_.find(resolved_path_);
  if (modules_.end() != iter) {
    return (*iter).second;
  }
  // Invoke the dynamic linker to open the shared object
  void* handle = ::dlopen(file_name.c_str(), RTLD_LAZY);
  if (!handle) {
    return module;
  }
  // Create a new shared object module
  module.reset(new SoModule(id, resolved_path_, secure_, require_, handle,
        argc_, argv_));
  if (!module.get()) {
    ::dlclose(handle);
    return module;
  }
  // Insert the module into the module store
  modules_.insert(ModulePair(module->GetFileName(), module));
  return module;
}

ModulePointer ModuleFactory::NewModule(const char* id, const char* path) {
  // Try to load a script module
  ModulePointer module = NewScriptModule(id, path);
  if (module.get()) {
    return module;
  }
  // Try to load a shared object module
  module = NewSoModule(id, path);
  if (module.get()) {
    return module;
  }
  return module;
}

} // namespace internal

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
