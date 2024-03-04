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
#include <kestrel/normalize_path.hpp>
// Include twice to test idempotence.
#include <kestrel/normalize_path.hpp>
//

#include <string>

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/SST_TEST_SHOW.hpp>
#include <sst/catalog/test_main.hpp>

using namespace kestrel;

int main() {
  return sst::test_main([] {
    ;

    //------------------------------------------------------------------

#define F(x, y)                                                        \
  ([] {                                                                \
    std::string r;                                                     \
    SST_TEST_BOOL((r = normalize_path(x), r == y),                     \
                  SST_TEST_SHOW((r)));                                 \
  }())

    F(".", ".");
    F("..", ".");
    F("../", ".");
    F("../.", ".");
    F("../..", ".");
    F("../../", ".");
    F(".././", ".");
    F("..//..", ".");
    F("../x", "x");
    F("./", ".");
    F("./.", ".");
    F("./..", ".");
    F("./../", ".");
    F("././", ".");
    F("./x/./y/./.././z/./../.", "x");
    F("/.", ".");
    F("/..", ".");
    F("/../", ".");
    F("/../.", ".");
    F("/../..", ".");
    F("/../../", ".");
    F("/.././", ".");
    F("/./", ".");
    F("/./.", ".");
    F("/./..", ".");
    F("/./../", ".");
    F("/././", ".");
    F("/x", "x");
    F("x/..", ".");
    F("x/../y", "y");
    F("x/y/../../../z", "z");
    F("x/y/../../z", "z");
    F("x/y/../z", "x/z");
    F("x/y/../z/..", "x");

#undef F

    //------------------------------------------------------------------
  });
}
