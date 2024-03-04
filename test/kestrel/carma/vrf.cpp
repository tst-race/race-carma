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
#include <kestrel/carma/vrf.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/vrf.hpp>
//

#include <algorithm>
#include <array>
#include <string>

#include <sst/catalog/SST_TEST_BOOL.hpp>
#include <sst/catalog/test_main.hpp>

using namespace kestrel;
using namespace kestrel::carma;

int main() {
  return sst::test_main([] {
    ;

    std::array<unsigned char, vrf::pk_size()> pk1;
    std::array<unsigned char, vrf::pk_size()> pk2;
    std::array<unsigned char, vrf::sk_size()> sk1;
    std::array<unsigned char, vrf::sk_size()> sk2;
    std::array<unsigned char, vrf::output_size()> output1;
    std::array<unsigned char, vrf::output_size()> output2;
    std::array<unsigned char, vrf::proof_size()> proof1;
    std::array<unsigned char, vrf::proof_size()> proof2;
    std::string input1;
    std::string input2;

    SST_TEST_BOOL((vrf::keygen(pk1, sk1),
                   vrf::keygen(pk2, sk2),
                   input1 = "test1",
                   input2 = "test2",
                   vrf::eval(sk1, input1, proof1, output1),
                   vrf::eval(sk2, input2, proof2, output2),
                   vrf::verify(pk1, input1, proof1)
                       && vrf::verify(pk2, input2, proof2)
                       && !std::equal(output1.begin(),
                                      output1.end(),
                                      output2.begin())
                       && !std::equal(proof1.begin(),
                                      proof1.end(),
                                      proof2.begin())));

    ;
  });
}
