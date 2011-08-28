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

/**
 * \brief The module factory is used by the module loader to load new modules
 */

#ifndef MOKA_MODULE_FACTORY_H
#define MOKA_MODULE_FACTORY_H

#include <climits>
#include <map>
#include "moka/script-module.h"
#include <tr1/memory>

namespace moka {

namespace internal {

class ModuleFactory;

typedef std::tr1::shared_ptr<Module> ModulePointer;

typedef std::pair<std::string, ModulePointer> ModulePair;

typedef std::map<std::string, ModulePointer> ModuleMap;

} // namespace internal

} // namespace moka

/// \brief The Moka module factory
class moka::internal::ModuleFactory {
public:
  /**
   * \brief Construct a module factory
   *
   * \param secure [in] Indicates if this should be a secure module factory
   * \param require [in] The object implementing the 'require' function
   * \param module [in] The main module object
   * \param argc [in] A pointer to the application argument count
   * \param argc [in] A pointer to the application argument vector
   */
  ModuleFactory(bool secure, v8::Handle<v8::Object> require,
      ModulePointer module, int* argc, char*** argv);

  /// \brief Default destructor
  ~ModuleFactory() {}

  /**
   * \brief Construct a new module
   *
   * \param id [in] The ID of the module to be constructed
   * \param path [in] The path to search in for the module
   *
   * \return If the module was found, this function returns a pointer
   *         to the new module. If the module was not found NULL is
   *         returned. The return value can be checked with
   *         std::tr1::shared_ptr::get().
   */
  ModulePointer NewModule(const char* id, const char* path);

  /**
   * \brief Construct a new module from JavaScript
   *
   * \param id [in] The ID of the module to be constructed
   * \param path [in] The path to search in for the module
   *
   * \return If the module was found, this function returns a pointer
   *         to the new module. If the module was not found NULL is
   *         returned. The return value can be checked with
   *         std::tr1::shared_ptr::get().
   */
  ModulePointer NewScriptModule(const char* id, const char* path);

  /**
   * \brief Construct a new module from a shared library
   *
   * \param id [in] The ID of the module to be constructed
   * \param path [in] The path to search in for the module
   *
   * \return If the module was found, this function returns a pointer
   *         to the new module. If the module was not found NULL is
   *         returned. The return value can be checked with
   *         std::tr1::shared_ptr::get().
   */
  ModulePointer NewSoModule(const char* id, const char* path);

  /**
   * \brief Remove a module from the module store
   *
   * Modules are cached by the module factory when they are created. If a
   * module fails to load after it has been created it can be removed from
   * the store by calling this function.
   *
   * \param module [in] The module to be removed from the module store
   */
  void RemoveModule(ModulePointer module) {
    if (!module.get()) {
      return;
    }
    ModuleMap::iterator iter = modules_.find(module->GetFileName());
    if (modules_.end() != iter) {
      modules_.erase(iter);
    }
  }

private: // non-copyable/instantiable
  ModuleFactory(Module const& that);

  void operator=(Module const& that);

private: // private data
  bool secure_;
  v8::Persistent<v8::Object> require_;
  int* argc_;
  char*** argv_;
  ModuleMap modules_;
  char resolved_path_[PATH_MAX];
};

#endif // MOKA_MODULE_FACTORY_H

// vim: tabstop=2:sw=2:expandtab
