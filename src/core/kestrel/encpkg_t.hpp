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

#ifndef KESTREL_ENCPKG_T_HPP
#define KESTREL_ENCPKG_T_HPP

#include <EncPkg.h>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <type_traits>
#include <utility>
#include <vector>

namespace kestrel {

//
// This class is designed to be pooled. After default constructing or
// acquiring an instance, TODO.
//
// An instance of this class should never be accessed by multiple
// threads at the same time, even if the accesses are conceptually
// const.
//

class encpkg_t final {
  using trace_id_type = decltype(std::declval<EncPkg>().getTraceId());
  using span_id_type = decltype(std::declval<EncPkg>().getSpanId());

  SST_STATIC_ASSERT((std::is_integral<trace_id_type>::value));
  SST_STATIC_ASSERT((std::is_integral<span_id_type>::value));

  std::vector<unsigned char> blob_;
  trace_id_type trace_id_;
  span_id_type span_id_;

public:
  void init(EncPkg const & src);

  bool operator==(encpkg_t const & other) const;
  bool operator!=(encpkg_t const & other) const;

  EncPkg to_EncPkg() const;

  std::vector<unsigned char> & blob() noexcept {
    return blob_;
  }

  std::vector<unsigned char> const & blob() const noexcept {
    return blob_;
  }

  //--------------------------------------------------------------------
  // JSON
  //--------------------------------------------------------------------
  //
  // This is only for logging purposes, so convenience is more important
  // than performance.
  //

  nlohmann::json to_json() const;

  //--------------------------------------------------------------------
};

} // namespace kestrel

#endif // #ifndef KESTREL_ENCPKG_T_HPP
