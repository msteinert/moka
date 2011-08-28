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
 * \file
 * \brief Interface to the module loader
 */

#ifndef MOKA_MODULE_LOADER_H
#define MOKA_MODULE_LOADER_H

#include <moka/macros.h>
#include <stack>
#include <string>
#include <tr1/memory>
#include <v8.h>

namespace moka {

class ModuleLoader;
class Module;

namespace internal {

class ModuleFactory;

} // namespace internal

typedef std::tr1::shared_ptr<Module> ModulePointer;

typedef std::tr1::shared_ptr<internal::ModuleFactory> ModuleFactoryPointer;

typedef std::stack<ModulePointer> ModuleStack;

} // namespace moka

/// A CommonJS 1.1 module loader
class MOKA_EXPORT moka::ModuleLoader {
public:
  /**
   * \brief Construct a new module loader
   */
  ModuleLoader();

  /**
   * \brief Construct a new module loader
   *
   * \param secure [in] Indicates if this should be a secure module loader.
   */
  ModuleLoader(bool secure);

  /// \brief Default destructor
  ~ModuleLoader();

  /**
   * \brief Get an error message after an initialization failure.
   *
   * If the the initialization function fails, call this function to get
   * more detail about the failure.
   *
   * \return A detailed error message.
   */
  const char* GetError() const {
    return error_.c_str();
  }

  /**
   * \brief Initialize the module loader.
   *
   * If the module loader is initialized with this function shared object
   * modules will not receive command line arguments.
   *
   * \param file_name [in] The main program script file name
   *
   * \return This function returns true if the module loader was
   *         successfully initialized, false otherwise.
   */
  bool Initialize(const char* file_name);

  /**
   * \brief Initialize the module loader.
   *
   * If the module loader is initialized with this function command line
   * arguments will be passed to shared object modules. Shared object
   * modules may modify the contents of the command line vector.
   *
   * \param file_name [in] The main program script file name
   * \param argc [in/out] A pointer to the command line argument count
   * \param argv [in/out] A pointer to the command line argument vector
   *
   * \return This function returns true if the module loader was
   *         successfully initialized, false otherwise.
   */
  bool Initialize(const char* file_name, int* argc, char*** argv);

private: // non-copyable
  ModuleLoader(ModuleLoader const& that);

  void operator=(ModuleLoader const& that);

private: // private methods
  static v8::Handle<v8::Value> Require(const v8::Arguments& args);

private: // private data
  std::string error_;
  bool initialized_;
  bool secure_;
  v8::Handle<v8::Context> context_;
  v8::Persistent<v8::Object> require_;
  v8::Persistent<v8::Array> paths_;
  ModuleFactoryPointer module_factory_;
  ModuleStack module_stack_;
};

#endif // MOKA_MODULE_LOADER_H

// vim: tabstop=2:sw=2:expandtab
