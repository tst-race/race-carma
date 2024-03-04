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

#ifndef KESTREL_LINK_T_HPP
#define KESTREL_LINK_T_HPP

#include <LinkProperties.h>
#include <kestrel/old_config_t.hpp>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <map>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <set>
#include <string>
#include <vector>

namespace kestrel {

class link_t final {
  bool moved_from_ = false;
  sdk_wrapper_t & sdk_;
  old_config_t const & old_config_;
  link_id_t id_;
  std::set<psn_t> personas_;
  LinkProperties properties_;
  std::map<connection_id_t, connection_t> connections_;

public:
  explicit link_t(tracing_event_t tev,
                  old_config_t const & config,
                  sdk_wrapper_t & sdk,
                  link_id_t const & id,
                  std::vector<std::string> const & personas);

  ~link_t() noexcept {
  }

  link_t(link_t const &) = delete;
  link_t & operator=(link_t const &) = delete;

  link_t(link_t && other) noexcept;
  link_t & operator=(link_t && other) = delete;

  link_id_t const & id() const noexcept;

  void add_personas(std::vector<std::string> const & personas);
  std::set<psn_t> const & personas() const noexcept;

  LinkProperties const & properties() const noexcept;
  LinkProperties const & properties(LinkProperties const & properties);

  std::map<connection_id_t, connection_t> & connections() noexcept;
  std::map<connection_id_t, connection_t> const &
  connections() const noexcept;

  nlohmann::json to_json() const;
};

} // namespace kestrel

#endif // #ifndef KESTREL_LINK_T_HPP
