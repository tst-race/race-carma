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
#include <kestrel/clrmsg_t.hpp>
// Include twice to test idempotence.
#include <kestrel/clrmsg_t.hpp>
//

#include <utility>

#include <sst/catalog/remove_cvref_t.hpp>

#include <kestrel/generic_hash_t.hpp>

namespace kestrel {

sst::remove_cvref_t<decltype(std::declval<generic_hash_t>().finish())>
clrmsg_t::content_hash() const {
  generic_hash_t hash;
  hash.update(msg_);
  hash.update(from_);
  hash.update(to_);
  hash.update(time_);
  hash.update(nonce_);
  hash.update(trace_id_);
  hash.update(span_id_);
  return hash.finish();
}

} // namespace kestrel
