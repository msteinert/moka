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
#include "commonjs/module.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <sstream>

using namespace commonjs;
using namespace std;
using namespace v8;

Module::Module()
	: exception_("None")
	, initialized_(false)
{
}

Module::~Module()
{
	path_.Dispose();
	for (map<string, struct module*>::iterator iter = modules_.begin();
			iter != modules_.end(); ++iter) {
		(*iter).second->object.Dispose();
		dlclose((*iter).second->handle);
	}
}

bool Module::Initialize(int* argc, char*** argv)
{
	HandleScope handle_scope;
	if (initialized_) {
		exception_.assign("Already initialized");
		return false;
	}
	context_ = Context::GetEntered();
	if (context_.IsEmpty()) {
		exception_.assign("No currently entered context");
		return false;
	}
	argc_ = argc;
	argv_ = argv;
	// Create module.path in handle_scope_
	uint32_t index = 0;
	Local<Array> path = Array::New();
	// Add directories from environment
	const char* env = getenv("COMMONJSPATH");
	if (env) {
		string token;
		stringstream commonjspath(env);
		while (getline(commonjspath, token, ':')) {
			path->Set(index++, String::New(token.c_str()));
		}
	}
	// Add current working directory
	path->Set(index++, String::New("."));
	env = getenv("HOME");
	if (env) {
		// Add $HOME/lib/commonjs
		path->Set(index++, String::Concat(String::New(env),
						String::New("/lib/commonjs")));
	}
	// Add $libdir/commonjs
	path->Set(index++, String::New(LIBDIR "/commonjs"));
	// Store 'path' as a persistent object
	path_ = Persistent<Array>::New(path);
	// Create 'require' object
	Handle<ObjectTemplate> require_templ = ObjectTemplate::New();
	require_templ->SetInternalFieldCount(1);
	require_templ->SetCallAsFunctionHandler(Require);
	require_templ->Set(String::New("path"), path_,
			static_cast<PropertyAttribute>(ReadOnly | DontDelete));
	// Create module
	Local<Object> require = require_templ->NewInstance();
	require->SetInternalField(0, External::New(this));
	context_->Global()->Set(String::New("require"), require);
	initialized_ = true;
	return true;
}

const char* Module::Exception() const
{
	return exception_.c_str();
}

Handle<Value> Module::Require(const Arguments& arguments)
{
	HandleScope handle_scope;
	if (arguments.Length() != 1) {
		return handle_scope.Close(ThrowException(String::New(
						"A single argument is "
						"required")));
	}
	if (!arguments[0]->IsString()) {
		return handle_scope.Close(ThrowException(String::New(
						"Argument one must be a "
						"string")));
	}
	Local<Object> object = arguments.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	if (!module->initialized_) {
		return handle_scope.Close(ThrowException(String::New(
						"Module loader is not "
						"initialized")));
	}
	map<string, struct module*>::iterator iter =
		module->modules_.find(*String::Utf8Value(arguments[0]));
	if (module->modules_.end() != iter) {
		return (*iter).second->object;
	}
	Local<Array> properties = module->path_->GetPropertyNames();
	if (properties.IsEmpty()) {
		return handle_scope.Close(Undefined());
	}
	uint32_t index = 0;
	while (index < properties->Length()) {
		Local<Value> value = properties->Get(index++);
		if (!value->IsUint32()) {
			return handle_scope.Close(ThrowException(String::New(
							"Index error")));
		}
		Local<Value> path = module->path_->Get(value->Uint32Value());
		if (path.IsEmpty()) {
			return handle_scope.Close(ThrowException(String::New(
							"Index error")));
		}
		if (path->IsString()) {
			Handle<Value> object = module->RequireSharedObject(
					*String::Utf8Value(arguments[0]),
					*String::Utf8Value(path));
			if (!object.IsEmpty()) {
				return handle_scope.Close(object);
			}
			object = module->RequireScript(
					*String::Utf8Value(arguments[0]),
					*String::Utf8Value(path));
			if (!object.IsEmpty()) {
				return handle_scope.Close(object);
			}
			object = module->RequireModule(
					*String::Utf8Value(arguments[0]),
					*String::Utf8Value(path));
			if (!object.IsEmpty()) {
				return handle_scope.Close(object);
			}
		}
	}
	string error("No module named ");
	error.append(*String::Utf8Value(arguments[0]));
	return handle_scope.Close(ThrowException(String::New(error.c_str())));
}

Handle<Value> Module::RequireSharedObject(const char* name, const char* path)
{
	HandleScope handle_scope;
	string full_path(string(path) + '/' + name + ".so");
	void* handle = dlopen(full_path.c_str(), RTLD_LAZY);
	if (!handle) {
		Handle<Value> value;
		return handle_scope.Close(Handle<Value>());
	}
	string module_name(string(name) + "_module");
	struct module* init = static_cast<struct module*>(dlsym(handle,
				module_name.c_str()));
	if (!init) {
		dlclose(handle);
		return handle_scope.Close(Handle<Value>());
	}
	if (!init->initialize) {
		dlclose(handle);
		return handle_scope.Close(Handle<Value>());
	}
	if (init->version_major != COMMONJS_MODULE_VERSION_MAJOR) {
		dlclose(handle);
		return handle_scope.Close(Handle<Value>());
	}
	Handle<Object> object = init->initialize(argc_, argv_);
	if (object.IsEmpty()) {
		dlclose(handle);
		return handle_scope.Close(Handle<Value>());
	}
	init->object = Persistent<Object>::New(object);
	init->handle = handle;
	modules_.insert(pair<string, struct module*>(string(name), init));
	return init->object;
}

Handle<Value> Module::RequireScript(const char* name, const char* path)
{
	HandleScope handle_scope;
	return handle_scope.Close(Handle<Value>());
}

Handle<Value> Module::RequireModule(const char* name, const char* path)
{
	HandleScope handle_scope;
	return handle_scope.Close(Handle<Value>());
}
