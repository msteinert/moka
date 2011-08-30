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

#ifndef MOKA_LOCALE_H
#define MOKA_LOCALE_H

#include "moka/module.h"

namespace moka {

namespace locale {

v8::Handle<v8::Value> SetLocale(const v8::Arguments& arguments);

v8::Handle<v8::Value> LocaleConv(const v8::Arguments& arguments);

v8::Handle<v8::Value> NlLangInfo(const v8::Arguments& arguments);

} // namespace locale

} // namespace moka

#endif // MOKA_LOCALE_H

// vim: tabstop=2:sw=2:expandtab
