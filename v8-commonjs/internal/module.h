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

#ifndef COMMONJS_INTERNAL_MODULE_H
#define COMMONJS_INTERNAL_MODULE_H

#include <string>
#include <v8.h>

namespace commonjs {

namespace internal {

class Module;

}

} // namespace commonjs

/**
 * A CommonJS 1.1 module loader
 */
class commonjs::internal::Module {
public:
  Module(bool secure, v8::Handle<v8::Object> require,
      v8::Handle<v8::Context> context);

  Module(bool secure, v8::Handle<v8::Object> require);

  ~Module();

  const v8::Handle<v8::Value> Exception() const {
    return exception_;
  }

  bool Initialize(const char* id, const char* uri);

  bool RequireScript(const char* id, const char* path);

  const v8::Handle<v8::Object> GetModule() const {
    return module_;
  }

  const v8::Handle<v8::Object> GetExports() const {
    return exports_;
  }

  const char* GetDirname() const {
    return dirname_.c_str();
  }

  const char* GetId() const {
    return id_.c_str();
  }
    
private: // non-copyable
  Module(Module const& that);

  void operator=(Module const& that);

private: // private data
  bool secure_;
  bool context_owner_;
  v8::Persistent<v8::Context> context_;
  v8::Persistent<v8::Object> require_;
  v8::Persistent<v8::Object> exports_;
  v8::Persistent<v8::Object> module_;
  v8::Persistent<v8::Value> exception_;
  void* handle_;
  std::string dirname_;
  std::string id_;
};

#endif // COMMONJS_INTERNAL_MODULE_H

// vim: tabstop=2:sw=2:expandtab
