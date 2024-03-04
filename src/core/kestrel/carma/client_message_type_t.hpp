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

#ifndef KESTREL_CARMA_CLIENT_MESSAGE_TYPE_T_HPP
#define KESTREL_CARMA_CLIENT_MESSAGE_TYPE_T_HPP

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_STRONG_ENUM_CLASS.hpp>

#include <kestrel/bytes_t.hpp>
#include <kestrel/serialization.hpp>

namespace kestrel {

namespace carma {

namespace guts {

// TODO: These will eventually need to use hardcoded values for cross
//       version compatibility.

SST_STRONG_ENUM_CLASS(client_message_type_t,
                      member(add_contact_response),
                      member(message))

inline serialize_size_t
serialize_size(client_message_type_t const & src) {
  return kestrel::serialize_size(src.value());
}

inline bytes_t & serialize(bytes_t & dst,
                           client_message_type_t const & src) {
  bytes_t::size_type const old_dst_size = dst.size();
  kestrel::serialize(dst, src.value());
  SST_ASSERT((serialize_size(src).value == dst.size() - old_dst_size));
  return dst;
}

inline void deserialize(bytes_t const & src,
                        bytes_t::size_type & idx,
                        client_message_type_t & dst) {
  SST_ASSERT((idx < src.size()));
  bytes_t::size_type old_idx = idx;
  client_message_type_t::value_type x;
  kestrel::deserialize(src, idx, x);
  dst = client_message_type_t(x);
  SST_ASSERT((serialize_size(dst).value == idx - old_idx));
}

} // namespace guts

using guts::client_message_type_t;

} // namespace carma

} // namespace kestrel

#endif // #ifndef KESTREL_CARMA_CLIENT_MESSAGE_TYPE_T_HPP
