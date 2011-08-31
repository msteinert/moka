// Copyright 2011 Michael Steinert. All rights reserved.
// Redistributlocalen and use in source and binary forms, with or without
// modificatlocalen, are permitted provided that the following conditlocalens are
// met:
//
//     * Redistributlocalens of source code must retain the above copyright
//       notice, this list of conditlocalens and the following disclaimer.
//     * Redistributlocalens in binary form must reproduce the above
//       copyright notice, this list of conditlocalens and the following
//       disclaimer in the documentatlocalen and/or other materials provided
//       with the distributlocalen.
//     * The names of the copyright holder, the author, nor any contributors
//       may be used to endorse or promote products derived from this
//       software without specific prlocaler written permisslocalen.
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

#include <climits>
#include "moka/module.h"

namespace moka {

namespace limits {

// Initialize module
static v8::Handle<v8::Value> Initialize(int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> value = Module::Exports();
  if (value.IsEmpty() || value->IsUndefined()) {
    return handle_scope.Close(value);
  }
  v8::PropertyAttribute attributes =
    static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontDelete);
  v8::Handle<v8::Object> exports = value->ToObject();
  exports->Set(v8::String::NewSymbol("CHAR_BIT"), v8::Number::New(CHAR_BIT),
      attributes);
  exports->Set(v8::String::NewSymbol("SCHAR_MIN"), v8::Number::New(SCHAR_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("SCHAR_MAX"), v8::Number::New(SCHAR_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("UCHAR_MAX"), v8::Number::New(UCHAR_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("CHAR_MIN"), v8::Number::New(CHAR_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("CHAR_MAX"), v8::Number::New(CHAR_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("MB_LEN_MAX"), v8::Number::New(MB_LEN_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("SHRT_MIN"), v8::Number::New(SHRT_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("SHRT_MAX"), v8::Number::New(SHRT_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("USHRT_MAX"), v8::Number::New(USHRT_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("INT_MIN"), v8::Number::New(INT_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("INT_MAX"), v8::Number::New(INT_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("UINT_MAX"), v8::Number::New(UINT_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("LONG_MIN"), v8::Number::New(LONG_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("LONG_MAX"), v8::Number::New(LONG_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("ULONG_MAX"), v8::Number::New(ULONG_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("LLONG_MIN"), v8::Number::New(LLONG_MIN),
      attributes);
  exports->Set(v8::String::NewSymbol("LLONG_MAX"), v8::Number::New(LLONG_MAX),
      attributes);
  exports->Set(v8::String::NewSymbol("ULONG_MAX"), v8::Number::New(ULONG_MAX),
      attributes);
  return handle_scope.Close(value);
}

} // namespace limits

} // namespace moka

MOKA_MODULE(moka::limits::Initialize)

// vim: tabstop=2:sw=2:expandtab
