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

#ifndef KESTREL_OUTBOX_ENTRY_T_HPP
#define KESTREL_OUTBOX_ENTRY_T_HPP

#include <EncPkg.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/link_id_t.hpp>
#include <kestrel/origin_handle_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/pooled.hpp>
#include <kestrel/race_handle_t.hpp>
#include <map>
#include <set>
#include <sst/time.h>
#include <utility>
#include <vector>

namespace kestrel {

struct outbox_entry_t final : origin_handle_t {
  EncPkg message;
  psn_t psn;

  struct attempt_t final {
    decltype(sst::mono_time_ns()) time;
    link_id_t link_id;
    connection_id_t connection_id;
    race_handle_t handle;
  };
  std::vector<pooled<attempt_t>> attempts;

  std::set<link_id_t> link_ids;
  std::set<race_handle_t> handles;

  outbox_entry_t()
      : origin_handle_t(race_handle_t::null()),
        message({}) {
  }

  void init(race_handle_t const & origin_handle,
            psn_t const & psn,
            EncPkg && message) {
    set_origin_handle(origin_handle);
    this->message = std::move(message);
    this->psn = psn;
    attempts.clear();
    link_ids.clear();
    handles.clear();
  }
};

} // namespace kestrel

#endif // #ifndef KESTREL_OUTBOX_ENTRY_T_HPP
