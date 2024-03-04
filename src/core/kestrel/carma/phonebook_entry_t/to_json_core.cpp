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
#include <kestrel/carma/phonebook_entry_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/phonebook_entry_t.hpp>
//

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/to_hex.hpp>
#include <sst/catalog/to_string.hpp>

#include <kestrel/carma/node_count_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/carma/phonebook_set_t.hpp>
#include <kestrel/carma/role_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/vrf_eval_result_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_entry_t::to_json_core(json_t & dst) const {

  SST_ASSERT((!moved_from_));

  unparse_bucket(dst);
  unparse_bucket_clients(dst);
  unparse_bucket_mb_servers(dst);
  unparse_group(dst);
  unparse_mc_leaders(dst);
  unparse_order(dst);

  dst["pk"] = sst::to_hex(pk());

  unparse_psn(dst);
  unparse_role(dst);
  unparse_ticket(dst);
  unparse_vrf_pk(dst);

} //

} // namespace carma
} // namespace kestrel
