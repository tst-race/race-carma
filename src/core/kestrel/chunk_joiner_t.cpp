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
#include <kestrel/chunk_joiner_t.hpp>
// Include twice to test idempotence.
#include <kestrel/chunk_joiner_t.hpp>
//

#include <string>
#include <utility>

#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_BOT.hpp>
#include <sst/catalog/SST_TEV_TOP.hpp>
#include <sst/catalog/optional.hpp>

#include <kestrel/common_sdk_t.hpp>
#include <kestrel/corruption_t.hpp>
#include <kestrel/tracing_event_t.hpp>

namespace kestrel {

chunk_joiner_t::chunk_joiner_t(tracing_event_t tev,
                               std::string dir,
                               common_sdk_t * const sdk)
    : dir_(std::move(dir)),
      sdk_(sdk) {
  SST_TEV_TOP(tev);
  SST_TEV_BOT(tev);
}

sst::optional<bytes_t>
chunk_joiner_t::add(tracing_event_t tev,
                    bytes_t const & id,
                    bytes_t::size_type const index,
                    bytes_t::size_type const count,
                    bytes_t chunk) {
  SST_TEV_TOP(tev);
  if (index >= count) {
    throw corruption_t();
  }
  auto const group_it = groups_.find(id);
  if (group_it == groups_.end()) {
    if (count == 1U) {
      return chunk;
    }
    group_t & group = groups_[id];
    group.count = count;
    group.chunks[index] = std::move(chunk);
    return {};
  }
  group_t & group = group_it->second;
  if (count != group.count) {
    throw corruption_t();
  }
  auto const chunk_it = group.chunks.find(index);
  if (chunk_it == group.chunks.end()) {
    group.chunks.emplace(index, std::move(chunk));
  } else if (chunk != chunk_it->second) {
    throw corruption_t();
  }
  if (group.chunks.size() < group.count) {
    return {};
  }
  bytes_t x;
  for (auto const & y : group.chunks) {
    x.insert(x.end(), y.second.begin(), y.second.end());
  }
  groups_.erase(group_it);
  return x;
  SST_TEV_BOT(tev);
}

void chunk_joiner_t::flush(tracing_event_t tev) {
  SST_TEV_TOP(tev);
  SST_TEV_BOT(tev);
}

} // namespace kestrel
