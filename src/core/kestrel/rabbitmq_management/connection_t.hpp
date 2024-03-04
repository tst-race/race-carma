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

#ifndef KESTREL_RABBITMQ_MANAGEMENT_CONNECTION_T_HPP
#define KESTREL_RABBITMQ_MANAGEMENT_CONNECTION_T_HPP

#include <sst/catalog/SST_NOEXCEPT.hpp>

#include <kestrel/easier_ta2_connection_t.hpp>
#include <kestrel/rabbitmq_management/package_t.hpp>

namespace kestrel {
namespace rabbitmq_management {

class plugin_t;
class link_t;
class package_t;

class connection_t final : public easier_ta2_connection_t<plugin_t,
                                                          link_t,
                                                          connection_t,
                                                          package_t> {

  friend class package_t;

  using base = easier_ta2_connection_t<plugin_t,
                                       link_t,
                                       connection_t,
                                       package_t>;

  //--------------------------------------------------------------------

private:

  bool declared_queue_ = false;

public:

  void recv() override;

  void recv_proc(void * p_context, void * p_fetch, bool const success);

  //--------------------------------------------------------------------
  // Constructor
  //--------------------------------------------------------------------

public:

  using base::base;

  //--------------------------------------------------------------------
  // Default operations
  //--------------------------------------------------------------------

public:

  connection_t() = delete;

  ~connection_t() SST_NOEXCEPT(true) override = default;

  connection_t(connection_t const &) = delete;

  connection_t & operator=(connection_t const &) = delete;

  connection_t(connection_t &&) = delete;

  connection_t & operator=(connection_t &&) = delete;

  //--------------------------------------------------------------------
};

} // namespace rabbitmq_management
} // namespace kestrel

#endif // #ifndef KESTREL_RABBITMQ_MANAGEMENT_CONNECTION_T_HPP
