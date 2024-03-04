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
#include <kestrel/simple_kv_reader.hpp>
// Include twice to test idempotence.
#include <kestrel/simple_kv_reader.hpp>
//

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/SST_TEST_SHOW.hpp>
#include <sst/catalog/SST_TEST_THROW.hpp>
#include <sst/catalog/test_main.hpp>

#include <kestrel/simple_kv_syntax_error.hpp>

using namespace kestrel;

int main() {
  return sst::test_main([] {
    ;

    simple_kv_reader r;

    SST_TEST_BOOL((r = simple_kv_reader(""), !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader("a:b"),
         r.read() && r.key() == "a" && r.value() == "b" && !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader("a:b,"),
         r.read() && r.key() == "a" && r.value() == "b" && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader("a:b,c:d"),
                   r.read() && r.key() == "a" && r.value() == "b"
                       && r.read() && r.key() == "c" && r.value() == "d"
                       && !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader(":"),
         r.read() && r.key() == "" && r.value() == "" && !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader(":,"),
         r.read() && r.key() == "" && r.value() == "" && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader(":,:"),
                   r.read() && r.key() == "" && r.value() == ""
                       && r.read() && r.key() == "" && r.value() == ""
                       && !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader(",::"),
         r.read() && r.key() == "" && r.value() == ":" && !r.read()));

    SST_TEST_BOOL(
        (r = simple_kv_reader(",::,"),
         r.read() && r.key() == "" && r.value() == ":" && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader("tricky%%:yes%,it%:is"),
                   r.read() && r.key() == "tricky%"
                       && r.value() == "yes,it:is" && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader("%f%o%o:%,"),
                   r.read() && r.key() == "foo" && r.value() == ","
                       && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader("%f%o%o:%%,"),
                   r.read() && r.key() == "foo" && r.value() == "%"
                       && !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader(","), !r.read()));

    SST_TEST_BOOL((r = simple_kv_reader(",,"), !r.read()));

    SST_TEST_THROW((r = simple_kv_reader(",k,"), r.read()),
                   simple_kv_syntax_error);

    SST_TEST_BOOL((r = simple_kv_reader(",k,:"),
                   r.read() && r.key() == "k," && r.value() == ""));

    SST_TEST_THROW((r = simple_kv_reader("%"), r.read()),
                   simple_kv_syntax_error);

    ;
  });
}
