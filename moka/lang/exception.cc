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

#include "moka/lang/exception.h"

namespace moka {

v8::Handle<v8::Value> Exception::New(v8::Handle<v8::Value> message) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Object> object;
  if (!message.IsEmpty()) {
    object = v8::Object::New();
    object->Set(v8::String::NewSymbol("message"), message);
  }
  if (object.IsEmpty()) {
    return GetTemplate()->GetFunction()->NewInstance();
  } else {
    v8::Handle<v8::Value> argv[1] = { object };
    return GetTemplate()->GetFunction()->NewInstance(1, argv);
  }
}

v8::Handle<v8::FunctionTemplate> Exception::GetTemplate() {
  v8::HandleScope handle_scope;
  static v8::Persistent<v8::FunctionTemplate> templ_;
  if (!templ_.IsEmpty()) {
    return templ_;
  }
  v8::Local<v8::FunctionTemplate> templ = v8::FunctionTemplate::New(New);
  templ->SetClassName(v8::String::NewSymbol("Exception"));
  templ->PrototypeTemplate()->Set(v8::String::NewSymbol("toString"),
      v8::FunctionTemplate::New(ToString)->GetFunction());
  templ_ = v8::Persistent<v8::FunctionTemplate>::New(templ);
  return templ_;
}

v8::Handle<v8::Value> Exception::New(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  if (!arguments.IsConstructCall()) {
    int argc = arguments.Length();
    v8::Local<v8::Value>* argv = new v8::Local<v8::Value>[argc];
    if (!argv) {
      return handle_scope.Close(v8::ThrowException(
            v8::String::New("No memory")));
    }
    for (int index = 0; index < argc; ++index) {
      argv[index] = arguments[index];
    }
    v8::Local<v8::Object> instance =
      GetTemplate()->GetFunction()->NewInstance(argc, argv);
    delete[] argv;
    return handle_scope.Close(instance);
  }
  v8::Handle<v8::Object> self = arguments.This();
  self->Set(v8::String::NewSymbol("name"), v8::String::NewSymbol("Exception"));
  if (arguments.Length()) {
    if (!arguments[0].IsEmpty()) {
      self->Set(v8::String::NewSymbol("message"), arguments[0]);
    }
  }
  return self;
}

v8::Handle<v8::Value> Exception::ToString(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  std::string string;
  v8::Local<v8::Object> self = arguments.This();
  v8::Local<v8::Value> name = self->Get(v8::String::NewSymbol("name"));
  if (!name->Equals(v8::Undefined())) {
    string.append(*v8::String::Utf8Value(name->ToString()));
  }
  v8::Local<v8::Value> message = self->Get(v8::String::NewSymbol("message"));
  if (!message->Equals(v8::Undefined())) {
    if (!string.empty()) {
      string.append(": ");
    }
    string.append(*v8::String::Utf8Value(message->ToString()));
  }
  if (string.empty()) {
    return handle_scope.Close(v8::Local<v8::String>());
  }
  return handle_scope.Close(v8::String::New(string.c_str()));
}

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
