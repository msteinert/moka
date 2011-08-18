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
#include <climits>
#include <cstdlib>
#include <cstring>
#include <tr1/memory>
#include "v8-commonjs/system.h"

// Needed to enumerate the environment
extern char** environ;

namespace commonjs {

// File
File::File()
  : file_(NULL)
  , error_(0)
  , message_("Success") {}

File::File(FILE* file)
  : file_(file)
  , error_(0)
  , message_("Success") {}

File::~File() {
  if (file_) {
    ::fclose(file_);
  }
}

bool File::Open(const char* file_name, const char *mode) {
  if (file_) {
    error_ = errno;
    return false;
  }
  file_ = fopen(file_name, mode);
  if (!file_) {
    error_ = errno;
    return false;
  }
  return true;
}

void File::Close() {
  if (file_) {
    ::fclose(file_);
    file_ = NULL;
  }
}

void File::Flush() {
  if (file_) {
    ::fflush(file_);
  }
}

void File::SetError() {
  error_ = INT_MAX;
  message_.assign("Set error");
}

void File::SetError(const char* message) {
  error_ = INT_MAX;
  message_.assign(message);
}

bool File::CheckError() {
  if (error_) {
    return true;
  }
  if (file_) {
    ::fflush(file_);
    int error = ::ferror(file_);
    if (error) {
      char message[BUFSIZ];
      ::strerror_r(error, message, BUFSIZ);
      message_.assign(message);
      return true;
    }
  }
  return false;
}

void File::Print(char character) {
  if (file_) {
    ::fputc(character, file_);
  }
}

void File::Print(const char* string) {
  if (file_) {
    ::fputs(string, file_);
  }
}

void File::Print(v8::Handle<v8::Value> value) {
  if (!file_) {
    return;
  }
  ::fputs(*v8::String::Utf8Value(value->ToString()), file_);
}

void File::Println(v8::Handle<v8::Value> value) {
  if (!file_) {
    return;
  }
  Print(value);
  ::fputc('\n', file_);
}

void File::Write(v8::Handle<v8::Value> value) {
  if (!file_) {
    return;
  }
  v8::String::Utf8Value string(value->ToString());
  ::fwrite(*string, sizeof **string, string.length(), stdout);
}

// PrintStream 
v8::Persistent<v8::Object> PrintStream::New(FILE* file) {
  v8::HandleScope handle_scope;
  if (!file) {
    return v8::Persistent<v8::Object>();
  }
  v8::Local<v8::ObjectTemplate> stream_templ = v8::ObjectTemplate::New();
  stream_templ->SetInternalFieldCount(1);
  stream_templ->Set(v8::String::NewSymbol("close"),
      v8::FunctionTemplate::New(Close));
  stream_templ->Set(v8::String::NewSymbol("flush"),
      v8::FunctionTemplate::New(Flush));
  stream_templ->Set(v8::String::NewSymbol("setError"),
      v8::FunctionTemplate::New(SetError));
  stream_templ->Set(v8::String::NewSymbol("checkError"),
      v8::FunctionTemplate::New(CheckError));
  stream_templ->Set(v8::String::NewSymbol("print"),
      v8::FunctionTemplate::New(Print));
  stream_templ->Set(v8::String::NewSymbol("println"),
      v8::FunctionTemplate::New(Println));
  stream_templ->Set(v8::String::NewSymbol("write"),
      v8::FunctionTemplate::New(Write));
  // Create the file structure
  struct File* file_ptr = new File(file);
  if (!file_ptr) {
    return v8::Persistent<v8::Object>();
  }
  v8::Persistent<v8::Object> stream =
    v8::Persistent<v8::Object>::New(stream_templ->NewInstance());
  stream->SetInternalField(0, v8::External::New(file_ptr));
  stream.MakeWeak(static_cast<void*>(file_ptr), Delete);
  return stream;
}

void PrintStream::Delete(v8::Persistent<v8::Value> object, void* parameters) {
  struct File* file_ptr = static_cast<File*>(parameters);
  if (file_ptr) {
    delete file_ptr;
  }
  object.Dispose();
  object.Clear();
}

v8::Handle<v8::Value> PrintStream::Close(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Check the number of arguments
  if (arguments.Length() != 0) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("No arguments accepted")));
  }
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  file_ptr->Close();
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> PrintStream::Flush(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Check the number of arguments
  if (arguments.Length() != 0) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("No arguments accepted")));
  }
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  file_ptr->Flush();
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> PrintStream::SetError(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Check the number of arguments
  if (arguments.Length() != 0) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("No arguments accepted")));
  }
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  file_ptr->SetError();
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> PrintStream::CheckError(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Check the number of arguments
  if (arguments.Length() != 0) {
    return handle_scope.Close(v8::ThrowException(
          v8::String::New("No arguments accepted")));
  }
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  if (file_ptr->CheckError()) {
    return handle_scope.Close(v8::True());
  }
  return handle_scope.Close(v8::False());
}

