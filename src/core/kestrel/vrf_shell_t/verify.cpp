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
#include <kestrel/vrf_shell_t.hpp>
// Include twice to test idempotence.
#include <kestrel/vrf_shell_t.hpp>
//

#include <cstddef>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/checked_cast.hpp>

namespace kestrel {

bool vrf_shell_t::verify(
    std::vector<unsigned char> const & pk,
    std::vector<unsigned char> const & input,
    std::vector<unsigned char> const & proof,
    std::vector<unsigned char> const & output) const {
  SST_ASSERT((!moved_from_));
  SST_ASSERT((core_ != nullptr));
  SST_ASSERT((pk.size() == core_->pk_size()));
  SST_ASSERT((proof.size() == core_->proof_size()));
  SST_ASSERT((output.size() == core_->output_size()));
  unsigned char const empty = 0;
  return core_->verify(pk.data(),
                       input.empty() ? &empty : input.data(),
                       sst::checked_cast<std::size_t>(input.size()),
                       proof.data(),
                       output.data());
}

} // namespace kestrel
