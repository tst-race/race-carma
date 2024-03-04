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

#ifndef KESTREL_STRING_ID_T_HPP
#define KESTREL_STRING_ID_T_HPP

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <string>
#include <type_traits>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/buffer_overrun.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/from_varint.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/json/get_as.hpp>
#include <sst/catalog/old/from_bytes.hpp>
#include <sst/catalog/old/to_bytes.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/perfect_gt.hpp>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_varint.hpp>
#include <sst/catalog/track_it.hpp>
#include <sst/catalog/varint_size.hpp>

#include <kestrel/json_t.hpp>
#include <kestrel/slugify.hpp>

namespace kestrel {

//
// TODO: This class shouldn't need to store duplicate copies of its data
//       in different containers assuming all algorithms are written
//       generally to accept arbitrary byte containers. std::string
//       should be sufficient. This class should then become
//       "std::string but strongly aliased and with extra operations".
//

template<class Tag>
class string_id_t {

  std::vector<unsigned char> bytes_;
  std::string string_;

public:
  string_id_t() = default;
  string_id_t(string_id_t const &) = default;
  string_id_t(string_id_t &&) = default;
  string_id_t & operator=(string_id_t const &) = default;
  string_id_t & operator=(string_id_t &&) = default;
  ~string_id_t() = default;

  //--------------------------------------------------------------------
  // Constructors
  //--------------------------------------------------------------------

  // TODO: Add constructors to take rvalues.

  explicit string_id_t(std::vector<unsigned char> const & src)
      : bytes_(src.begin(), src.end()),
        string_(src.begin(), src.end()) {
  }

  explicit string_id_t(unsigned char const * const src,
                       unsigned char const * const end)
      : bytes_((SST_ASSERT((src != nullptr)), src),
               (SST_ASSERT((end != nullptr)), end)),
        string_(src, end) {
  }

  explicit string_id_t(std::string const & src)
      : bytes_(src.begin(), src.end()),
        string_(src.begin(), src.end()) {
  }

  explicit string_id_t(char const * const src, char const * const end)
      : bytes_((SST_ASSERT((src != nullptr)), src),
               (SST_ASSERT((end != nullptr)), end)),
        string_(src, end) {
  }

  //--------------------------------------------------------------------
  // Views
  //--------------------------------------------------------------------
  //
  // TODO: bytes() and string() should disappear eventually, leaving
  //       only value() remaining. Anything that wants to use bytes()
  //       should be written against (unsigned char *, size_t) instead.
  //       Then we can simply use the innards of the string. Hopefully
  //       there should be no cases where the RACE SDK forces us into
  //       truly needing a std::vector.
  //

  std::vector<unsigned char> const & bytes() const {
    return bytes_;
  }

  std::string const & string() const {
    return string_;
  }

  std::string & value() {
    return string_;
  }

  std::string const & value() const {
    return string_;
  }

  //--------------------------------------------------------------------
  // Assignment operators
  //--------------------------------------------------------------------

  string_id_t & operator=(std::vector<unsigned char> const & src) {
    try {
      bytes_ = src;
      string_.clear();
      string_.insert(string_.end(), src.begin(), src.end());
      return *this;
    } catch (...) {
      string_.clear();
      bytes_.clear();
      throw;
    }
  }

  string_id_t & operator=(std::string const & src) {
    try {
      string_ = src;
      bytes_.clear();
      bytes_.insert(bytes_.end(), src.begin(), src.end());
      return *this;
    } catch (...) {
      string_.clear();
      bytes_.clear();
      throw;
    }
  }

  //--------------------------------------------------------------------
  // Comparison operators
  //--------------------------------------------------------------------

#define CARMA_F(op)                                                    \
  bool operator op(string_id_t const & other) const {                  \
    return bytes_ op other.bytes_;                                     \
  }

  CARMA_F(<)
  CARMA_F(>)
  CARMA_F(<=)
  CARMA_F(>=)
  CARMA_F(==)
  CARMA_F(!=)

#undef CARMA_F

  //--------------------------------------------------------------------
  // Slug conversions
  //--------------------------------------------------------------------

  std::string to_path_slug() const {
    return kestrel::slugify(string_);
  }

  static string_id_t from_path_slug(std::string const & path_slug) {
    return string_id_t(kestrel::unslugify(path_slug));
  }

  //--------------------------------------------------------------------
  // Container functions
  //--------------------------------------------------------------------

  auto begin() const noexcept -> decltype(bytes_.begin()) {
    return bytes_.begin();
  }

  void clear() noexcept {
    bytes_.clear();
    string_.clear();
  }

  auto size() const noexcept -> decltype(bytes_.size()) {
    return bytes_.size();
  }

  //--------------------------------------------------------------------
  // JSON conversions
  //--------------------------------------------------------------------

  friend void from_json(nlohmann::json const & src, string_id_t & dst) {
    dst.string_ =
        kestrel::unslugify(sst::json::get_as<std::string>(src));
    dst.bytes_.clear();
    dst.bytes_.insert(dst.bytes_.begin(),
                      dst.string_.begin(),
                      dst.string_.end());
  }

  friend void to_json(nlohmann::json & dst, string_id_t const & src) {
    dst = kestrel::slugify(src.string_);
  }

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size = std::size_t>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;
    n += sst::varint_size(bytes_.size());
    n += bytes_.size();
    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    dst = sst::to_varint(bytes_.size(), dst);
    dst = std::copy(bytes_.begin(), bytes_.end(), dst);
    return dst;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT((sst::perfect_ge(avail, 0)));
    bytes_.resize(sst::from_varint<decltype(bytes_.size())>(
        sst::track_it(src, &avail)));
    sst::copy_bytes(&src, bytes_.size(), avail, bytes_.begin());
    string_ = std::string(reinterpret_cast<char const *>(bytes_.data()),
                          reinterpret_cast<char const *>(bytes_.data())
                              + bytes_.size());
    return src;
  }

  //--------------------------------------------------------------------
};

} // namespace kestrel

//----------------------------------------------------------------------
// Hashing
//----------------------------------------------------------------------

namespace std {

template<class Tag>
struct hash<kestrel::string_id_t<Tag>> {
  std::size_t operator()(kestrel::string_id_t<Tag> const & src) const {
    return std::hash<std::string>()(src.string());
  }
};

} // namespace std

#endif // #ifndef KESTREL_STRING_ID_T_HPP
