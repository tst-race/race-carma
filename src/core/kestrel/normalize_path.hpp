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

#ifndef KESTREL_NORMALIZE_PATH_HPP
#define KESTREL_NORMALIZE_PATH_HPP

#include <string>

namespace kestrel {

//
// This function normalizes a path x for use with the RACE SDK's
// filesystem functions. The following steps are taken:
//
//    1. If x does not match the regular expression
//       /[-%./0-9A-Z_a-z]+/, an exception is thrown.
//
//    2. If x begins with a slash, "." is prepended to x.
//
//    3. If x ends with a slash, "." is appended to x.
//
//    4. Each maximal sequence of slashes is replaced with a
//       single slash.
//
//    5. Each "." component is deleted. If this results in x being
//       empty, x is changed to ".".
//
//    6. Starting from the left, each ".." component is deleted.
//       After each deletion, if a previous component exists, it
//       is also deleted. If this results in x being empty, x is
//       changed to ".".
//

std::string normalize_path(std::string x);

} // namespace kestrel

#endif // #ifndef KESTREL_NORMALIZE_PATH_HPP
