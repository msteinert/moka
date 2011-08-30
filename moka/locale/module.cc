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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <clocale>
#include <langinfo.h>
#include "moka/locale/locale.h"
#include "moka/module.h"

namespace moka {

namespace locale {

// Initialize module
static v8::Handle<v8::Value> Initialize(int* argc, char*** argv) {
  v8::HandleScope handle_scope;
  v8::Handle<v8::Value> value = Module::Exports();
  if (value.IsEmpty() || value->IsUndefined()) {
    return handle_scope.Close(value);
  }
  v8::Handle<v8::Object> exports = value->ToObject();
  // Locale functions
  exports->Set(v8::String::NewSymbol("setlocale"),
      v8::FunctionTemplate::New(SetLocale)->GetFunction());
  exports->Set(v8::String::NewSymbol("localeconv"),
      v8::FunctionTemplate::New(LocaleConv)->GetFunction());
  // Locale constants
  exports->Set(v8::String::NewSymbol("LC_ALL"), v8::Number::New(LC_ALL));
  exports->Set(v8::String::NewSymbol("LC_COLLATE"), v8::Int32::New(LC_COLLATE));
  exports->Set(v8::String::NewSymbol("LC_CTYPE"), v8::Int32::New(LC_CTYPE));
  exports->Set(v8::String::NewSymbol("LC_MESSAGES"),
      v8::Int32::New(LC_MESSAGES));
  exports->Set(v8::String::NewSymbol("LC_MONETARY"),
      v8::Int32::New(LC_MONETARY));
  exports->Set(v8::String::NewSymbol("LC_NUMERIC"), v8::Int32::New(LC_NUMERIC));
  exports->Set(v8::String::NewSymbol("LC_TIME"), v8::Int32::New(LC_TIME));
  // Langinfo functions
  exports->Set(v8::String::NewSymbol("nl_langinfo"),
      v8::FunctionTemplate::New(NlLangInfo)->GetFunction());
  // Langinfo constants
  exports->Set(v8::String::NewSymbol("CODESET"), v8::Int32::New(CODESET));
  exports->Set(v8::String::NewSymbol("D_T_FMT"), v8::Int32::New(D_T_FMT));
  exports->Set(v8::String::NewSymbol("D_FMT"), v8::Int32::New(D_FMT));
  exports->Set(v8::String::NewSymbol("T_FMT"), v8::Int32::New(T_FMT));
  exports->Set(v8::String::NewSymbol("T_FMT_AMPM"), v8::Int32::New(T_FMT_AMPM));
  exports->Set(v8::String::NewSymbol("DAY_1"), v8::Int32::New(DAY_1));
  exports->Set(v8::String::NewSymbol("DAY_2"), v8::Int32::New(DAY_2));
  exports->Set(v8::String::NewSymbol("DAY_3"), v8::Int32::New(DAY_3));
  exports->Set(v8::String::NewSymbol("DAY_4"), v8::Int32::New(DAY_4));
  exports->Set(v8::String::NewSymbol("DAY_5"), v8::Int32::New(DAY_5));
  exports->Set(v8::String::NewSymbol("DAY_6"), v8::Int32::New(DAY_6));
  exports->Set(v8::String::NewSymbol("DAY_7"), v8::Int32::New(DAY_7));
  exports->Set(v8::String::NewSymbol("ABDAY_1"), v8::Int32::New(ABDAY_1));
  exports->Set(v8::String::NewSymbol("ABDAY_2"), v8::Int32::New(ABDAY_2));
  exports->Set(v8::String::NewSymbol("ABDAY_3"), v8::Int32::New(ABDAY_3));
  exports->Set(v8::String::NewSymbol("ABDAY_4"), v8::Int32::New(ABDAY_4));
  exports->Set(v8::String::NewSymbol("ABDAY_5"), v8::Int32::New(ABDAY_5));
  exports->Set(v8::String::NewSymbol("ABDAY_6"), v8::Int32::New(ABDAY_6));
  exports->Set(v8::String::NewSymbol("ABDAY_7"), v8::Int32::New(ABDAY_7));
  exports->Set(v8::String::NewSymbol("MON_1"), v8::Int32::New(MON_1));
  exports->Set(v8::String::NewSymbol("MON_2"), v8::Int32::New(MON_2));
  exports->Set(v8::String::NewSymbol("MON_3"), v8::Int32::New(MON_3));
  exports->Set(v8::String::NewSymbol("MON_4"), v8::Int32::New(MON_4));
  exports->Set(v8::String::NewSymbol("MON_5"), v8::Int32::New(MON_5));
  exports->Set(v8::String::NewSymbol("MON_6"), v8::Int32::New(MON_6));
  exports->Set(v8::String::NewSymbol("MON_7"), v8::Int32::New(MON_7));
  exports->Set(v8::String::NewSymbol("MON_8"), v8::Int32::New(MON_8));
  exports->Set(v8::String::NewSymbol("MON_9"), v8::Int32::New(MON_9));
  exports->Set(v8::String::NewSymbol("MON_10"), v8::Int32::New(MON_10));
  exports->Set(v8::String::NewSymbol("MON_11"), v8::Int32::New(MON_11));
  exports->Set(v8::String::NewSymbol("MON_12"), v8::Int32::New(MON_12));
  exports->Set(v8::String::NewSymbol("ABMON_1"), v8::Int32::New(ABMON_1));
  exports->Set(v8::String::NewSymbol("ABMON_2"), v8::Int32::New(ABMON_2));
  exports->Set(v8::String::NewSymbol("ABMON_3"), v8::Int32::New(ABMON_3));
  exports->Set(v8::String::NewSymbol("ABMON_4"), v8::Int32::New(ABMON_4));
  exports->Set(v8::String::NewSymbol("ABMON_5"), v8::Int32::New(ABMON_5));
  exports->Set(v8::String::NewSymbol("ABMON_6"), v8::Int32::New(ABMON_6));
  exports->Set(v8::String::NewSymbol("ABMON_7"), v8::Int32::New(ABMON_7));
  exports->Set(v8::String::NewSymbol("ABMON_8"), v8::Int32::New(ABMON_8));
  exports->Set(v8::String::NewSymbol("ABMON_9"), v8::Int32::New(ABMON_9));
  exports->Set(v8::String::NewSymbol("ABMON_10"), v8::Int32::New(ABMON_10));
  exports->Set(v8::String::NewSymbol("ABMON_11"), v8::Int32::New(ABMON_11));
  exports->Set(v8::String::NewSymbol("ABMON_12"), v8::Int32::New(ABMON_12));
  exports->Set(v8::String::NewSymbol("RADIXCHAR"), v8::Int32::New(RADIXCHAR));
  exports->Set(v8::String::NewSymbol("THOUSEP"), v8::Int32::New(THOUSEP));
  exports->Set(v8::String::NewSymbol("YESEXPR"), v8::Int32::New(YESEXPR));
  exports->Set(v8::String::NewSymbol("NOEXPR"), v8::Int32::New(NOEXPR));
  exports->Set(v8::String::NewSymbol("CRNCYSTR"), v8::Int32::New(CRNCYSTR));
  exports->Set(v8::String::NewSymbol("ERA"), v8::Int32::New(ERA));
  exports->Set(v8::String::NewSymbol("ERA_D_T_FMT"),
      v8::Int32::New(ERA_D_T_FMT));
  exports->Set(v8::String::NewSymbol("ERA_D_FMT"), v8::Int32::New(ERA_D_FMT));
  exports->Set(v8::String::NewSymbol("ERA_T_FMT"), v8::Int32::New(ERA_T_FMT));
  exports->Set(v8::String::NewSymbol("ALT_DIGITS"), v8::Int32::New(ALT_DIGITS));
  return handle_scope.Close(value);
}

} // namespace locale

} // namespace moka

MOKA_MODULE(moka::locale::Initialize)

// vim: tabstop=2:sw=2:expandtab
