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
#include "commonjs/module.h"
#include <cstdio>
#include <cstring>
#include <libgen.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace commonjs;
using namespace std;
using namespace v8;

Handle<Value> Print(const Arguments& args)
{
	for (int i = 0; i < args.Length(); ++i) {
		HandleScope handle_scope;
		if (i != 0) {
			fputc(' ', stdout);
		}
		String::Utf8Value s(args[i]);
		fwrite(*s, sizeof(**s), s.length(), stdout);
	}
	fputc('\n', stdout);
	fflush(stdout);
	return Undefined();
}

void Report(TryCatch& try_catch)
{
	HandleScope handle_scope;
	String::Utf8Value exception(try_catch.Exception());
	Handle<Message> message = try_catch.Message();
	if (message.IsEmpty()) {
		printf("%s\n", *exception);
		return;
	}
	String::Utf8Value filename(message->GetScriptResourceName());
	int line = message->GetLineNumber();
	printf("[%s:%d] %s\n", *filename, line, *exception);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "error: %s <script>\n", basename(argv[0]));
		return 1;
	}
	// Create a stack allocated handle scope
	HandleScope handle_scope;
	// Create a global object template
	Handle<ObjectTemplate> global_template = ObjectTemplate::New();
	// Add print function
	global_template->Set(String::New("print"),
			FunctionTemplate::New(Print));
	// Create a new context
	Persistent<Context> context = Context::New(0, global_template);
	// Enter the created context
	Context::Scope scope(context);
	// Initialize module loader
	Module module;
	if (!module.Initialize(&argc, &argv)) {
		fprintf(stderr, "error: module loader: %s\n",
				module.Exception());
		context.Dispose();
		return 1;
	}
	// Read file into a v8 string
	FILE* file = fopen(argv[1], "rb");
	if (not file) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		context.Dispose();
		return 1;
	}
	struct stat buf;
	if (fstat(fileno(file), &buf)) {
		fprintf(stderr, "error: %s\n", strerror(errno));
		context.Dispose();
		fclose(file);
		return 1;
	}
	if (!S_ISREG(buf.st_mode)) {
		fprintf(stderr, "error: '%s' is not a regular file\n", argv[1]);
		context.Dispose();
		fclose(file);
		return 1;
	}
	int size = buf.st_size;
	char* characters = new char[size + 1];
	for (int i = 0; i < size;) {
		int read = static_cast<int>(fread(&characters[i], 1, size - i,
					file));
		i += read;
	}
	fclose(file);
	Handle<String> source = String::New(characters, size);
	delete[] characters;
	if (source.IsEmpty()) {
		fprintf(stderr, "error: failed to read %s\n", argv[1]);
		context.Dispose();
		return 1;
	}
	// Compile string
	TryCatch try_catch;
	Handle<Script> script = Script::Compile(source, String::New(argv[1]));
	if (script.IsEmpty()) {
		Report(try_catch);
		context.Dispose();
		return 1;
	}
	// Execute string
	Handle<Value> result = script->Run();
	if (result.IsEmpty()) {
		Report(try_catch);
		context.Dispose();
		return 1;
	}
	// Clean up
	context.Dispose();
	V8::Dispose();
	return 0;
}
