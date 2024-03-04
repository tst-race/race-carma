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
#include <kestrel/slugify.hpp>
// Include twice to test idempotence.
#include <kestrel/slugify.hpp>
//

#include <string>

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/SST_TEST_SHOW.hpp>
#include <sst/catalog/test_main.hpp>

#include <kestrel/slugify.hpp>

using namespace kestrel;

int main() {
  return sst::test_main([] {
    ;

    std::string x;

    //------------------------------------------------------------------
    // slugify
    //------------------------------------------------------------------

#define F(A, B)                                                        \
  SST_TEST_BOOL((x = slugify((A)), x == (B)), SST_TEST_SHOW((x)))

    F((""), (""));
    F(("%"), ("%%"));
    F(("%%"), ("%%%%"));
    F(("-"), ("%2D"));
    F(("."), ("%2E"));
    F((":"), ("%3A"));
    F(("@"), ("%40"));
    F(("Hello"), ("%48ello"));
    F(("X"), ("%58"));
    F(("hello"), ("hello"));
    F(("shared"), ("share%64"));
    F(("x"), ("x"));

#undef F

    //------------------------------------------------------------------
    // unslugify
    //------------------------------------------------------------------

#define F(A, B)                                                        \
  SST_TEST_BOOL((x = unslugify((A)), x == (B)), SST_TEST_SHOW((x)))

    F((""), (""));
    F(("%"), ("%"));
    F(("%%"), ("%"));
    F(("%%%"), ("%%"));
    F(("%%%%"), ("%%"));
    F(("%2"), ("%2"));
    F(("%21"), ("!"));
    F(("%2A"), ("*"));
    F(("%2D"), ("-"));
    F(("%2E"), ("."));
    F(("%2G"), ("%2G"));
    F(("%2G%"), ("%2G%"));
    F(("%2G%%"), ("%2G%"));
    F(("%2G%%%"), ("%2G%%"));
    F(("%2G%%%%"), ("%2G%%"));
    F(("%2a"), ("*"));
    F(("%2a%"), ("*%"));
    F(("%2a%2"), ("*%2"));
    F(("%2a%2A"), ("**"));
    F(("%2a%2X"), ("*%2X"));
    F(("%2a%2x"), ("*%2x"));
    F(("%2d"), ("-"));
    F(("%2e"), ("."));
    F(("%2g"), ("%2g"));
    F(("%3A"), (":"));
    F(("%3B"), (";"));
    F(("%3b"), (";"));
    F(("%40"), ("@"));
    F(("%48ello"), ("Hello"));
    F(("%58"), ("X"));
    F(("%A2"), ("\xA2"));
    F(("%B3"), ("\xB3"));
    F(("%a2"), ("\xA2"));
    F(("%b3"), ("\xB3"));
    F(("%g9"), ("%g9"));
    F(("-"), ("-"));
    F(("."), ("."));
    F(("hello"), ("hello"));
    F(("share%64"), ("shared"));
    F(("shared"), ("shared"));
    F(("x"), ("x"));

#undef F

    //------------------------------------------------------------------

    ;
  });
}
