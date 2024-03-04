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
#include <kestrel/encpkg_t.hpp>
// Include twice to test idempotence.
#include <kestrel/encpkg_t.hpp>

#include <EncPkg.h>
#include <algorithm>
#include <kestrel/generic_hash_t.hpp>
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>
#include <sst/catalog/checked_resize.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/to_hex.hpp>

namespace kestrel {

void encpkg_t::init(EncPkg const & other) {
  auto const & other_blob = other.getCipherText();
  sst::checked_resize(blob_, other_blob.size());
  std::copy(other_blob.begin(), other_blob.end(), blob_.begin());
  trace_id_ = other.getTraceId();
  span_id_ = other.getSpanId();
}

bool encpkg_t::operator==(encpkg_t const & other) const {
  bool x = true;
  x = x && blob_ == other.blob_;
  x = x && trace_id_ == other.trace_id_;
  x = x && span_id_ == other.span_id_;
  return x;
}

bool encpkg_t::operator!=(encpkg_t const & other) const {
  return !operator==(other);
}

EncPkg encpkg_t::to_EncPkg() const {
  return EncPkg(trace_id_, span_id_, blob_);
}

//----------------------------------------------------------------------
// JSON
//----------------------------------------------------------------------
//
// This is only for logging purposes, so convenience is more important
// than performance.
//

nlohmann::json encpkg_t::to_json() const {
  // Do not include trace_id_ or span_id_ in the hash, as these fields
  // may be different for the sender and receiver.
  generic_hash_t hash;
  hash.update(blob_);
  return {{"blob", {{"size", sst::to_string(blob_.size())}}},
          {"trace_id", sst::to_string(trace_id_)},
          {"span_id", sst::to_string(span_id_)},
          {"hash", sst::to_hex(hash.finish())}};
};

//----------------------------------------------------------------------

} // namespace kestrel
