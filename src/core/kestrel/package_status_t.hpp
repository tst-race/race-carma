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

#ifndef KESTREL_PACKAGE_STATUS_T_HPP
#define KESTREL_PACKAGE_STATUS_T_HPP

#include <sst/catalog/boxed.hpp>

#include <PackageStatus.h>

#include <kestrel/json_t.hpp>

namespace kestrel {

class package_status_t final
    : sst::boxed<PackageStatus, package_status_t> {
  using boxed = sst::boxed<PackageStatus, package_status_t>;
  friend class sst::boxed<PackageStatus, package_status_t>;

public:
  using boxed::boxed;
  using boxed::operator==;
  using boxed::operator!=;
  using boxed::value;

  static constexpr package_status_t invalid() noexcept {
    return package_status_t(PACKAGE_INVALID);
  }
  static constexpr package_status_t sent() noexcept {
    return package_status_t(PACKAGE_SENT);
  }
  static constexpr package_status_t received() noexcept {
    return package_status_t(PACKAGE_RECEIVED);
  }
  static constexpr package_status_t failed_generic() noexcept {
    return package_status_t(PACKAGE_FAILED_GENERIC);
  }
  static constexpr package_status_t failed_network_error() noexcept {
    return package_status_t(PACKAGE_FAILED_NETWORK_ERROR);
  }

  //--------------------------------------------------------------------
  // to_json (ADL)
  //--------------------------------------------------------------------

  friend void to_json(json_t & dst, package_status_t const & src);

  //--------------------------------------------------------------------
  // to_json (member)
  //--------------------------------------------------------------------

public:

  json_t to_json() const;

  //--------------------------------------------------------------------
};

//----------------------------------------------------------------------
// to_json (ADL)
//----------------------------------------------------------------------

void to_json(json_t & dst, package_status_t const & src);

//----------------------------------------------------------------------

} // namespace kestrel

#endif // #ifndef KESTREL_PACKAGE_STATUS_T_HPP
