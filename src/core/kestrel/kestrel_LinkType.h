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

#ifndef KESTREL_KESTREL_LINKTYPE_H
#define KESTREL_KESTREL_LINKTYPE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t kestrel_LinkType;

typedef kestrel_LinkType KESTREL_LinkType;
#define KESTREL_LT_C(N) ((KESTREL_LinkType) + N)

#define KESTREL_LT_UNDEF KESTREL_LT_C(0)
#define KESTREL_LT_SEND KESTREL_LT_C(1)
#define KESTREL_LT_RECV KESTREL_LT_C(2)
#define KESTREL_LT_BIDI KESTREL_LT_C(3)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef KESTREL_KESTREL_LINKTYPE_H
