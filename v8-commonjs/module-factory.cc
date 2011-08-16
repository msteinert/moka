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

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "v8-commonjs/module-factory.h"
#include "v8-commonjs/script-module.h"

namespace commonjs {

namespace internal {

ModuleFactory::ModuleFactory(bool secure, v8::Handle<v8::Object> require,
    ModulePointer module)
  : secure_(secure)
  , require_(require) {
  modules_.insert(ModulePair(module->GetFileName(), module));
}

ModulePointer ModuleFactory::NewScriptModule(const char* id, const char* path) {
  ModulePointer module;
  std::string file_name(path);
  file_name.append("/");
  file_name.append(id);
  file_name.append(".js");
  if (!realpath(file_name.c_str(), resolved_path_)) {
    return module;
  }
  ModuleMap::iterator iter = modules_.find(resolved_path_);
  if (modules_.end() != iter) {
    return (*iter).second;
  }
  FILE* file = ::fopen(resolved_path_, "rb");
  if (!file) {
    return module;
  }
  struct stat buf;
  if (::fstat(fileno(file), &buf)) {
    ::fclose(file);
    return module;
  }
  if (!S_ISREG(buf.st_mode)) {
    ::fclose(file);
    return module;
  }
  module.reset(new ScriptModule(id, resolved_path_, secure_, require_, file,
        buf.st_size));
  modules_.insert(ModulePair(module->GetFileName(), module));
  return module;
}

ModulePointer ModuleFactory::NewModule(const char* id, const char* path,
    int* /* argc */, char*** /* argv */) {
  ModulePointer module = NewScriptModule(id, path);
  if (module.get()) {
    return module;
  }
  return module;
}

} // namespace internal

} // namespace commonjs

// vim: tabstop=2:sw=2:expandtab
