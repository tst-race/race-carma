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

#ifndef KESTREL_SLUGIFY_HPP
#define KESTREL_SLUGIFY_HPP

//
// The result of slugifying a std::string is the original string with
// the following modifications:
//
//    1. Each byte that does not match the regular expression
//       /[-.0-9_a-z]/ is percent-encoded with uppercase hex
//       digits.
//
//    2. If the result would begin with a byte that matches the
//       regular expression /[-.]/, that byte is also
//       percent-encoded.
//
//    3. If the result would end with a byte that matches the
//       regular expression /[-.]/, that byte is also
//       percent-encoded.
//
//    4. If the result would be exactly "shared", then the last
//       byte is percent-encoded.
//
// Deslugifying a string simply expands all percent-encoded sequences,
// with any hex digit case. Invalid sequences are maintained.
//

#include <string>

namespace kestrel {

std::string slugify(std::string const & src);

std::string unslugify(std::string const & src);

} // namespace kestrel

#endif // #ifndef KESTREL_SLUGIFY_HPP
