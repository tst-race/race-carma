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

#include <ClrMsg.h>
#include <kestrel/clrmsg_t.hpp>
#include <iostream>
#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/test_main.hpp>
#include <sst/catalog/to_string.hpp>
#include <vector>

using namespace kestrel;

int main() {
  return sst::test_main([]() {

    clrmsg_t a;
    a.to_bytes_init(ClrMsg("hello", "alice", "bob", 1, 2, 3, 4));

    std::vector<unsigned char> a_bytes;
    a_bytes.resize(a.to_bytes_size<decltype(a_bytes.size())>());
    std::cout << "a_bytes.size() == " << sst::to_string(a_bytes.size())
              << "\n";
    SST_TEST_BOOL(a.to_bytes(a_bytes.begin()) == a_bytes.end());

    clrmsg_t b;
    b.from_bytes_init();
    auto avail = a_bytes.size();
    SST_TEST_BOOL(b.from_bytes(a_bytes.begin(), avail)
                  == a_bytes.end());
    SST_TEST_BOOL(avail == 0);
    SST_TEST_BOOL(a == b);

    std::vector<unsigned char> b_bytes;
    b_bytes.resize(b.to_bytes_size<decltype(b_bytes.size())>());
    b.to_bytes(b_bytes.begin());
    SST_TEST_BOOL(a_bytes == b_bytes);
  });
}
