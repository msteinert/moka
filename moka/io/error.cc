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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "moka/io/error.h"

namespace moka {

namespace io {

v8::Handle<v8::Value> Error::New(const char* message) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> argv[1] = { v8::String::New(message) };
  return GetTemplate()->GetFunction()->NewInstance(1, argv);
}

v8::Handle<v8::FunctionTemplate> Error::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->Inherit(Module::Exception::GetTemplate());
  templ->SetClassName(v8::String::NewSymbol("Error"));
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Error::New(
    const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    return Module::ConstructCall(GetTemplate(), arguments);
  }
  v8::Handle<v8::Object> self = arguments.This();
  self->Set(v8::String::NewSymbol("name"),
      v8::String::NewSymbol("Error"));
  if (arguments.Length()) {
    if (!arguments[0].IsEmpty()) {
      self->Set(v8::String::NewSymbol("message"), arguments[0]);
    }
  }
  return self;
}

} // namespace io

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
