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

#ifndef KESTREL_KESTREL_H
#define KESTREL_KESTREL_H

#include <stdint.h>

#include <kestrel/kestrel_LinkType.h>
#include <kestrel/kestrel_PluginResponse.h>
#include <kestrel/kestrel_RaceHandle.h>
#include <kestrel/kestrel_char.h>
#include <kestrel/kestrel_ta2_plugin.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------

typedef int32_t kestrel_sdk_status;

typedef kestrel_sdk_status KESTREL_sdk_status;
#define KESTREL_SDK_C(N) ((KESTREL_sdk_status) + N)

#define KESTREL_SDK_INVALID KESTREL_SDK_C(0)
#define KESTREL_SDK_OK KESTREL_SDK_C(1)
#define KESTREL_SDK_SHUTTING_DOWN KESTREL_SDK_C(2)
#define KESTREL_SDK_PLUGIN_MISSING KESTREL_SDK_C(3)
#define KESTREL_SDK_INVALID_ARGUMENT KESTREL_SDK_C(4)
#define KESTREL_SDK_QUEUE_FULL KESTREL_SDK_C(5)

//----------------------------------------------------------------------

typedef int32_t kestrel_channel_status;

typedef kestrel_channel_status KESTREL_channel_status;
#define KESTREL_CHANNEL_C(N) ((KESTREL_channel_status) + N)

#define KESTREL_CHANNEL_UNDEF KESTREL_CHANNEL_C(0)
#define KESTREL_CHANNEL_AVAILABLE KESTREL_CHANNEL_C(1)
#define KESTREL_CHANNEL_UNAVAILABLE KESTREL_CHANNEL_C(2)
#define KESTREL_CHANNEL_ENABLED KESTREL_CHANNEL_C(3)
#define KESTREL_CHANNEL_DISABLED KESTREL_CHANNEL_C(4)
#define KESTREL_CHANNEL_STARTING KESTREL_CHANNEL_C(5)
#define KESTREL_CHANNEL_FAILED KESTREL_CHANNEL_C(6)
#define KESTREL_CHANNEL_UNSUPPORTED KESTREL_CHANNEL_C(7)

//----------------------------------------------------------------------

typedef int32_t kestrel_link_status;

typedef kestrel_link_status KESTREL_link_status;
#define KESTREL_LINK_C(N) ((KESTREL_link_status) + N)

#define KESTREL_LINK_UNDEF KESTREL_LINK_C(0)
#define KESTREL_LINK_CREATED KESTREL_LINK_C(1)
#define KESTREL_LINK_LOADED KESTREL_LINK_C(2)
#define KESTREL_LINK_DESTROYED KESTREL_LINK_C(3)

//----------------------------------------------------------------------

typedef int32_t kestrel_connection_status;

typedef kestrel_connection_status KESTREL_connection_status;
#define KESTREL_CONNECTION_C(N) ((KESTREL_connection_status) + N)

#define KESTREL_CONNECTION_INVALID KESTREL_CONNECTION_C(0)
#define KESTREL_CONNECTION_OPEN KESTREL_CONNECTION_C(1)
#define KESTREL_CONNECTION_CLOSED KESTREL_CONNECTION_C(2)
#define KESTREL_CONNECTION_AWAITING_CONTACT KESTREL_CONNECTION_C(3)
#define KESTREL_CONNECTION_INIT_FAILED KESTREL_CONNECTION_C(4)
#define KESTREL_CONNECTION_AVAILABLE KESTREL_CONNECTION_C(5)
#define KESTREL_CONNECTION_UNAVAILABLE KESTREL_CONNECTION_C(6)

//----------------------------------------------------------------------

typedef int32_t kestrel_package_status;

typedef kestrel_package_status KESTREL_package_status;
#define KESTREL_PACKAGE_C(N) ((KESTREL_package_status) + N)

#define KESTREL_PACKAGE_INVALID KESTREL_PACKAGE_C(0)
#define KESTREL_PACKAGE_SENT KESTREL_PACKAGE_C(1)
#define KESTREL_PACKAGE_RECEIVED KESTREL_PACKAGE_C(2)
#define KESTREL_PACKAGE_FAILED_GENERIC KESTREL_PACKAGE_C(3)
#define KESTREL_PACKAGE_FAILED_NETWORK_ERROR KESTREL_PACKAGE_C(4)
#define KESTREL_PACKAGE_FAILED_TIMEOUT KESTREL_PACKAGE_C(5)

//----------------------------------------------------------------------

typedef int32_t kestrel_link_direction;

typedef kestrel_link_direction KESTREL_link_direction;
#define KESTREL_LD_C(N) ((KESTREL_link_direction) + N)

#define KESTREL_LD_UNDEF KESTREL_LD_C(0)
#define KESTREL_LD_CREATOR_TO_LOADER KESTREL_LD_C(1)
#define KESTREL_LD_LOADER_TO_CREATOR KESTREL_LD_C(2)
#define KESTREL_LD_BIDI KESTREL_LD_C(3)

//----------------------------------------------------------------------

typedef struct kestrel_channel_properties kestrel_channel_properties;

struct kestrel_channel_properties {

  kestrel_link_direction link_direction;

}; //

//----------------------------------------------------------------------

typedef struct kestrel_link_properties kestrel_link_properties;

struct kestrel_link_properties {

  kestrel_LinkType link_type;
  kestrel_char const * link_address;

}; //

//----------------------------------------------------------------------

typedef struct kestrel_ta1_sdk kestrel_ta1_sdk;

struct kestrel_ta1_sdk {

  // TODO

}; //

//----------------------------------------------------------------------

typedef struct kestrel_ta2_sdk kestrel_ta2_sdk;

struct kestrel_ta2_sdk {

  kestrel_sdk_status (*get_entropy)(kestrel_ta2_sdk * sdk,
                                    uint8_t * buf,
                                    uint64_t size);

  kestrel_sdk_status (*generate_link_id)(kestrel_ta2_sdk * sdk,
                                         kestrel_char * buf,
                                         uint64_t size,
                                         uint64_t * need);

  kestrel_sdk_status (*generate_connection_id)(kestrel_ta2_sdk * sdk,
                                               kestrel_char * buf,
                                               uint64_t size,
                                               uint64_t * need);

  kestrel_sdk_status (*get_active_persona)(kestrel_ta2_sdk * sdk,
                                           kestrel_char * buf,
                                           uint64_t size,
                                           uint64_t * need);

  kestrel_sdk_status (*async_error)(kestrel_ta2_sdk * sdk,
                                    kestrel_RaceHandle handle,
                                    kestrel_PluginResponse error);

  kestrel_sdk_status (*on_channel_status_changed)(
      kestrel_ta2_sdk * sdk,
      kestrel_RaceHandle handle,
      kestrel_channel_status status);

  kestrel_sdk_status (*on_link_status_changed)(
      kestrel_ta2_sdk * sdk,
      kestrel_RaceHandle handle,
      kestrel_link_status status);

  kestrel_sdk_status (*on_connection_status_changed)(
      kestrel_ta2_sdk * sdk,
      kestrel_RaceHandle handle,
      kestrel_connection_status status);

  kestrel_sdk_status (*on_package_status_changed)(
      kestrel_ta2_sdk * sdk,
      kestrel_RaceHandle handle,
      kestrel_package_status status);

}; //

//----------------------------------------------------------------------

typedef struct kestrel_ta1_plugin kestrel_ta1_plugin;

struct kestrel_ta1_plugin {

  // TODO

}; //

//----------------------------------------------------------------------

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef KESTREL_KESTREL_H
