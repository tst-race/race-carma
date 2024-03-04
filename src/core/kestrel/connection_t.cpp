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
#include <kestrel/connection_t.hpp>
// Include twice to test idempotence.
#include <kestrel/connection_t.hpp>

#include <EncPkg.h>
#include <SdkResponse.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/race_handle_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_exception_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <cassert>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/to_string.hpp>
#include <utility>

namespace kestrel {

connection_t::connection_t(tracing_event_t tev,
                           sdk_wrapper_t & sdk,
                           connection_id_t const & id)
    : sdk_((SST_TEV_ADD(tev), sdk)),
      id_(id) {
}

connection_t::~connection_t() noexcept {
  tracing_event_t SST_TEV_DEF(tev);
  try {
    if (!moved_from_) {
      sdk_.closeConnection(SST_TEV_ARG(tev), id_.string(), 0);
    }
  } catch (...) {
  }
}

connection_t::connection_t(connection_t && other) noexcept
    : sdk_((assert(!other.moved_from_), other.sdk_)),
      id_(std::move(other.id_)) {
  other.moved_from_ = true;
}

connection_id_t const & connection_t::id() const noexcept {
  assert(!moved_from_);
  return id_;
}

race_handle_t connection_t::send(tracing_event_t tev,
                                 encpkg_t const & encpkg) {
  assert(!moved_from_);
  SST_TEV_ADD(tev);
  try {
    SST_TEV_ADD(tev, "outgoing_connection", to_json());

    SST_TEV_ADD(tev,
                  "send_size",
                  sst::to_string(encpkg.blob().size()));
    SdkResponse const r = sdk_.sendEncryptedPackage(SST_TEV_ARG(tev),
                                                    encpkg.to_EncPkg(),
                                                    id_.string(),
                                                    0);

    // Note that we only emit the outgoing_encpkg event after the
    // sendEncryptedPackage call has succeeded, not before. This is
    // because we want the property "if an outgoing_encpkg event was
    // emitted then a successful sendEncryptedPackage call was made" to
    // always be true, as this property is useful for analysis. Ideally
    // we could get if and only if, but that's impossible in practice,
    // so this is the best option.
    SST_TEV_ADD(tev, "outgoing_encpkg", encpkg.to_json());
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev, "event", "outgoing_encpkg"));

    return race_handle_t(r.handle);
  }
  SST_TEV_RETHROW(tev);
}

nlohmann::json connection_t::to_json() const {
  assert(!moved_from_);
  auto j = nlohmann::json::object();
  j["id"] = id_;
  return j;
}

} // namespace kestrel
