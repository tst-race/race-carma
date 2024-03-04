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
#include <kestrel/packet_packet_t.hpp>
// Include twice to test idempotence.
#include <kestrel/packet_packet_t.hpp>

// Standard C++ headers
#include <cstddef>

namespace kestrel {

constexpr int packet_packet_t::receiver_size_size_;
constexpr int packet_packet_t::payload_size_size_;

template std::size_t
packet_packet_t::to_bytes_size<std::size_t>() const;

} // namespace kestrel
