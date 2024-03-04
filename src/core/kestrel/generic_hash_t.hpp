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

#ifndef KESTREL_GENERIC_HASH_T_HPP
#define KESTREL_GENERIC_HASH_T_HPP

#include <array>
#include <kestrel/sodium.hpp>
#include <cstddef>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/enable_if.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/remove_cvref.hpp>
#include <sst/catalog/old/to_bytes.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_unsigned.hpp>
#include <sst/catalog/type_max.hpp>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace kestrel {

class generic_hash_t final {
  crypto_generichash_state state_{};
  std::array<unsigned char, crypto_generichash_BYTES> hash_{};
  bool updating_ = true;

public:
  generic_hash_t();

  template<class Size>
  void update(unsigned char const * x, Size const n) {
    SST_ASSERT((updating_));
    SST_ASSERT((x != nullptr));
    SST_ASSERT((sst::perfect_ge(n, 0)));
    auto r = sst::to_unsigned(n);
    while (r != 0) {
      using T = unsigned long long;
      constexpr auto max = sst::type_max<T>::value / T(4);
      auto const k = T(r < max ? r : max);
      auto const s = sodium::crypto_generichash_update(&state_, x, k);
      if (s != 0) {
        throw std::runtime_error(
            "crypto_generichash_update() failed (return value: "
            + sst::to_string(s) + ")");
      }
      x += k;
      r -= k;
    }
  }

  template<
      class ContiguousByteSequence,
      typename sst::enable_if<
          std::is_pointer<decltype(
              std::declval<ContiguousByteSequence>().data())>::value
          && (std::is_same<typename sst::remove_cvref<decltype(
                               *std::declval<ContiguousByteSequence>()
                                    .data())>::type,
                           unsigned char>::value
              || std::is_same<
                  typename sst::remove_cvref<
                      decltype(*std::declval<ContiguousByteSequence>()
                                    .data())>::type,
                  char>::value
              || std::is_same<
                  typename sst::remove_cvref<
                      decltype(*std::declval<ContiguousByteSequence>()
                                    .data())>::type,
                  signed char>::value)>::type = 0>
  void update(ContiguousByteSequence const & x) {
    auto const data = x.data();
    if (data != nullptr) {
      update(reinterpret_cast<unsigned char const *>(data), x.size());
    }
  }

  template<
      class T,
      typename sst::enable_if<std::is_integral<T>::value>::type = 0>
  void update(T const x) {
    std::array<unsigned char, sizeof(T)> y;
    sst::old::to_bytes(x,
                  y.data(),
                  y.size(),
                  std::is_signed<T>::value ?
                      sst::integer_rep::twos_complement() :
                      sst::integer_rep::pure_unsigned());
    update(y);
  }

  std::array<unsigned char, crypto_generichash_BYTES> const & finish();
};

extern template void generic_hash_t::update(unsigned char const * x,
                                            std::size_t n);

} // namespace kestrel

#endif // #ifndef KESTREL_GENERIC_HASH_T_HPP
