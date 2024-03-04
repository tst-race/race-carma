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

#ifndef KESTREL_PSN_DEFAULT_HASH_T_HPP
#define KESTREL_PSN_DEFAULT_HASH_T_HPP

#include <kestrel/psn_hash_1_t.hpp>

namespace kestrel {

//
// psn_default_hash_t is an alias for the PSN hash class that CARMA will use for
// all PSN hashes that it sends to other nodes. All psn_hash_*_t classes
// are still supported for PSN hashes received from other nodes.
//
// To migrate to a new hash, the following approach should be used:
//
//    1. Implement the new hash as a new psn_hash_*_t class, but
//       don't update the psn_default_hash_t alias yet.
//
//    2. Wait some reasonable amount of time to give all nodes a
//       chance to update CARMA to a newer version that supports
//       (but does not yet utilize) the new psn_hash_*_t class.
//
//    3. Update the psn_default_hash_t alias to point to the new
//       psn_hash_*_t class.
//

//
// For serializing PSN hashes, the following scheme is used.
//
// The first byte of a serialized PSN hash is always part of the header.
//
// If the MSB of the first byte is 0, then there are no further bytes in
// the header, the hash is of type psn_hash_1_t, and the low 7 bits of
// the byte must be 15 (so the byte is 0x0F).
//
// The actual hash then follows the header.
//
// For psn_hash_1_t, the hash is sha256(psn || "kestrel") truncated to
// 15 bytes. The total size including the header is 16 bytes.
//

using psn_default_hash_t = psn_hash_1_t;

} // namespace kestrel

#endif // #ifndef KESTREL_PSN_DEFAULT_HASH_T_HPP