v8::Handle<v8::Value> PrintStream::Print(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  for (int index = 0; index < arguments.Length(); ++index) {
    if (index) {
      file_ptr->Print(' ');
    }
    file_ptr->Print(arguments[index]);
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> PrintStream::Println(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  for (int index = 0; index < arguments.Length(); ++index) {
    if (index) {
      file_ptr->Print(' ');
    }
    file_ptr->Println(arguments[index]);
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

v8::Handle<v8::Value> PrintStream::Write(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  // Get the File pointer
  v8::Local<v8::Object> object = arguments.Holder();
  v8::Local<v8::External> external =
    v8::Local<v8::External>::Cast(object->GetInternalField(0));
  File* file_ptr = static_cast<File*>(external->Value());
  for (int index = 0; index < arguments.Length(); ++index) {
    if (index) {
      file_ptr->Print(' ');
    }
    file_ptr->Write(arguments[index]);
  }
  return handle_scope.Close(v8::Handle<v8::Value>());
}

// Stdout
bool Stdout::Initialize(commonjs::Module& module) {
  v8::HandleScope handle_scope;
  v8::Persistent<v8::Object> stream = PrintStream::New(stdout);
  if (stream.IsEmpty()) {
    return false;
  }
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("stdout"), stream);
  return true;
}

// Stderr
bool Stderr::Initialize(commonjs::Module& module) {
  v8::HandleScope handle_scope;
  v8::Persistent<v8::Object> stream = PrintStream::New(stderr);
  if (stream.IsEmpty()) {
    return false;
  }
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("stderr"), stream);
  return true;
}

// Env
static v8::Handle<v8::Value> EnvGet(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  const char* value = ::getenv(*v8::String::Utf8Value(property));
  if (value) {
    return handle_scope.Close(v8::String::New(value));
  }
  return handle_scope.Close(v8::Local<v8::Value>());
}

static v8::Handle<v8::Value> EnvSet(v8::Local<v8::String> property,
    v8::Local<v8::Value> value, const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (-1 == ::setenv(*v8::String::Utf8Value(property),
        *v8::String::Utf8Value(value), 1)) {
    return handle_scope.Close(v8::Handle<v8::Value>());
  }
  return handle_scope.Close(value);
}

static v8::Handle<v8::Integer> EnvQuery(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (::getenv(*v8::String::Utf8Value(property))) {
    return handle_scope.Close(v8::Integer::New(v8::None));
  }
  return handle_scope.Close(v8::Handle<v8::Integer>());
}

static v8::Handle<v8::Boolean> EnvDelete(v8::Local<v8::String> property,
    const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  if (::getenv(*v8::String::Utf8Value(property))) {
    if (-1 != ::unsetenv(*v8::String::Utf8Value(property))) {
      return handle_scope.Close(v8::True());
    }
  }
  return handle_scope.Close(v8::False());
}

v8::Handle<v8::Array> EnvEnumerate(const v8::AccessorInfo& /* info */) {
  v8::HandleScope handle_scope;
  int size = 0;
  while (environ[++size])
    ;
  v8::Local<v8::Array> env = v8::Array::New(size);
  for (int index = 0; index < size; ++index) {
    const char* equal = strchr(environ[index], '=');
    if (equal) {
      env->Set(index, v8::String::New(environ[index], equal - environ[index]));
    } else {
      env->Set(index, v8::String::New(environ[index]));
    }
  }
  return handle_scope.Close(env);
}

bool Env::Initialize(Module& module) {
  v8::HandleScope handle_scope;
  v8::Local<v8::ObjectTemplate> env_templ = v8::ObjectTemplate::New();
  env_templ->SetNamedPropertyHandler(EnvGet, EnvSet, EnvQuery, EnvDelete,
      EnvEnumerate);
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("env"), env_templ->NewInstance());
  return true;
}

// Args
bool Args::Initialize(Module& module, int argc, char** argv) {
  v8::HandleScope handle_scope;
  v8::Local<v8::Array> args = v8::Array::New(argc);
  for (int index = 0; index < argc; ++index) {
    args->Set(index, v8::String::New(argv[index]));
  }
  v8::Handle<v8::Object> exports = module.GetExports();
  exports->Set(v8::String::NewSymbol("args"), args);
  return true;
}

static bool system_initialize(Module& module, int* argc, char*** argv)
{
  v8::HandleScope handle_scope;
  if (!Stdout::Initialize(module)) {
    return false;
  }
  if (!Stderr::Initialize(module)) {
    return false;
  }
  if (!Env::Initialize(module)) {
    return false;
  }
  if (!Args::Initialize(module, argc ? *argc : 0, argv ? *argv : NULL)) {
    return false;
  }
  return true;
}

} // namespace commonjs

COMMONJS_MODULE(commonjs::system_initialize)

// vim: tabstop=2:sw=2:expandtab
