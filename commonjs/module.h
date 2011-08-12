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

#ifndef COMMONJS_MODULE_H
#define COMMONJS_MODULE_H

#include <map>
#include <string>
#include <v8.h>

namespace commonjs {

/**
 * A CommonJS 1.1 module loader
 */
class Module {
public:
	Module();

	~Module();

	const char* Exception() const;

	bool Initialize(int* argc, char*** argv);

private:
	std::string exception_;

	bool initialized_;

	v8::Handle<v8::Context> context_;

	int *argc_;

	char ***argv_;

	v8::Persistent<v8::Array> path_;

	std::map<std::string, struct module*> modules_;

	Module(Module const& that);

	void operator=(Module const& that);

	static v8::Handle<v8::Value> Require(const v8::Arguments& args);

	v8::Handle<v8::Value> RequireSharedObject(const char* module,
			const char* path);

	v8::Handle<v8::Value> RequireScript(const char* module,
			const char* path);

	v8::Handle<v8::Value> RequireModule(const char* module,
			const char* path);
};

typedef v8::Handle<v8::Object> (*InitializeCallback)(int* argc, char*** argv);

struct module {
	int version_major;
	int version_minor;
	InitializeCallback initialize;
	v8::Persistent<v8::Object> object;
	void* handle;
};

}

#define COMMONJS_MODULE_VERSION_MAJOR (1)

#define COMMONJS_MODULE_VERSION_MINOR (1)

#define COMMONJS_MODULE(name, initialize) \
extern "C" { \
	commonjs::module name## _module = { \
		COMMONJS_MODULE_VERSION_MAJOR, \
		COMMONJS_MODULE_VERSION_MINOR, \
		initialize, \
		v8::Persistent<v8::Object>(), \
		0 \
	}; \
}

#endif // COMMONJS_MODULE_H
