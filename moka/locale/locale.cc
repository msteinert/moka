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
#include <cstdlib>
#include <cstring>
#include <clocale>
#include <langinfo.h>
#include "moka/locale/locale.h"
#include <sstream>

namespace moka {

namespace locale {

v8::Handle<v8::Value> SetLocale(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  int category;
  v8::Handle<v8::String> locale;
  switch (arguments.Length()) {
  case 2:
    if (!arguments[1]->IsUndefined()) {
      if (!arguments[1]->IsString()) {
        return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
                v8::String::New("Argument two must be a string"))));
      }
      locale = arguments[1]->ToString();
    }
    // Fall through
  case 1:
    if (!arguments[0]->IsInt32()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an integer"))));
    }
    category = arguments[0]->ToInt32()->Value();
    break;
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One or two arguments allowed"))));
  }
  char* current = NULL;
  if (locale.IsEmpty()) {
    current = ::setlocale(category, NULL);
  } else {
    current = ::setlocale(category, *v8::String::AsciiValue(locale));
  }
  if (current) {
    return handle_scope.Close(v8::String::New(current));
  } else {
    return handle_scope.Close(v8::Undefined());
  }
}

v8::Handle<v8::Value> LocaleConv(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  switch (arguments.Length()) {
  case 0:
    // Do nothing
    break;
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("Zero arguments allowed"))));
  }
  struct lconv* conv = localeconv();
  if (!conv) {
    return handle_scope.Close(v8::Undefined());
  }
  v8::Local<v8::Object> object = v8::Object::New();
  // Numeric settings
  v8::Local<v8::Object> numeric = v8::Object::New();
  numeric->Set(v8::String::NewSymbol("decimal_point"),
      conv->decimal_point
        ? v8::String::New(conv->decimal_point)
        : v8::Undefined());
  numeric->Set(v8::String::NewSymbol("thousands_sep"),
      conv->thousands_sep
        ? v8::String::New(conv->thousands_sep)
        : v8::Undefined());
  numeric->Set(v8::String::NewSymbol("grouping"),
      conv->grouping
        ? v8::String::New(conv->grouping)
        : v8::Undefined());
  object->Set(v8::String::NewSymbol("LC_NUMERIC"), numeric);
  // Monetary settings
  v8::Local<v8::Object> monetary = v8::Object::New();
  monetary->Set(v8::String::NewSymbol("int_curr_symbol"),
      conv->int_curr_symbol
        ? v8::String::New(conv->int_curr_symbol)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("currency_symbol"),
      conv->currency_symbol
        ? v8::String::New(conv->currency_symbol)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("mon_decimal_point"),
      conv->mon_decimal_point
        ? v8::String::New(conv->mon_decimal_point)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("mon_thousands_sep"),
      conv->mon_thousands_sep
        ? v8::String::New(conv->mon_thousands_sep)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("mon_grouping"),
      conv->mon_grouping
        ? v8::String::New(conv->mon_grouping)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("positive_sign"),
      conv->positive_sign
        ? v8::String::New(conv->positive_sign)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("negative_sign"),
      conv->negative_sign
        ? v8::String::New(conv->negative_sign)
        : v8::Undefined());
  monetary->Set(v8::String::NewSymbol("int_frac_digits"),
      v8::Number::New(conv->int_frac_digits));
  monetary->Set(v8::String::NewSymbol("p_cs_precedes"),
      v8::Boolean::New(conv->p_cs_precedes ? true : false));
  monetary->Set(v8::String::NewSymbol("p_sep_by_space"),
      v8::Boolean::New(conv->p_sep_by_space ? true : false));
  monetary->Set(v8::String::NewSymbol("n_cs_precedes"),
      v8::Boolean::New(conv->n_cs_precedes ? true : false));
  monetary->Set(v8::String::NewSymbol("n_sep_by_space"),
      v8::Boolean::New(conv->n_sep_by_space ? true : false));
  monetary->Set(v8::String::NewSymbol("p_sign_posn"),
      v8::Number::New(conv->p_sign_posn));
  monetary->Set(v8::String::NewSymbol("n_sign_posn"),
      v8::Number::New(conv->n_sign_posn));
  object->Set(v8::String::NewSymbol("LC_MONETARY"), monetary);
  return handle_scope.Close(object);
}

v8::Handle<v8::Value> NlLangInfo(const v8::Arguments& arguments) {
  v8::HandleScope handle_scope;
  nl_item item;
  switch (arguments.Length()) {
  case 1:
    if (!arguments[0]->IsInt32()) {
      return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
              v8::String::New("Argument one must be an integer"))));
    }
    item = arguments[0]->ToInt32()->Value();
    break;
  default:
    return handle_scope.Close(v8::ThrowException(v8::Exception::TypeError(
            v8::String::New("One argument allowed"))));
  }
  char* info = nl_langinfo(item);
  if (info) {
    return handle_scope.Close(v8::String::New(info));
  } else {
    return handle_scope.Close(v8::Undefined());
  }
}

} // namespace locale

} // namespace moka

// vim: tabstop=2:sw=2:expandtab
