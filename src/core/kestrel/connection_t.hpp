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

#ifndef KESTREL_CONNECTION_T_HPP
#define KESTREL_CONNECTION_T_HPP

#include <kestrel/connection_id_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {

class connection_t final {
  bool moved_from_ = false;
  sdk_wrapper_t & sdk_;
  connection_id_t id_;

public:
  explicit connection_t(tracing_event_t tev,
                        sdk_wrapper_t & sdk,
                        connection_id_t const & id);

  ~connection_t() noexcept;

  connection_t(connection_t const &) = delete;
  connection_t & operator=(connection_t const &) = delete;

  connection_t(connection_t && other) noexcept;
  connection_t & operator=(connection_t && other) = delete;

  connection_id_t const & id() const noexcept;

  race_handle_t send(tracing_event_t tev, encpkg_t const & encpkg);

  nlohmann::json to_json() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_CONNECTION_T_HPP
