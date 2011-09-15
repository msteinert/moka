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

#ifndef MOKA_BYTES_H
#define MOKA_BYTES_H

#include <endian.h>

namespace moka {

namespace bytes {

template<typename T, size_t size> struct Swap;

template<typename T, size_t size> struct NoSwap;

template<typename T, int endianness> struct Get;

template<typename T, int endianness> struct Set;

template<typename T> struct Swap<T, 2>;

template<typename T> struct Swap<T, 4>;

template<> struct Swap<float, 4>;

template<typename T> struct Swap<T, 8>;

template<> struct Swap<double, 8>;

template<typename T> struct NoSwap<T, 2>;

template<typename T> struct NoSwap<T, 4>;

template<> struct NoSwap<float, 4>;

template<typename T> struct NoSwap<T, 8>;

template<> struct NoSwap<double, 8>;

template<typename T> struct Get<T, LITTLE_ENDIAN>;

template<typename T> struct Get<T, BIG_ENDIAN>;

template<typename T> struct Set<T, LITTLE_ENDIAN>;

template<typename T> struct Set<T, BIG_ENDIAN>;

} // namespace bytes

} // namespace moka

template<typename T, size_t size>
struct moka::bytes::Swap {
  inline T operator()(int8_t* bytes);
  inline void operator()(T value, int8_t* bytes);
};

template<typename T, size_t size>
struct moka::bytes::NoSwap {
  inline T operator()(int8_t* bytes);
  inline void operator()(T value, int8_t* bytes);
};

template<typename T, int endianness>
struct moka::bytes::Get {
  inline T operator()(int8_t* bytes);
};

template<typename T, int endianness>
struct moka::bytes::Set {
  inline void operator()(T value, int8_t* bytes);
};

template<typename T>
struct moka::bytes::Swap<T, 2> {
  inline T operator()(int8_t* bytes) {
    return bytes[0]
      | bytes[1] << 8;
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0x00ff);
    bytes[1] = (value & 0xff00) >> 8;
  }
};

template<typename T>
struct moka::bytes::Swap<T, 4> {
  inline T operator()(int8_t* bytes) {
    return bytes[0]
      | bytes[1] << 8
      | bytes[2] << 16
      | bytes[3] << 24;
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0x000000ff);
    bytes[1] = (value & 0x0000ff00) >> 8;
    bytes[2] = (value & 0x00ff0000) >> 16;
    bytes[3] = (value & 0xff000000) >> 24;
  }
};

template<>
struct moka::bytes::Swap<float, 4> {
  inline float operator()(int8_t* bytes) {
    uint32_t value = Swap<uint32_t, sizeof(uint32_t)>()(bytes);
    float* value_ptr = reinterpret_cast<float*>(&value);
    return *value_ptr;
  }
  inline void operator()(float value, int8_t* bytes) {
    uint32_t *value_ptr = reinterpret_cast<uint32_t*>(&value);
    Swap<uint32_t, sizeof(uint32_t)>()(*value_ptr, bytes);
  }
};

template<typename T>
struct moka::bytes::Swap<T, 8> {
  inline T operator()(int8_t* bytes) {
    return bytes[0]
      | bytes[1] << 8
      | bytes[2] << 16
      | bytes[3] << 24
      | static_cast<uint64_t>(bytes[4]) << 32
      | static_cast<uint64_t>(bytes[5]) << 40
      | static_cast<uint64_t>(bytes[6]) << 48
      | static_cast<uint64_t>(bytes[7]) << 56;
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0x00000000000000ff);
    bytes[1] = (value & 0x000000000000ff00) >> 8;
    bytes[2] = (value & 0x0000000000ff0000) >> 16;
    bytes[3] = (value & 0x00000000ff000000) >> 24;
    bytes[4] = (value & 0x000000ff00000000) >> 32;
    bytes[5] = (value & 0x0000ff0000000000) >> 40;
    bytes[6] = (value & 0x00ff000000000000) >> 48;
    bytes[7] = (value & 0xff00000000000000) >> 56;
  }
};

template<>
struct moka::bytes::Swap<double, 8> {
  inline double operator()(int8_t* bytes) {
    uint64_t value = Swap<uint64_t, sizeof(uint64_t)>()(bytes);
    double* value_ptr = reinterpret_cast<double*>(&value);
    return *value_ptr;
  }
  inline void operator()(double value, int8_t* bytes) {
    uint64_t *value_ptr = reinterpret_cast<uint64_t*>(&value);
    Swap<uint64_t, sizeof(uint64_t)>()(*value_ptr, bytes);
  }
};

template<typename T>
struct moka::bytes::NoSwap<T, 2> {
  inline T operator()(int8_t* bytes) {
    return bytes[0] << 8
      | bytes[1];
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0xff00) >> 8;
    bytes[1] = (value & 0x00ff);
  }
};

