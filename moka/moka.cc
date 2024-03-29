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

#include <cerrno>
#include <cstdio>
#include <cstring>
#include "moka/moka.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

void Report(v8::TryCatch& try_catch) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Message> message = try_catch.Message();
  if (message.IsEmpty()) {
    fprintf(stderr, "%s\n", *v8::String::Utf8Value(try_catch.Exception()));
    return;
  }
  fprintf(stderr, "[%s:%d] %s\n",
      *v8::String::Utf8Value(message->GetScriptResourceName()),
      message->GetLineNumber(), *v8::String::Utf8Value(try_catch.Exception()));
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "error: %s <script>\n", argv[0]);
    return 1;
  }
  // Create a stack allocated handle scope
  v8::HandleScope handle_scope;
  // Create a new context
  v8::Persistent<v8::Context> context = v8::Context::New();
  // Enter the created context
  v8::Context::Scope scope(context);
  // Initialize module loader
  moka::ModuleLoader loader;
  if (!loader.Initialize(argv[1], &argc, &argv)) {
    fprintf(stderr, "error: module loader: %s\n", loader.GetError());
    context.Dispose();
    v8::V8::Dispose();
    return 1;
  }
  // Read file into a v8 string
  FILE* file = fopen(argv[1], "rb");
  if (!file) {
    fprintf(stderr, "error: %s\n", strerror(errno));
    context.Dispose();
    v8::V8::Dispose();
    return 1;
  }
  struct stat buf;
  if (fstat(fileno(file), &buf)) {
    fprintf(stderr, "error: %s\n", strerror(errno));
    context.Dispose();
    v8::V8::Dispose();
    fclose(file);
    return 1;
  }
  if (!S_ISREG(buf.st_mode)) {
    fprintf(stderr, "error: '%s' is not a regular file\n", argv[1]);
    context.Dispose();
    v8::V8::Dispose();
    fclose(file);
    return 1;
  }
  char* characters = new char[buf.st_size + 1];
  if (!characters) {
    fprintf(stderr, "error: %s\n", strerror(errno));
    context.Dispose();
    v8::V8::Dispose();
    fclose(file);
    return 1;
  }
  size_t size = fread(characters, 1, buf.st_size, file);
  if (static_cast<off_t>(size) < buf.st_size) {
    if (ferror(file)) {
      fprintf(stderr, "error: %s\n", strerror(errno));
      context.Dispose();
      v8::V8::Dispose();
      delete[] characters;
      fclose(file);
      return 1;
    }
    clearerr(file);
  }
  fclose(file);
  v8::Handle<v8::String> source = v8::String::New(characters, size);
  delete[] characters;
  if (source.IsEmpty()) {
    fprintf(stderr, "error: failed to read %s\n", argv[1]);
    context.Dispose();
    v8::V8::Dispose();
    return 1;
  }
  // Compile string
  v8::TryCatch try_catch;
  v8::Handle<v8::Script> script = v8::Script::Compile(source,
      v8::String::New(argv[1]));
  if (script.IsEmpty()) {
    Report(try_catch);
    context.Dispose();
    v8::V8::Dispose();
    return 1;
  }
  // Execute string
  v8::Handle<v8::Value> result = script->Run();
  if (result.IsEmpty()) {
    Report(try_catch);
    context.Dispose();
    v8::V8::Dispose();
    return 1;
  }
  // Clean up
  context.Dispose();
  v8::V8::Dispose();
  return 0;
}

// vim: tabstop=2:sw=2:expandtab
