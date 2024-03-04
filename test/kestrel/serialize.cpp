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
#include <kestrel/serialization.hpp>
// Include twice to test idempotence.
#include <kestrel/serialization.hpp>
//

#include <string>
#include <vector>

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/test_main.hpp>

#include <kestrel/serialization.hpp>

using namespace kestrel;

int main() {
  return sst::test_main([] {
    ;

    std::vector<unsigned char> x;

    SST_TEST_BOOL(
        (x = serialize({}), x == std::vector<unsigned char>{}));

    SST_TEST_BOOL(
        (x = serialize({}, 1), x == std::vector<unsigned char>{1}));

    SST_TEST_BOOL((x = serialize({}, 1, 2),
                   x == std::vector<unsigned char>{1, 2}));

    SST_TEST_BOOL((x = serialize({}, std::string("a")),
                   x == std::vector<unsigned char>{1, 'a'}));

    SST_TEST_BOOL(
        (x = serialize({}, std::string("a"), std::string("b")),
         x == std::vector<unsigned char>{1, 'a', 1, 'b'}));

    SST_TEST_BOOL(
        (x = serialize({},
                       std::string("ab"),
                       std::vector<unsigned char>{10, 11, 12}),
         x == std::vector<unsigned char>{2, 'a', 'b', 3, 10, 11, 12}));

    ;
  });
}
