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
 * \brief Interface to CommonJS modules
 */

#ifndef V8_COMMONJS_MODULE_H
#define V8_COMMONJS_MODULE_H

#include <string>
#include <v8.h>
#include <v8-commonjs/macros.h>

namespace commonjs {

class Module;

/**
 * \brief The shared module initialization callback signature
 *
 * The module loader will call this initialization function during the
 * shared module loading process. This function should add new objects
 * the exports for the @module. Exports can be retrieved by calling the
 * function commonjs::Module::GetExports().
 *
 * If any errors occur during the module initialization, the module should
 * return @false and optionally set an exception by calling
 * commonjs::Module::SetException(). If no exception is set then a generic
 * exception will be returned to JavaScript.
 *
 * \param module [in/out] The v8-commonjs module object for the module
 * \param argc [in/out] A pointer to application argument count
 * \param argv [in/out] A pointer to the application argument vector
 *
 * \return This function should return @true if the module was successfully
 *         initialized. If this function returns success the module's exports
 *         will be returned to JavaScript. If the module couldnot be
 *         initialized this function should return @false.
 */
typedef bool (*InitializeCallback)(Module& module, int* argc, char*** argv);

/**
 * \brief The shared module initialization structure
 *
 * In order to be loaded, shared modules must expose this a structure of this
 * type called "commonjs_module__". The macro COMMONJS_MODULE() is provided
 * to assist the programmer in exporting this structure.
 *
 * The major version must match the library major version in order for the
 * module to be successfully loaded. The minor version must be greater than
 * or equal to the library minor version.
 *
 * The Initialization function will be called during the module loading
 * process. If this function is NULL then the module will fail to be loaded.
 */
struct module {
  int version_major; ///< The module major version
  int version_minor; ///< The module minor version
  InitializeCallback initialize; ///< The module initialization callback
};

} // namespace commonjs

/// \brief A CommonJS 1.1 module
class COMMONJSEXPORT commonjs::Module {
public:
  /**
   * \brief Construct a module in an existing V8 context
   *
   * The returned module will execute in the provided context. This
   * constructor is intended for use by the main module.
   *
   * \param id [in] The ID of the new module
   * \param file_name [in] The absolute path of the file containing the module
   * \param secure [in] Indicates if this should be a secure module
   * \param require [in] The object implementing the 'require' function
   * \param context [in] The context this module should execute in
   */
  Module(const char* id, const char* file_name, bool secure,
      v8::Handle<v8::Object> require, v8::Handle<v8::Context> context);

  /**
   * \brief Construct a module in a new V8 context
   *
   * The returned module will execute in a newly constructed context.
   *
   * \param id [in] The id of the new module
   * \param file_name [in] The absolute path of the file containing the module
   * \param secure [in] Indicates if this should be a secure module
   * \param require [in] The object implementing the 'require' function
   */
  Module(const char* id, const char* file_name, bool secure,
      v8::Handle<v8::Object> require);

  /// \brief The default destructor
  virtual ~Module();

  /**
   * \brief Module base class initialization
   *
   * All modules must be successfully initialized before being loaded.
   *
   * \return This function returns @true if the initialization was
   *         successful, @false otherwise.
   */
  bool Initialize();

  /**
   * \brief Load a module
   *
   * This function evaluates any require code and initializes module exports.
   *
   * \return This function returns @true if the module was successfully
   *         loaded, @false otherwise.
   */
  virtual bool Load() {
    return true;
  }

  /**
   * \brief Get the module ID
   *
   * \return The module ID.
   */
  const char* GetId() const {
    return id_.c_str();
  }

  /**
   * \brief Get the module file name (absolute path)
   *
   * \return The module file name.
   */
  const char* GetFileName() const {
    return file_name_.c_str();
  }

  /**
   * \brief Get the module directory name (absolute path)
   *
   * \return The module directory name.
   */
  const char* GetDirectoryName();

  /**
   * \brief Get the V8 context for this module
   *
   * \return The context for this module.
   */
  const v8::Handle<v8::Context> GetContext() const {
    return context_;
  }

  /**
   * \brief Get the 'module' object for this module
   *
   * \return The 'module' object.
   */
  const v8::Handle<v8::Object> GetModule() const {
    return module_;
  }

  /**
   * \brief Get the V8 'exports' object for this module
   *
   * Shared object modules should add their APIs and objects to this object.
   *
   * \return The V8 'exports' object.
   */
  const v8::Handle<v8::Object> GetExports() const {
    return exports_;
  }

  /**
   * \brief Get an exception object after an API error
   *
   * \return The previously set exception object.
   */
  const v8::Handle<v8::Value> GetException() const {
    return exception_;
  }

  /**
   * \brief Set an exception from a C string
   *
   * \param exception [in] The exception message
   */
  void SetException(const char* exception) {
    if (!exception_.IsEmpty()) {
      exception_.Dispose();
    }
    exception_ = v8::Persistent<v8::Value>::New(
        v8::Exception::Error(v8::String::New(exception)));
  }

  /**
   * \brief Set an exception from a C++ std::string
   *
   * \param exception [in] The exception message
   */
  void SetException(const std::string& exception) {
    SetException(exception.c_str());
  }

  /**
   * \brief Set an exception from a V8 exception object
   *
   * \param exception [in] The exception
   */
  void SetException(v8::Handle<v8::Value> exception) {
    if (!exception_.IsEmpty()) {
      exception_.Dispose();
    }
    exception_ = v8::Persistent<v8::Value>::New(exception);
  }

private: // non-copyable
  Module(Module const& that);

  void operator=(Module const& that);

private: // private methods
  static v8::Handle<v8::Value> Print(const v8::Arguments& args);

private: // private data
  std::string id_;
  std::string file_name_;
  char* directory_name_;
  bool secure_;
  bool initialized_;
  bool context_owner_;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Object> require_;
  v8::Persistent<v8::Object> exports_;
  v8::Persistent<v8::Object> module_;
  v8::Persistent<v8::Value> exception_;
};

/**
 * \brief The current major version of the module loader
 *
 * The module loader will only load modules that have a major version number
 * equal to this value.
 */
#define COMMONJS_MODULE_VERSION_MAJOR (1)

/**
 * \brief The current minor version of the module loader
 *
 * The module loader will only load modules that have a minor version number
 * greater than or equal to this value.
 */
#define COMMONJS_MODULE_VERSION_MINOR (0)

/**
 * \brief Helper macro for shared module implementations
 *
 * This macro may be used by module implementations to ensure the correct
 * declaration and linkage for the commonjs_module__ structure.
 *
 * The initialization function must of the type:
 * commonjs::Module::InitializeCallback
 *
 * The side-effect of calling this macro is to declare a globally visible
 * commonjs::module structure that the module loader will use to load the
 * module.
 *
 * \param initialize [in] A pointer to the module initialization function
 */
#define COMMONJS_MODULE(initialize) \
extern "C" { \
  commonjs::module COMMONJSEXPORT commonjs_module__ = { \
    COMMONJS_MODULE_VERSION_MAJOR, \
    COMMONJS_MODULE_VERSION_MINOR, \
    initialize, \
  }; \
}

#endif // V8_COMMONJS_MODULE_H

// vim: tabstop=2:sw=2:expandtab
