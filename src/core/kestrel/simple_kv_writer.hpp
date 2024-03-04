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

#ifndef KESTREL_SIMPLE_KV_WRITER_HPP
#define KESTREL_SIMPLE_KV_WRITER_HPP

#include <iterator>
#include <string>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/enable_if_t.hpp>
#include <sst/catalog/is_integer.hpp>
#include <sst/catalog/moved_from.hpp>
#include <sst/catalog/to_string.hpp>

namespace kestrel {

//
// See simple_kv_reader.hpp for the documentation of Simple KV strings.
//

class simple_kv_writer final {

  sst::moved_from moved_from_;

  using string_t = std::string;
  using char_t = typename string_t::value_type;

  string_t buf_;
  string_t dst_;
  bool first_ = true;

  void escape_into_buf(char_t const * p);

public:

  simple_kv_writer() = default;
  simple_kv_writer(simple_kv_writer const &) = delete;
  simple_kv_writer & operator=(simple_kv_writer const &) = delete;
  simple_kv_writer(simple_kv_writer &&) = default;
  simple_kv_writer & operator=(simple_kv_writer &&) = default;
  ~simple_kv_writer() = default;

  void write(char_t const * key, char_t const * value);

  void write(string_t const & key, char_t const * value) {
    write(key.c_str(), value);
  }

  void write(char_t const * key, string_t const & value) {
    write(key, value.c_str());
  }

  void write(string_t const & key, string_t const & value) {
    write(key.c_str(), value.c_str());
  }

  template<class T, sst::enable_if_t<sst::is_integer<T>::value> = 0>
  void write(char_t const * key, T const value) {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(key != nullptr);
    buf_.clear();
    if (!first_) {
      buf_ += static_cast<char_t>(',');
    }
    escape_into_buf(key);
    buf_ += static_cast<char_t>(',');
    sst::to_string(value, std::back_inserter(buf_));
    dst_ += buf_;
    first_ = false;
  }

  template<class T, sst::enable_if_t<sst::is_integer<T>::value> = 0>
  void write(string_t const & key, T const value) {
    write(key.c_str(), value);
  }

  SST_NODISCARD() string_t & get() noexcept {
    SST_ASSERT(!moved_from_);
    return dst_;
  }

  SST_NODISCARD() string_t const & get() const noexcept {
    SST_ASSERT(!moved_from_);
    return dst_;
  }

  void reset() noexcept {
    SST_ASSERT(!moved_from_);
    dst_.clear();
    first_ = true;
  }

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_SIMPLE_KV_WRITER_HPP
