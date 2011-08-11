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

#ifndef V8_MODULE_H
#define V8_MODULE_H

#include <map>
#include <string>
#include <vector>
#include <v8.h>

namespace v8 {

/**
 * The v8 JavaScript module loader
 */
class Module {
public:
	Module();

	~Module();

	const char* Exception() const;

	bool Initialize(int argc, char *argv[]);

private:
	char exception_[256];

	bool initialized_;

	Local<Context> context_;

	int argc_;

	char **argv_;

	std::vector<std::string> path_;

	std::map<std::string, void*> handles_;

	Module(Module const&);

	void operator=(Module const&);

	void SetException(const char* exception);

	static Handle<Value> Import(const Arguments& args);

	static Handle<Value> GetPathIndex(uint32_t index,
			const AccessorInfo &info);

	static Handle<Value> SetPathIndex(uint32_t index, Local<Value> value,
			const AccessorInfo& info);

	static Handle<Integer> QueryPathIndex(uint32_t index,
			const AccessorInfo &info);

	static Handle<Boolean> DeletePathIndex(uint32_t index,
			const AccessorInfo &info);

	static Handle<Array> EnumeratePathIndex(const AccessorInfo& info);
};

typedef Handle<Object> (*InitializeCallback)(Handle<Template> templ, int argc,
		char *argv[]);
}

#endif // V8_MODULE_H
