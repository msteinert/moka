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

#ifndef V8_COMMONJS_MODULE_FACTORY_H
#define V8_COMMONJS_MODULE_FACTORY_H

#include <climits>
#include <map>
#include <tr1/memory>
#include "v8-commonjs/script-module.h"

namespace commonjs {

namespace internal {

class ModuleFactory;

typedef std::tr1::shared_ptr<Module> ModulePointer;

typedef std::pair<std::string, ModulePointer> ModulePair;

typedef std::map<std::string, ModulePointer> ModuleMap;

} // namespace internal

} // namespace commonjs

/**
 * A CommonJS 1.1 module factory
 */
class commonjs::internal::ModuleFactory {
public:
  ModuleFactory(bool secure, v8::Handle<v8::Object> require,
      ModulePointer module);

  ~ModuleFactory() {}

  ModulePointer NewModule(const char* id, const char* path,
      int* argc_, char*** argv_);

  ModulePointer NewScriptModule(const char* id, const char* path);

private: // non-copyable/instantiable
  ModuleFactory(Module const& that);

  void operator=(Module const& that);

private: // private data
  bool secure_;
  v8::Persistent<v8::Object> require_;
  ModuleMap modules_;
  char resolved_path_[PATH_MAX];
};

#endif // V8_COMMONJS_MODULE_FACTORY_H

// vim: tabstop=2:sw=2:expandtab
