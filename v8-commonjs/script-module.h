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
 * \brief A CommonJS JavaScript source code module
 */

#ifndef V8_COMMONJS_SCRIPT_MODULE_H
#define V8_COMMONJS_SCRIPT_MODULE_H

#include <cstdio>
#include <v8.h>
#include "v8-commonjs/module.h"

namespace commonjs {

namespace internal {

class ScriptModule;

} // namespace internal

} // namespace commonjs

/// \brief A CommonJS script module
class commonjs::internal::ScriptModule: public commonjs::Module {
public:
  /**
   * \brief Construct a module from JavaScript source code
   *
   * \param id [in] The ID of the new module
   * \param file_name [in] The absolute path of the file containing the module
   * \param secure [in] Indicates if this should be a secure module
   * \param require [in] The object implementing the 'require' function
   * \param file [in] An open file handle (this object will own @file)
   * \param size [in] The size of @file in bytes
   */
  ScriptModule(const char* id, const char* file_name, bool secure,
      v8::Handle<v8::Object> require, FILE* file, size_t size);

  /// \brief Destructor
  virtual ~ScriptModule();

  /**
   * \brief Load a JavaScript source code module
   *
   * This function reads JavaScript from a file handle and then attempts
   * to compile and run the script.
   *
   * \return This function returns @true if successful, @false otherwise.
   */
  virtual bool Load();

private:
  FILE* file_;
  size_t size_;
  bool loaded_;
};

#endif // V8_COMMONJS_SCRIPT_MODULE_H

// vim: tabstop=2:sw=2:expandtab
