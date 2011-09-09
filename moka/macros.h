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
 * \brief Internally used macros
 */

#ifndef MOKA_MACROS_H
#define MOKA_MACROS_H

#include <endian.h>

#if defined(__GNUC__) && (__GNUC__ >= 4)
/// \brief Control symbol visibility
#define MOKA_EXPORT __attribute__ ((visibility("default")))
#else
#define MOKA_EXPORT
#endif

#if BYTE_ORDER == LITTLE_ENDIAN
#define MOKA_INT16_LITTLE_ENDIAN(bytes) \
  static_cast<int16_t>(bytes[0] << 8 | bytes[1])
#define MOKA_INT16_BIG_ENDIAN(bytes) \
  static_cast<int16_t>(bytes[1] << 8 | bytes[0])
#elif BYTE_ORDER == BIG_ENDIAN
#define MOKA_INT16_LITTLE_ENDIAN(bytes) \
  static_cast<int16_t>(bytes[1] << 8 | bytes[0])
#define MOKA_INT16_BIG_ENDIAN(bytes) \
  static_cast<int16_t>(bytes[0] << 8 | bytes[1])
#else
#error Unknown byte order
#endif

#endif // MOKA_MACROS_H

// vim: tabstop=2:sw=2:expandtab
