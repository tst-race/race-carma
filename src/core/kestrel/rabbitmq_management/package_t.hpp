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

#ifndef KESTREL_RABBITMQ_MANAGEMENT_PACKAGE_T_HPP
#define KESTREL_RABBITMQ_MANAGEMENT_PACKAGE_T_HPP

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <kestrel/easier_ta2_package_t.hpp>

namespace kestrel {
namespace rabbitmq_management {

class plugin_t;
class link_t;
class connection_t;

class package_t final : public easier_ta2_package_t<plugin_t,
                                                    link_t,
                                                    connection_t,
                                                    package_t> {

  using base =
      easier_ta2_package_t<plugin_t, link_t, connection_t, package_t>;

  //--------------------------------------------------------------------
  // send
  //--------------------------------------------------------------------

public:

  void send() final;

  void send_proc(void *, bool success);

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  using base::base;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  package_t() = delete;

  ~package_t() SST_NOEXCEPT(true) override = default;

  package_t(package_t const &) = delete;

  package_t & operator=(package_t const &) = delete;

  package_t(package_t &&) = delete;

  package_t & operator=(package_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq_management
} // namespace kestrel

#endif // #ifndef KESTREL_RABBITMQ_MANAGEMENT_PACKAGE_T_HPP
