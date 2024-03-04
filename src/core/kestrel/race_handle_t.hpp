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

#ifndef KESTREL_RACE_HANDLE_T_HPP
#define KESTREL_RACE_HANDLE_T_HPP

// Standard C++ headers
#include <cstddef>
#include <functional>

#include <sst/catalog/boxed.hpp>

#include <SdkResponse.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

class race_handle_t final : sst::boxed<RaceHandle, race_handle_t> {
  using boxed = sst::boxed<RaceHandle, race_handle_t>;
  friend class sst::boxed<RaceHandle, race_handle_t>;

  enum class enum_t : RaceHandle {
    null = NULL_RACE_HANDLE,
  };

public:
  using boxed::operator!=;
  using boxed::operator<;
  using boxed::operator<=;
  using boxed::operator==;
  using boxed::operator>;
  using boxed::operator>=;
  using boxed::value;

  explicit constexpr race_handle_t(RaceHandle handle) : boxed(handle) {
  }

  explicit constexpr race_handle_t() : boxed(NULL_RACE_HANDLE) {
  }

  explicit race_handle_t(SdkResponse const & response);

  constexpr operator enum_t() const noexcept {
    return static_cast<enum_t>(value());
  }

  static constexpr race_handle_t null() noexcept {
    return race_handle_t(NULL_RACE_HANDLE);
  }

  friend void to_json(nlohmann::json & dst, race_handle_t const & src);
};

} // namespace kestrel

namespace std {

template<>
struct hash<kestrel::race_handle_t> final {
  using result_type = std::size_t;
  using argument_type = kestrel::race_handle_t;
  std::size_t operator()(kestrel::race_handle_t const & src) const;
};

} // namespace std

#endif // #ifndef KESTREL_RACE_HANDLE_T_HPP
