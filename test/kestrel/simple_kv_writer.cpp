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
#include <kestrel/simple_kv_writer.hpp>
// Include twice to test idempotence.
#include <kestrel/simple_kv_writer.hpp>
//

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/test_main.hpp>

using namespace kestrel;

int main() {
  return sst::test_main([] {
    ;

    simple_kv_writer w;

    SST_TEST_BOOL((w = simple_kv_writer(), w.get() == ""));

    SST_TEST_BOOL(
        (w = simple_kv_writer(), w.write("a", "b"), w.get() == "a:b"));

    SST_TEST_BOOL((w = simple_kv_writer(),
                   w.write("a", "b"),
                   w.write("c", "d"),
                   w.get() == "a:b,c:d"));

    SST_TEST_BOOL((w = simple_kv_writer(),
                   w.write(",", ","),
                   w.get() == "%,:%,"));

    SST_TEST_BOOL((w = simple_kv_writer(),
                   w.write("", ""),
                   w.write("%", ":"),
                   w.get() == ":,%%:%:"));

    ;
  });
}
