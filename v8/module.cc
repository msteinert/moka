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
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "v8/module.h"

using namespace std;
using namespace v8;

Module::Module()
	: initialized_(false)
	, argv_(0)
{
}

Module::~Module()
{
	if (argv_) {
		for (int i = 0; i < argc_; ++i) {
			delete[] argv_[i];
		}
		delete[] argv_;
	}
}

bool Module::Initialize(int argc, char *argv[])
{
	if (initialized_) {
		SetException("already initialized");
		return false;
	}
	context_ = Context::GetEntered();
	if (context_.IsEmpty()) {
		SetException("no currently entered context");
		return false;
	}
	argc_ = argc;
	argv_ = new char*[argc];
	if (!argv_) {
		SetException("out of memory");
		return false;
	}
	for (int i = 0; i < argc; ++i) {
		argv_[i] = new char[strlen(argv[i] + 1)];
		if (!argv_[i]) {
			argc_ = i;
			SetException("out of memory");
			return false;
		}
		strcpy(argv_[i], argv[i]);
	}
	const char* env = getenv("V8PATH");
	if (env) {
		string path;
		stringstream v8path(env);
		while (getline(v8path, path, ':')) {
			path_.push_back(path);
		}
	}
	path_.push_back(string("."));
	env = getenv("HOME");
	if (env) {
		path_.push_back(string(env) + "/lib/v8");
	}
	path_.push_back(string(PREFIX "/lib/v8"));
	HandleScope handle_scope;
	// Create module.import
	Handle<ObjectTemplate> module_templ = ObjectTemplate::New();
	module_templ->SetInternalFieldCount(1);
	module_templ->Set(String::New("import"), FunctionTemplate::New(Import));
	// Create module.__path__
	Local<ObjectTemplate> path_templ = ObjectTemplate::New();
	path_templ->SetIndexedPropertyHandler(GetPathIndex, SetPathIndex,
			QueryPathIndex, DeletePathIndex, EnumeratePathIndex);
	path_templ->SetInternalFieldCount(1);
	Local<Object> path = path_templ->NewInstance();
	path->SetInternalField(0, External::New(this));
	module_templ->Set(String::NewSymbol("__path__"), path);
	// Create module
	Local<Object> module = module_templ->NewInstance();
	module->SetInternalField(0, External::New(this));
	context_->Global()->Set(String::New("module"), module);
	initialized_ = true;
	return true;
}

const char* Module::Exception() const
{
	return exception_;
}

void Module::SetException(const char* exception)
{
	strcpy(exception_, exception);
}

Handle<Value> Module::Import(const Arguments& args)
{
	if (args.Length() != 1) {
		return ThrowException(String::New(
				"import: a single argument is required"));
	}
	if (!args[0]->IsString()) {
		return ThrowException(String::New(
				"import: argument 1 must be a string"));
	}
	//Module& module = New();
	// ...
	//initialize(templ, module.argc_, module.argv_);
	return Undefined();
}

Handle<Value> Module::GetPathIndex(uint32_t index, const AccessorInfo& info)
{
	Local<Object> object = info.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	if (index < module->path_.size()) {
		if (module->path_[index].length()) {
			return String::New(module->path_[index].c_str());
		}
	}
	return Undefined();
}

Handle<Value> Module::SetPathIndex(uint32_t index, Local<Value> value,
		const AccessorInfo& info)
{
	Local<Object> object = info.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	if (module->path_.size() < index + 1) {
		module->path_.resize(index + 1);
	}
	module->path_[index] = string(*String::Utf8Value(value));
	return value;
}

Handle<Integer> Module::QueryPathIndex(uint32_t index,
		const AccessorInfo &info)
{
	Local<Object> object = info.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	if (index < module->path_.size()) {
		if (module->path_[index].length()) {
			HandleScope scope;
			return scope.Close(Integer::New(None));
		}
	}
	return Handle<Integer>();
}

Handle<Boolean> Module::DeletePathIndex(uint32_t index,
		const AccessorInfo &info)
{
	Local<Object> object = info.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	if (index < module->path_.size()) {
		if (module->path_[index].length()) {
			module->path_[index].clear();
			return True();
		}
	}
	return False();
}

Handle<Array> Module::EnumeratePathIndex(const AccessorInfo& info)
{
	Local<Object> object = info.Holder();
	Local<External> external =
		Local<External>::Cast(object->GetInternalField(0));
	Module* module = static_cast<Module*>(external->Value());
	Local<Array> path = Array::New(module->path_.size());
	int i = 0;
	for (vector<string>::iterator iter = module->path_.begin();
			iter < module->path_.end(); ++iter) {
		if ((*iter).length()) {
			path->Set(i++, String::New((*iter).c_str()));
		} else {
			path->Set(i++, Undefined());
		}
	}
	return path;
}
