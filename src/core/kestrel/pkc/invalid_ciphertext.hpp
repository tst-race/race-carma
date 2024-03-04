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

#ifndef KESTREL_PKC_INVALID_CIPHERTEXT_HPP
#define KESTREL_PKC_INVALID_CIPHERTEXT_HPP

#include <stdexcept>

namespace kestrel {
namespace pkc {

//
// An invalid_ciphertext object is thrown when an invalid ciphertext is
// detected.
//

class invalid_ciphertext : public std::runtime_error {

public:

  invalid_ciphertext()
      : std::runtime_error(
          "Failed to decrypt a ciphertext. This usually means the "
          "ciphertext was intended for another recipient. Rarely, it "
          "can mean the ciphertext was corrupt. An unintended "
          "recipient cannot distinguish between these two "
          "possibilities.") {
  }

}; //

} // namespace pkc
} // namespace kestrel

#endif // #ifndef KESTREL_PKC_INVALID_CIPHERTEXT_HPP
