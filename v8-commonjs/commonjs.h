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

/**
 * \file
 * \brief The V8-CommonJS API
 *
 * \mainpage V8-CommonJS API Reference
 *
 * V8-CommonJS is an implementation of the CommonJS specification in C++
 * for Google's V8 JavaScript engine. This API is meant to be used by
 * embedder's of the V8 API.
 *
 * For more information and source code see https://github.com/msteinert/v8-commonjs
 */

#ifndef V8_COMMONJS_H
#define V8_COMMONJS_H

// Include the module API
#include <v8-commonjs/module.h>

// Include the module loader API
#include <v8-commonjs/module-loader.h>

// Include the V8 API
#include <v8.h>

namespace commonjs {

/**
 * \brief Load a module from C++.
 *
 * This function calls the require function in the current execution context.
 *
 * \note The result of calling this function without initializing the module
 *       loader is undefined.
 *
 * \param name [in] The name of the module to load.
 *
 * \return The exports from the loaded module.
 */
v8::Handle<v8::Value> Require(v8::Handle<v8::String> name);

}

#endif // V8_COMMONJS_H

// vim: tabstop=2:sw=2:expandtab
