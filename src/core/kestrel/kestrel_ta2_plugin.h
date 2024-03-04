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

#ifndef KESTREL_KESTREL_TA2_PLUGIN_H
#define KESTREL_KESTREL_TA2_PLUGIN_H

#include <stdint.h>

#include <kestrel/kestrel_LinkType.h>
#include <kestrel/kestrel_PluginResponse.h>
#include <kestrel/kestrel_RaceHandle.h>
#include <kestrel/kestrel_bool.h>
#include <kestrel/kestrel_char.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct kestrel_ta2_plugin kestrel_ta2_plugin;

struct kestrel_ta2_plugin {

  void * plugin;

  kestrel_char * name;

  kestrel_PluginResponse (*activateChannel)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * channelGid,
      kestrel_char const * roleName);

  kestrel_PluginResponse (*closeConnection)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * connectionId);

  kestrel_PluginResponse (*createLink)(void * plugin,
                                       kestrel_RaceHandle handle,
                                       kestrel_char const * channelGid);

  kestrel_PluginResponse (*deactivateChannel)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * channelGid);

  kestrel_PluginResponse (*destroyLink)(void * plugin,
                                        kestrel_RaceHandle handle,
                                        kestrel_char const * linkId);

  kestrel_PluginResponse (*flushChannel)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * channelGid,
      uint64_t batchId);

  kestrel_PluginResponse (*init)(void * plugin);

  kestrel_PluginResponse (*loadLinkAddress)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * channelGid,
      kestrel_char const * linkAddress);

  kestrel_PluginResponse (*onUserAcknowledgementReceived)(
      void * plugin,
      kestrel_RaceHandle handle);

  kestrel_PluginResponse (*onUserInputReceived)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_bool answered,
      kestrel_char const * response);

  kestrel_PluginResponse (*openConnection)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_LinkType linkType,
      kestrel_char const * linkId,
      kestrel_char const * linkHints,
      int32_t sendTimeout);

  kestrel_PluginResponse (*sendPackage)(
      void * plugin,
      kestrel_RaceHandle handle,
      kestrel_char const * connectionId,
      uint8_t const * pkg,
      uint64_t pkgSize,
      uint64_t timeoutTimestamp,
      uint64_t batchId);

  void (*shutdown)(void * plugin);

}; //

#ifdef __cplusplus
} // extern "C"
#endif

#endif // #ifndef KESTREL_KESTREL_TA2_PLUGIN_H
