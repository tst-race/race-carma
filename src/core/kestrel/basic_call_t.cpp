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
#include <kestrel/basic_call_t.hpp>
// Include twice to test idempotence.
#include <kestrel/basic_call_t.hpp>
//

#include <sst/catalog/mono_time_ns.hpp>

#include <kestrel/race_handle_t.hpp>

namespace kestrel {

//----------------------------------------------------------------------
// handle
//----------------------------------------------------------------------

race_handle_t const & basic_call_t::handle() const {
  return handle_;
}

//----------------------------------------------------------------------
// time
//----------------------------------------------------------------------

decltype(sst::mono_time_ns()) basic_call_t::time() const {
  return time_;
}

//----------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------

basic_call_t::basic_call_t(race_handle_t const & handle)
    : handle_(handle),
      time_(sst::mono_time_ns()) {
}

//----------------------------------------------------------------------

} // namespace kestrel
