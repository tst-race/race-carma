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

#ifndef KESTREL_SIMPLE_KV_READER_HPP
#define KESTREL_SIMPLE_KV_READER_HPP

#include <stdexcept>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_NODISCARD.h>
#include <sst/catalog/moved_from.hpp>

namespace kestrel {

//
// This class reads key-value pairs from a "Simple KV" string.
//
// Here is an example of a Simple KV string and its equivalent JSON:
//
//       name:apple,color:red,round:true
//
//       {
//         "name": "apple",
//         "color": "red",
//         "round": "true"
//       }
//
// All values are strings, as Simple KV is untyped.
//
// Escaping is done with the "%" character, which preserves the next
// character. For example, the Simple KV string "tricky%%:yes%,it%:is"
// has a single key-value pair with key "tricky%" and value "yes,it:is".
// You can escape characters other than ":", ",", and "%", but there's
// no particular reason to.
//
// The ":" character does not need to be escaped in a value, and the ","
// character does not need to be escaped in a key except when it is the
// first character of the key. However, for simplicity, it's best to
// always escape these characters.
//
// The "," characters that separate the key-value pairs can freely be
// repeated, and can be placed at the beginning and end of the whole
// Simple KV string. For example, the original example could also be
// written as ",,name:apple,color:red,,,round:true,".
//
// The null character is not allowed. If there's a null character, it
// will be interpreted as the end of the Simple KV string. Keys and
// values can therefore never have null characters in them.
//
// Duplicate keys are not given any special attention. All key-value
// pairs will come out of read() in the order they appear, duplicate
// keys or not.
//

class simple_kv_reader final {

  sst::moved_from moved_from_;

  using string_t = std::string;
  using char_t = typename string_t::value_type;

  string_t src_copy_;
  char_t const * src_ = nullptr;

  bool have_kv_ = false;
  string_t key_;
  string_t value_;

public:

  simple_kv_reader() : moved_from_(true) {
  }

  explicit simple_kv_reader(char_t const * const src)
      : src_((SST_ASSERT(src != nullptr), src)) {
  }

  explicit simple_kv_reader(string_t const * const src)
      : src_((SST_ASSERT(src != nullptr), src->c_str())) {
  }

  explicit simple_kv_reader(string_t src)
      : src_copy_(std::move(src)),
        src_(src_copy_.c_str()) {
  }

  simple_kv_reader(simple_kv_reader const &) = delete;
  simple_kv_reader & operator=(simple_kv_reader const &) = delete;
  simple_kv_reader(simple_kv_reader &&) = default;
  simple_kv_reader & operator=(simple_kv_reader &&) = default;
  ~simple_kv_reader() = default;

  SST_NODISCARD() bool read();

  SST_NODISCARD() string_t & key() noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(have_kv_);
    return key_;
  }

  SST_NODISCARD() string_t const & key() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(have_kv_);
    return key_;
  }

  SST_NODISCARD() string_t & value() noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(have_kv_);
    return value_;
  }

  SST_NODISCARD() string_t const & value() const noexcept {
    SST_ASSERT(!moved_from_);
    SST_ASSERT(have_kv_);
    return value_;
  }

}; //

} // namespace kestrel

#endif // #ifndef KESTREL_SIMPLE_KV_READER_HPP
