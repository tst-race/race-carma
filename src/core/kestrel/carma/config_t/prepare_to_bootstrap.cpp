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
#include <kestrel/carma/config_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/config_t.hpp>
//

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>

#include <kestrel/channel_id_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {
namespace carma {

void config_t::prepare_to_bootstrap(tracing_event_t tev,
                                    std::string dir,
                                    channel_id_t channel_id,
                                    link_address_t link_address) {
  SST_TEV_TOP(tev);

  config_t dst(SST_TEV_ARG(tev), std::move(dir), sdk_);

  dst.bootstrap() = bootstrap();
  dst.bootstrap().set_bootstrapper(local().psn());
  dst.bootstrap().set_channel_id(std::move(channel_id));
  dst.bootstrap().set_link_address(std::move(link_address));
  dst.bootstrap().exists(true);

  dst.global() = global();

  dst.local() = local();
  dst.local().set_phonebook(dst.phonebook());
  dst.local().set_psn(psn_t("None"));

  auto & pk =
      const_cast<std::vector<unsigned char> &>(dst.local().pk());
  std::fill(pk.begin(), pk.end(), static_cast<unsigned char>(0));

  auto & sk =
      const_cast<std::vector<unsigned char> &>(dst.local().sk());
  std::fill(sk.begin(), sk.end(), static_cast<unsigned char>(0));

  dst.phonebook().copy_assign(SST_TEV_ARG(tev), phonebook());

  dst.clear_deducible(SST_TEV_ARG(tev));

  dst.flush(SST_TEV_ARG(tev));

  SST_TEV_BOT(tev);
}

} // namespace carma
} // namespace kestrel
