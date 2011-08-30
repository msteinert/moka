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

#include "moka/io/buffer.h"
#include "moka/io/error.h"
#include "moka/io/iconv.h"
#include "moka/io/stream.h"
#include "moka/module.h"

namespace moka {

namespace io {

// Initialize module
static v8::Handle<v8::Value> Initialize(int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> value = Module::Exports();
  if (value.IsEmpty() || value->IsUndefined()) {
    return handle_scope.Close(value);
  }
  v8::Handle<v8::Object> exports = value->ToObject();
  exports->Set(v8::String::NewSymbol("Buffer"),
      Buffer::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("Error"),
      Error::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("Iconv"),
      Iconv::GetTemplate()->GetFunction());
  exports->Set(v8::String::NewSymbol("Stream"),
      Stream::GetTemplate()->GetFunction());
  return handle_scope.Close(value);
}

} // namespace io

} // namespace moka

MOKA_MODULE(moka::io::Initialize)

// vim: tabstop=2:sw=2:expandtab
