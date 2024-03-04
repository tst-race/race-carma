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

#ifndef KESTREL_GUID_T_HPP
#define KESTREL_GUID_T_HPP

#include <array>
#include <cstddef>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/boxed.hpp>
#include <sst/catalog/buffer_underrun.hpp>
#include <sst/catalog/char_bit_v.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/perfect_ge.hpp>
#include <sst/catalog/uchar_max_v.hpp>
#include <sst/catalog/unsigned_gt.hpp>
#include <sst/catalog/to_hex.hpp>

namespace kestrel {

namespace guts {
namespace guid_t {

static constexpr std::size_t size = 16;
using type = std::array<unsigned char, size>;

} // namespace guid_t
} // namespace guts

class guid_t final : sst::boxed<guts::guid_t::type, guid_t> {
  using boxed = sst::boxed<guts::guid_t::type, guid_t>;
  friend class sst::boxed<guts::guid_t::type, guid_t>;

public:
  static guid_t generate();

  //--------------------------------------------------------------------

  template<class T>
  guid_t operator+(T y) {
    SST_ASSERT(sst::perfect_ge(y, 0));
    guid_t r = *this;
    auto & x = r.value();
    bool carry = 0;
    for (auto i = x.size(); i-- > 0;) {
      unsigned int const a = value()[i];
      unsigned int const b =
          static_cast<unsigned int>(y) & sst::uchar_max_v;
      x[i] = static_cast<unsigned char>(a + b + carry);
      carry = x[i] <= a;
      if (!carry) {
        break;
      }
      y >>= sst::char_bit_v - 2;
      y >>= 2;
    }
    return r;
  }

  //--------------------------------------------------------------------
  // Comparison operators
  //--------------------------------------------------------------------

  using boxed::operator<;
  using boxed::operator>;
  using boxed::operator<=;
  using boxed::operator>=;
  using boxed::operator==;
  using boxed::operator!=;

  //--------------------------------------------------------------------
  // Serialization
  //--------------------------------------------------------------------

  template<class Size>
  Size to_bytes_size() const {
    sst::checked_t<Size> n = 0;
    n += value().size();
    return n.value();
  }

  template<class ByteIt>
  ByteIt to_bytes(ByteIt dst) const {
    dst = sst::copy_bytes(value().begin(), value().size(), dst);
    return dst;
  }

  template<class ByteIt, class Avail>
  ByteIt from_bytes(ByteIt src, Avail & avail) {
    SST_ASSERT(sst::perfect_ge(avail, 0));
    sst::copy_bytes(&src, value().size(), avail, value().begin());
    return src;
  }

  //--------------------------------------------------------------------
  // JSON
  //--------------------------------------------------------------------

  nlohmann::json to_json() const {
    return sst::to_hex(value());
  }

  //--------------------------------------------------------------------
};

extern template std::size_t guid_t::to_bytes_size<std::size_t>() const;

} // namespace kestrel

#endif // #ifndef KESTREL_GUID_T_HPP
