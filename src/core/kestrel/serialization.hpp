//
// Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an "AS
// IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language
// governing permissions and limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
//

#ifndef KESTREL_SERIALIZATION_HPP
#define KESTREL_SERIALIZATION_HPP

#include <stdexcept>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/from_varint.hpp>
#include <sst/catalog/is_integer.hpp>
#include <sst/catalog/span_t.hpp>
#include <sst/catalog/to_varint.hpp>
#include <sst/catalog/unsigned_gt.hpp>
#include <sst/catalog/varint_size.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/carma/mailbox_message_type_t.hpp>
#include <kestrel/psn_t.hpp>

// TODO: Put the inline functions into a .cpp file?

// TODO: These ideas can be generalized into SST.

namespace kestrel {

//
// serialize() performs "lightweight" serialization.
//
// The first argument is the output buffer. You should always pass a
// byte vector for this argument. If you pass an lvalue reference, the
// function will return the same reference back to you. If you pass an
// rvalue reference, the function will use it to move construct and
// return a new byte vector.
//
// The second and further arguments are the objects to serialize. The
// function serializes each object in turn, appending to the end of the
// output buffer.
//

//
// serialize_size() can be called with the same parameters as
// serialize(), minus the first parameter, and returns the number of
// bytes that would have been appended. The return value is wrapped in a
// struct to allow serialize_size() calls to be composed.
//

//
// deserialize() performs "lightweight" deserialization.
//
// The first argument is the input buffer. You should always pass a byte
// vector for this argument. The second argument is the index in the
// input buffer to start reading from.
//
// The third and further arguments are the objects into which to
// deserialize. The function deserializes into each object in turn,
// updating the input buffer index as it moves along.
//

//----------------------------------------------------------------------

struct serialize_size_t {
  bytes_t::size_type value;
};

//----------------------------------------------------------------------

inline serialize_size_t serialize_size() {
  return {static_cast<bytes_t::size_type>(0)};
}

inline bytes_t & serialize(bytes_t & dst) {
  bytes_t::size_type const old_dst_size = dst.size();
  SST_ASSERT((serialize_size().value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src, bytes_t::size_type & idx) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  SST_ASSERT((serialize_size().value == idx - old_idx));
}

//----------------------------------------------------------------------

template<class Src, sst::enable_if_t<sst::is_integer<Src>::value> = 0>
serialize_size_t serialize_size(Src const src) {
  auto const a = sst::varint_size(src);
  return {sst::checked_cast<bytes_t::size_type>(a)};
}

template<class Src, sst::enable_if_t<sst::is_integer<Src>::value> = 0>
bytes_t & serialize(bytes_t & dst, Src const src) {
  bytes_t::size_type const old_dst_size = dst.size();
  dst.resize(sst::checked_cast<decltype(dst.size())>(
      sst::checked(dst.size()) + sst::varint_size(src)));
  sst::to_varint(src, &dst[old_dst_size]);
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

template<class Dst, sst::enable_if_t<sst::is_integer<Dst>::value> = 0>
void deserialize(bytes_t const & src,
                 bytes_t::size_type & idx,
                 Dst & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  // TODO: SST should eventually provide a way to find the size of the
  //       varint that starts with the first byte of a byte sequence.
  bytes_t::size_type i;
  for (i = idx; i < src.size(); ++i) {
    if (!(static_cast<unsigned int>(src[i]) & sst::uchar_msb_v)) {
      break;
    }
  }
  if (i == src.size()) {
    throw std::runtime_error("Corrupt or malicious package data");
  }
  ++i;
  dst = sst::from_varint<Dst>(&src[idx]);
  idx = i;
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------

inline serialize_size_t serialize_size(serialize_size_t const src) {
  auto const a = serialize_size(src.value).value;
  auto const b = src.value;
  return {sst::checked_cast<bytes_t::size_type>(sst::checked(a) + b)};
}

//----------------------------------------------------------------------

template<class T, class N>
serialize_size_t serialize_size(sst::span_t<T, N> const src) {
  auto const a = serialize_size(src.size()).value;
  auto const b = src.size();
  return {sst::checked_cast<bytes_t::size_type>(sst::checked(a) + b)};
}

template<class T, class N>
bytes_t & serialize(bytes_t & dst, sst::span_t<T, N> const src) {
  bytes_t::size_type const old_dst_size = dst.size();
  serialize(dst, src.size());
  dst.insert(dst.end(), src.begin(), src.end());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

//----------------------------------------------------------------------

inline serialize_size_t serialize_size(bytes_t const & src) {
  auto const a = serialize_size(src.size()).value;
  auto const b = src.size();
  return {sst::checked_cast<bytes_t::size_type>(sst::checked(a) + b)};
}

inline bytes_t & serialize(bytes_t & dst, bytes_t const & src) {
  SST_ASSERT((&dst != &src));
  bytes_t::size_type const old_dst_size = dst.size();
  serialize(dst, src.size());
  dst.insert(dst.end(), src.begin(), src.end());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src,
                        bytes_t::size_type & idx,
                        bytes_t & dst) {
  SST_ASSERT((idx < src.size()));
  SST_ASSERT((&src != &dst));
  bytes_t::size_type old_idx = idx;
  decltype(dst.size()) n{};
  deserialize(src, idx, n);
  if (sst::unsigned_gt(n, src.size() - idx)) {
    throw std::runtime_error("Corrupt or malicious package data");
  }
  dst.clear();
  dst.insert(dst.end(), &src[0] + idx, &src[0] + idx + n);
  idx += static_cast<decltype(src.size())>(n);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------

inline serialize_size_t serialize_size(std::string const & src) {
  auto const a = serialize_size(src.size()).value;
  auto const b = src.size();
  return {sst::checked_cast<bytes_t::size_type>(sst::checked(a) + b)};
}

inline bytes_t & serialize(bytes_t & dst, std::string const & src) {
  bytes_t::size_type const old_dst_size = dst.size();
  serialize(dst, src.size());
  dst.insert(dst.end(), src.begin(), src.end());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src,
                        bytes_t::size_type & idx,
                        std::string & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  decltype(dst.size()) n{};
  deserialize(src, idx, n);
  if (sst::unsigned_gt(n, src.size() - idx)) {
    throw std::runtime_error("Corrupt or malicious package data");
  }
  dst.clear();
  dst.insert(dst.end(), &src[0] + idx, &src[0] + idx + n);
  idx += static_cast<decltype(src.size())>(n);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------

inline serialize_size_t serialize_size(psn_t const & src) {
  return serialize_size(src.value());
}

inline bytes_t & serialize(bytes_t & dst, psn_t const & src) {
  bytes_t::size_type const old_dst_size = dst.size();
  serialize(dst, src.value());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src,
                        bytes_t::size_type & idx,
                        psn_t & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  std::string x;
  deserialize(src, idx, x);
  dst = std::move(x);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------

inline serialize_size_t
serialize_size(carma::mailbox_message_type_t const & src) {
  return serialize_size(src.value());
}

inline bytes_t & serialize(bytes_t & dst,
                           carma::mailbox_message_type_t const & src) {
  bytes_t::size_type const old_dst_size = dst.size();
  serialize(dst, src.value());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src,
                        bytes_t::size_type & idx,
                        carma::mailbox_message_type_t & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  carma::mailbox_message_type_t::value_type x;
  deserialize(src, idx, x);
  dst = carma::mailbox_message_type_t(x);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

//----------------------------------------------------------------------
// Variadics
//----------------------------------------------------------------------

template<class Src,
         class... Args,
         sst::enable_if_t<(sizeof...(Args) > 0)> = 0>
serialize_size_t serialize_size(Src const & src, Args &&... args) {
  auto const a = serialize_size(src).value;
  auto const b = serialize_size(std::forward<Args>(args)...).value;
  return {sst::checked_cast<bytes_t::size_type>(sst::checked(a) + b)};
}

template<class Src,
         class... Args,
         sst::enable_if_t<(sizeof...(Args) > 0)> = 0>
bytes_t & serialize(bytes_t & dst, Src const & src, Args &&... args) {
  serialize(dst, src);
  serialize(dst, std::forward<Args>(args)...);
  return dst;
}

template<class... Args>
bytes_t serialize(bytes_t && dst, Args &&... args) {
  serialize(dst, std::forward<Args>(args)...);
  return std::move(dst);
}

template<class Dst,
         class... Args,
         sst::enable_if_t<(sizeof...(Args) > 0)> = 0>
void deserialize(bytes_t const & src,
                 bytes_t::size_type & idx,
                 Dst & dst,
                 Args &&... args) {
  deserialize(src, idx, dst);
  deserialize(src, idx, std::forward<Args>(args)...);
}

//----------------------------------------------------------------------

} // namespace kestrel

#endif // #ifndef KESTREL_SERIALIZATION_HPP
