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

// Include first to test independence.
#include <kestrel/carma/phonebook_set_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_set_t.hpp>
//

#include <exception>
#include <string>
#include <unordered_set>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/atomic.hpp>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/expect_array.hpp>
#include <sst/catalog/json/expect_string.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/phonebook_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------

void parse_phonebook_set(phonebook_t const & phonebook,
                         json_t & src,
                         char const * const key,
                         sst::unique_ptr<phonebook_set_t> & dst,
                         bool const required) {
  SST_ASSERT((key));
  if (dst) {
    return;
  }
  try {
    auto const it = src.find(key);
    if (it != src.end()) {
      phonebook_set_t xs;
      json_t & ys = sst::json::expect_array(*it);
      decltype(ys.size()) i = 0U;
      for (json_t & y : ys) {
        try {
          xs.emplace(&phonebook.expect<sst::json::exception>(
              psn_t(std::move(sst::json::expect_string(y)
                                  .get_ref<json_t::string_t &>()))));
          ++i;
        } catch (sst::json::exception const & e) {
          std::throw_with_nested(e.add_index(i));
        }
      }
      dst.emplace(std::move(xs));
      src.erase(it);
    } else if (required) {
      throw sst::json::exception::missing_value();
    }
  } catch (sst::json::exception const & e) {
    // TODO: e.add_key(key) was causing a compilation error at the
    //       time of writing this code. SST will eventually fix it.
    std::throw_with_nested(e.add_key(std::string(key)));
  }
}

void parse_phonebook_set(
    phonebook_t const & phonebook,
    json_t & src,
    char const * const key,
    sst::atomic<sst::unique_ptr<phonebook_set_t>> & dst,
    bool const required) {
  SST_ASSERT((key));
  sst::unique_ptr<phonebook_set_t> p;
  parse_phonebook_set(phonebook, src, key, p, required);
  dst.store(std::move(p));
}

//----------------------------------------------------------------------

void unparse_phonebook_set(json_t & dst,
                           char const * const key,
                           phonebook_set_t const & src) {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((key));
  json_t & a = dst[key] = json_t::array();
  for (phonebook_pair_t const * const & b : src) {
    a.emplace_back(b->first);
  }
}

void unparse_phonebook_set(json_t & dst,
                           char const * const key,
                           phonebook_set_t const * const src) {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((key));
  if (src) {
    unparse_phonebook_set(dst, key, *src);
  }
}

void unparse_phonebook_set(
    json_t & dst,
    char const * const key,
    sst::unique_ptr<phonebook_set_t> const & src) {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((key));
  unparse_phonebook_set(dst, key, src.get());
}

void unparse_phonebook_set(
    json_t & dst,
    char const * const key,
    sst::atomic<sst::unique_ptr<phonebook_set_t>> const & src) {
  SST_ASSERT((dst.is_object()));
  SST_ASSERT((key));
  unparse_phonebook_set(dst, key, src.load());
}

} // namespace carma
} // namespace kestrel
