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

#ifndef KESTREL_KESTREL_RACEHANDLE_H
#define KESTREL_KESTREL_RACEHANDLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t kestrel_RaceHandle;

typedef kestrel_RaceHandle KESTREL_handle;
#define KESTREL_HANDLE_C(N) ((KESTREL_handle) + N)

#define KESTREL_HANDLE_NULL KESTREL_HANDLE_C(0)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef KESTREL_KESTREL_RACEHANDLE_H