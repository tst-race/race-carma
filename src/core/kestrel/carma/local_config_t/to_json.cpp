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

// Include first to test independence.
#include <kestrel/carma/local_config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/local_config_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/to_hex.hpp>

#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>

namespace kestrel {
namespace carma {

void to_json(json_t & dst, local_config_t const & src) {

  SST_ASSERT((!src.moved_from_));

  dst = json_t::object();

  src.phonebook_entry_t::to_json_core(dst);

  src.unparse_mb_servers(dst);
  src.unparse_mc_groups(dst);
  src.unparse_mc_leader(dst);
  src.unparse_next_layer(dst);
  src.unparse_other_servers(dst);
  src.unparse_prev_layer(dst);
  src.unparse_rx_nodes(dst);

  dst["sk"] = sst::to_hex(src.sk());

  unparse_phonebook_set(dst, "tx_nodes", src.tx_nodes_);

  src.unparse_vrf_sk(dst);

} //

} // namespace carma
} // namespace kestrel
