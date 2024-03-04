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

#include <exception>
#include <utility>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/json/exception.hpp>
#include <sst/catalog/json/remove_to.hpp>
#include <sst/catalog/unique_ptr.hpp>

#include <kestrel/carma/global_config_t.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/vrf_eval_result_t.hpp>
#include <kestrel/vrf_shell_t.hpp>

namespace kestrel {
namespace carma {

void phonebook_entry_t::parse_ticket(json_t & src) {
  SST_ASSERT((!moved_from_));
  if (ticket_) {
    return;
  }
  parse_vrf_pk(src);
  if (!vrf_pk_) {
    return;
  }
  sst::unique_ptr<vrf_eval_result_t> x;
  if (dynamic_cast<local_config_t *>(this)) {
    sst::json::remove_to(src, x, "ticket");
  } else {
    sst::json::remove_to(src, x.ensure(), "ticket");
  }
  if (x && !x->verified()) {
    if (!global().vrf().verify(
            vrf_pk(),
            global().vrf().ticket_input(global().epoch_nonce()),
            x->proof(),
            x->output())) {
      try {
        throw sst::json::exception("Invalid VRF ticket proof/output");
      } catch (sst::json::exception const & e) {
        std::throw_with_nested(e.add_key("ticket"));
      }
    }
    x->set_verified(true);
  }
  ticket_ = std::move(x);
}

} // namespace carma
} // namespace kestrel