template<typename T>
struct moka::bytes::NoSwap<T, 4> {
  inline T operator()(int8_t* bytes) {
    return bytes[0] << 24 
      | bytes[1] << 16
      | bytes[2] << 8
      | bytes[3];
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0xff000000) >> 24;
    bytes[1] = (value & 0x00ff0000) >> 16;
    bytes[2] = (value & 0x0000ff00) >> 8;
    bytes[3] = (value & 0x000000ff);
  }
};

template<>
struct moka::bytes::NoSwap<float, 4> {
  inline float operator()(int8_t* bytes) {
    uint32_t value = NoSwap<uint32_t, sizeof(uint32_t)>()(bytes);
    float* value_ptr = reinterpret_cast<float*>(&value);
    return *value_ptr;
  }
  inline void operator()(float value, int8_t* bytes) {
    uint32_t *value_ptr = reinterpret_cast<uint32_t*>(&value);
    NoSwap<uint32_t, sizeof(uint32_t)>()(*value_ptr, bytes);
  }
};

template<typename T>
struct moka::bytes::NoSwap<T, 8> {
  inline T operator()(int8_t* bytes) {
    return static_cast<uint64_t>(bytes[0]) << 56
      | static_cast<uint64_t>(bytes[1]) << 48
      | static_cast<uint64_t>(bytes[2]) << 40
      | static_cast<uint64_t>(bytes[3]) << 32
      | bytes[4] << 24
      | bytes[5] << 16
      | bytes[6] << 8
      | bytes[7];
  }
  inline void operator()(T value, int8_t* bytes) {
    bytes[0] = (value & 0xff00000000000000) >> 56;
    bytes[1] = (value & 0x00ff000000000000) >> 48;
    bytes[2] = (value & 0x0000ff0000000000) >> 40;
    bytes[3] = (value & 0x000000ff00000000) >> 32;
    bytes[4] = (value & 0x00000000ff000000) >> 24;
    bytes[5] = (value & 0x0000000000ff0000) >> 16;
    bytes[6] = (value & 0x000000000000ff00) >> 8;
    bytes[7] = (value & 0x00000000000000ff);
  }
};

template<>
struct moka::bytes::NoSwap<double, 8> {
  inline double operator()(int8_t* bytes) {
    uint64_t value = NoSwap<uint64_t, sizeof(uint64_t)>()(bytes);
    double* value_ptr = reinterpret_cast<double*>(&value);
    return *value_ptr;
  }
  inline void operator()(double value, int8_t* bytes) {
    uint64_t *value_ptr = reinterpret_cast<uint64_t*>(&value);
    NoSwap<uint64_t, sizeof(uint64_t)>()(*value_ptr, bytes);
  }
};

template<typename T>
struct moka::bytes::Get<T, LITTLE_ENDIAN> {
#if BYTE_ORDER == LITTLE_ENDIAN
  inline T operator()(int8_t* bytes) {
    return NoSwap<T, sizeof(T)>()(bytes);
  }
#elif BYTE_ORDER == BIG_ENDIAN
  inline T operator()(int8_t* bytes) {
    return Swap<T, sizeof(T)>()(bytes);
  }
#else
#error Unknown byte order
#endif
};

template<typename T>
struct moka::bytes::Get<T, BIG_ENDIAN> {
#if BYTE_ORDER == LITTLE_ENDIAN
  inline T operator()(int8_t* bytes) {
    return Swap<T, sizeof(T)>()(bytes);
  }
#elif BYTE_ORDER == BIG_ENDIAN
  inline T operator()(int8_t* bytes) {
    return NoSwap<T, sizeof(T)>()(bytes);
  }
#else
#error Unknown byte order
#endif
};

template<typename T>
struct moka::bytes::Set<T, LITTLE_ENDIAN> {
#if BYTE_ORDER == LITTLE_ENDIAN
  inline void operator()(T value, int8_t* bytes) {
    NoSwap<T, sizeof(T)>()(value, bytes);
  }
#elif BYTE_ORDER == BIG_ENDIAN
  inline void operator()(T value, int8_t* bytes) {
    Swap<T, sizeof(T)>()(value, bytes);
  }
#else
#error Unknown byte order
#endif
};

template<typename T>
struct moka::bytes::Set<T, BIG_ENDIAN> {
#if BYTE_ORDER == LITTLE_ENDIAN
  inline void operator()(T value, int8_t* bytes) {
    Swap<T, sizeof(T)>()(value, bytes);
  }
#elif BYTE_ORDER == BIG_ENDIAN
  inline void operator()(T value, int8_t* bytes) {
    NoSwap<T, sizeof(T)>()(value, bytes);
  }
#else
#error Unknown byte order
#endif
};

#endif // MOKA_BYTES_H

// vim: tabstop=2:sw=2:expandtab
