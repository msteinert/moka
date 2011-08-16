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

#ifndef V8_COMMONJS_MODULE_H
#define V8_COMMONJS_MODULE_H

#include <string>
#include <v8.h>

namespace commonjs {

namespace internal {

class Module;

} // namespace internal

} // namespace commonjs

/**
 * A CommonJS 1.1 module
 */
class commonjs::internal::Module {
public:
  Module(const char* id, const char* file_name, bool secure,
      v8::Handle<v8::Object> require, v8::Handle<v8::Context> context);

  Module(const char* id, const char* file_name, bool secure,
      v8::Handle<v8::Object> require);

  virtual ~Module();

  bool Initialize();

  virtual v8::Handle<v8::Value> Load() {
    return GetExports();
  }

  const char* GetFileName() const {
    return file_name_.c_str();
  }

  const char* GetDirectoryName();

  const v8::Handle<v8::Context> GetContext() const {
    return context_;
  }

  const v8::Handle<v8::Object> GetModule() const {
    return module_;
  }

  const v8::Handle<v8::Object> GetExports() const {
    return exports_;
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
  void* handle_;
};

#endif // V8_COMMONJS_MODULE_H

// vim: tabstop=2:sw=2:expandtab
