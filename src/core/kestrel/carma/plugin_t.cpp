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
#include <kestrel/carma/plugin_t.hpp>
// Include twice to test idempotence.
#include <kestrel/carma/plugin_t.hpp>
//

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_TEV_DEF.hpp>
#include <sst/catalog/SST_TEV_ADD.hpp>
#include <sst/catalog/SST_TEV_ARG.hpp>
#include <sst/catalog/SST_TEV_RETHROW.hpp>
#include <sst/catalog/checked_t.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/copy_bytes.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/integer_rep.hpp>
#include <sst/catalog/json/get_from_string.hpp>
#include <sst/catalog/json/get_to.hpp>
#include <sst/catalog/min.hpp>
#include <sst/catalog/str_cmp.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/type_max.hpp>
#include <sst/catalog/what.hpp>
#include <sst/time.h>

// Other headers
#include <kestrel/json_t.hpp>
#include <kestrel/json_t.hpp>

// RACE headers
#include <ChannelProperties.h>
#include <ChannelStatus.h>
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <EncPkg.h>
#include <LinkProperties.h>
#include <LinkStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>

// CARMA headers
#include <kestrel/carma/contains.hpp>
#include <kestrel/carma/local_config_t.hpp>
#include <kestrel/carma/phonebook_entry_t.hpp>
#include <kestrel/carma/phonebook_pair_t.hpp>
#include <kestrel/channel_id_t.hpp>
#include <kestrel/channel_status_t.hpp>
#include <kestrel/channel_t.hpp>
#include <kestrel/config.h>
#include <kestrel/connection_id_t.hpp>
#include <kestrel/connection_status_t.hpp>
#include <kestrel/connection_t.hpp>
#include <kestrel/encpkg_t.hpp>
#include <kestrel/link_profile_t.hpp>
#include <kestrel/goodbox_entry_t.hpp>
#include <kestrel/link_address_packet_t.hpp>
#include <kestrel/link_address_t.hpp>
#include <kestrel/link_status_t.hpp>
#include <kestrel/message_status_t.hpp>
#include <kestrel/open_connection_call_t.hpp>
#include <kestrel/outbox_entry_t.hpp>
#include <kestrel/package_status_t.hpp>
#include <kestrel/packet_packet_t.hpp>
#include <kestrel/psn_t.hpp>
#include <kestrel/pkc.hpp>
#include <kestrel/plugin_state_t.hpp>
#include <kestrel/sdk_wrapper_t.hpp>
#include <kestrel/tracing_event_t.hpp>
#include <kestrel/tracing_exception_t.hpp>

namespace kestrel {
namespace carma {

//----------------------------------------------------------------------
// Channel tracking
//----------------------------------------------------------------------

void plugin_t::update_channels(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    CARMA_LOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "starting_channel_update"));
    auto const xs = sdk_.getAllChannelProperties(SST_TEV_ARG(tev));
    {
      primary_lock_t const primary_lock(*this->primary_mutex());
      for (auto const & properties : xs) {
        auto const id = channel_id_t(properties.channelGid);
        auto const it = channels_.find(id);
        if (it == channels_.end()) {
          channels_.emplace(std::piecewise_construct,
                            std::forward_as_tuple(id),
                            std::forward_as_tuple(SST_TEV_ARG(tev),
                                                  sdk_,
                                                  properties));
        } else {
          auto & channel = it->second;
          channel.properties(properties);
        }
      }
    }
    CARMA_LOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "finished_channel_update"));
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// Link tracking
//----------------------------------------------------------------------

void plugin_t::add_link(
    tracing_event_t tev,
    link_id_t const & id,
    LinkProperties const & properties,
    std::unordered_set<psn_t> const & personas) {
  SST_TEV_ADD(tev);
  try {
    link_type_t const type(properties.linkType);
    std::vector<std::string> ps;
    std::vector<std::reference_wrapper<std::set<link_id_t>>> ys;
    for (auto const & persona : personas) {
      ps.push_back(persona.string());
      ys.emplace_back(link_sets_[type][persona]);
    }
    process_raw_link_ids(SST_TEV_ARG(tev),
                         ps,
                         ys,
                         std::vector<LinkID>{id.string()});
  }
  SST_TEV_RETHROW(tev);
}

void plugin_t::add_link(tracing_event_t tev,
                              link_id_t const & id,
                              LinkProperties const & properties,
                              psn_t const & persona) {
  SST_TEV_ADD(tev);
  try {
    add_link(SST_TEV_ARG(tev),
             id,
             properties,
             std::unordered_set<psn_t>{persona});
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// createLink call tracking
//----------------------------------------------------------------------

race_handle_t plugin_t::create_link(tracing_event_t tev,
                                    channel_t & channel,
                                    psn_t const & persona) {
  SST_TEV_ADD(tev);
  try {
    race_handle_t const handle(
        sdk_.createLink(SST_TEV_ARG(tev),
                        channel.id().string(),
                        std::vector<std::string>{persona.string()},
                        0));
    create_link_calls_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(handle, channel, persona));
    return handle;
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// createLinkFromAddress call tracking
//----------------------------------------------------------------------

void plugin_t::create_link_from_address(
    tracing_event_t tev,
    channel_t & channel,
    link_profile_t const & link_profile) {
  SST_TEV_ADD(tev);
  try {
    race_handle_t const handle(
        sdk_.createLinkFromAddress(SST_TEV_ARG(tev),
                                   channel.id().string(),
                                   link_profile.address(),
                                   link_profile.personas_vector(),
                                   0));
    create_link_from_address_calls_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(handle,
                              channel,
                              link_profile.personas()));
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// loadLinkAddress call tracking
//----------------------------------------------------------------------

void plugin_t::load_link(tracing_event_t tev,
                               channel_id_t const & channel_id,
                               link_address_t const & link_address,
                               psn_t const & persona) {
  SST_TEV_ADD(tev);
  try {
    race_handle_t const handle(
        sdk_.loadLinkAddress(SST_TEV_ARG(tev),
                             channel_id.string(),
                             link_address.string(),
                             std::vector<std::string>{persona.string()},
                             0));
    load_link_calls_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(handle, channel_id, persona));
  }
  SST_TEV_RETHROW(tev);
}

void plugin_t::load_link(tracing_event_t tev,
                               channel_id_t const & channel_id,
                               link_profile_t const & link_profile) {
  SST_TEV_ADD(tev);
  try {
    race_handle_t const handle(
        sdk_.loadLinkAddress(SST_TEV_ARG(tev),
                             channel_id.string(),
                             link_profile.address(),
                             link_profile.personas_vector(),
                             0));
    load_link_calls_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(handle,
                              channel_id,
                              link_profile.personas()));
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// openConnection call tracking
//----------------------------------------------------------------------

void plugin_t::open_connection(tracing_event_t tev,
                                     link_t & link) {
  SST_TEV_ADD(tev);
  try {
    race_handle_t const handle(sdk_.openConnection(
        SST_TEV_ARG(tev),
        link.properties().linkType,
        link.id().string(),
        "{}",
        0,
        sst::checked_cast<int32_t>(old_config_.send_timeout()),
        0));
    open_connection_calls_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(handle),
        std::forward_as_tuple(handle, link.id()));
  }
  SST_TEV_RETHROW(tev);
}

void plugin_t::ensure_connection(tracing_event_t tev,
                                       link_t & link) {
  SST_TEV_ADD(tev);
  try {
    if (link.connections().empty()) {
      open_connection(SST_TEV_ARG(tev), link);
    }
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------

namespace {

template<class K, class V, class It>
bool subtract(std::map<K, V> & xs, It a, It const b) {
  bool overlap = false;
  auto it = xs.begin();
  while (it != xs.end() && a != b) {
    if (it->first < *a) {
      ++it;
    } else {
      if (!(*a < it->first)) {
        it = xs.erase(it);
        overlap = true;
      }
      ++a;
    }
  }
  return overlap;
}

} // namespace

//----------------------------------------------------------------------
// Link discovery
//----------------------------------------------------------------------

void plugin_t::run_link_discovery(tracing_event_t tev) {
  SST_TEV_ADD(tev);
  try {
    CARMA_LOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "starting_link_discovery"));
    primary_lock_t const primary_lock(*this->primary_mutex());
    // TODO: This was part of the "if I can't talk to one of my
    //       mailboxes, use the registration server" logic. It's left
    //       here commented out for reference. It should eventually be
    //       removed, as registration servers are defunct.
/*
    auto const & entry =
        config().phonebook().at(SST_TEV_ARG(tev), persona());
    switch (entry.role()) {
      case role_t::client(): {
        // TODO: Doing naive exhaustive creation probably won't be good
        //       long term.
        for (auto const & mailbox : entry.bucket_mb_servers()) {
          bool const can_send = [&]() {
            for (auto const & link_type :
                 {link_type_t::send(), link_type_t::bidi()}) {
              if (!link_sets_[link_type][mailbox].empty()) {
                return true;
              }
            }
            return false;
          }();
          bool const can_recv = [&]() {
            for (auto const & link_type :
                 {link_type_t::recv(), link_type_t::bidi()}) {
              if (!link_sets_[link_type][mailbox].empty()) {
                return true;
              }
            }
            return false;
          }();
          for (auto & channel_kv : channels_) {
            auto & channel = channel_kv.second;
            if (channel.properties().channelStatus
                != CHANNEL_AVAILABLE) {
              continue;
            }
            auto const & direction = channel.properties().linkDirection;
            bool b = false;
            b = b || (!can_send && direction == LD_CREATOR_TO_LOADER);
            b = b || (!can_recv && direction == LD_LOADER_TO_CREATOR);
            b = b || ((!can_send || !can_recv) && direction == LD_BIDI);
            if (b) {
              create_link(SST_TEV_ARG(tev), channel, mailbox);
            }
          }
        }
      } break;
      default: {
      } break;
    }
*/
    CARMA_LOG_INFO(
        sdk_,
        0,
        SST_TEV_ARG(tev, "event", "finished_link_discovery"));
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// do_network_maintenance
//----------------------------------------------------------------------
//
// TODO: This thread should eventually become a generic periodic event
//       dispatcher thread, not specifically network maintenance. Each
//       periodic event can run in its own thread, and to clean up, we
//       can join the dispatcher followed by any such existent threads.
//

void plugin_t::process_raw_link_ids(
    tracing_event_t tev,
    std::vector<std::string> const & personas,
    std::vector<std::reference_wrapper<std::set<link_id_t>>> const &
        link_sets,
    std::vector<LinkID> const & raw_link_ids) {
  role_t const role = config().phonebook().at(SST_TEV_ARG(tev), persona())->role();
  for (LinkID const & raw_link_id : raw_link_ids) {
    link_id_t const link_id(raw_link_id);
    auto link_it = links_.find(link_id);
    if (link_it == links_.end()) {
      link_it = links_
                    .emplace(link_id,
                             link_t(SST_TEV_ARG(tev),
                                    old_config_,
                                    sdk_,
                                    link_id,
                                    personas))
                    .first;
    } else {
      link_t & link = link_it->second;
      link.add_personas(personas);
    }
    link_t & link = link_it->second;
    if (role == role_t::client()
        && link.properties().connectionType != CT_INDIRECT) {
      continue;
    }
    ensure_connection(SST_TEV_ARG(tev), link);
    for (auto & link_set : link_sets) {
      link_set.get().emplace(link_id);
    }
  }
}

void plugin_t::do_network_maintenance(tracing_event_t tev) {

  local_config_t & local = config().local();

  //--------------------------------------------------------------------
  // Update the channels map
  //--------------------------------------------------------------------

  try {
    auto lock = channel_update_cooldown_mutex_.try_lock();
    if (lock) {
      if (channel_update_cooldown_future_.valid()) {
        try {
          channel_update_cooldown_future_.get();
        } catch (...) {
          // TODO: Log any exceptions.
        }
      }
      channel_update_cooldown_future_ = std::async(
          std::launch::async,
          [this](tracing_event_t tev, decltype(lock)) {
            update_channels(SST_TEV_ARG(tev));
          },
          SST_TEV_ARG(tev),
          std::move(lock));
    }
  } catch (...) {
    // TODO: Log any exceptions.
  }

  //--------------------------------------------------------------------
  // Link discovery
  //--------------------------------------------------------------------

  try {
    auto lock = link_discovery_cooldown_mutex_.try_lock();
    if (lock) {
      if (link_discovery_cooldown_future_.valid()) {
        try {
          link_discovery_cooldown_future_.get();
        } catch (...) {
          // TODO: Log any exceptions.
        }
      }
      link_discovery_cooldown_future_ = std::async(
          std::launch::async,
          [this](tracing_event_t tev, decltype(lock)) {
            run_link_discovery(SST_TEV_ARG(tev));
          },
          SST_TEV_ARG(tev),
          std::move(lock));
    }
  } catch (...) {
    // TODO: Log any exceptions.
  }

  //--------------------------------------------------------------------

  auto const current_time_ns = sst::mono_time_ns();

  //--------------------------------------------------------------------
  // Link polling maintenance
  //--------------------------------------------------------------------

  if ((link_polling_maintenance_previous_ns_ == 0
       || std::chrono::nanoseconds(
              current_time_ns - link_polling_maintenance_previous_ns_)
              >= link_polling_maintenance_cooldown_)) {
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                                 "event",
                                 "starting_link_polling_maintenance"));
    link_polling_maintenance_previous_ns_ = current_time_ns;

    for (auto const & psn_pbe : config().phonebook()) {
      psn_t const & persona = psn_pbe.first;
      std::shared_ptr<phonebook_entry_t const> const node =
          config().phonebook().at(SST_TEV_ARG(tev), persona);
      for (auto const & link_type : {link_type_t::send(),
                                     link_type_t::recv(),
                                     link_type_t::bidi()}) {
        bool const normal =
            !old_config_.dynamic_only() || link_type == link_type_t::recv();
        if (normal
            && ((link_type.can_send()
                 && contains(
                     config().local().tx_nodes(SST_TEV_ARG(tev)),
                     persona))
                || (link_type.can_recv()
                    && contains(
                        config().local().rx_nodes(SST_TEV_ARG(tev)),
                        persona)))) {
          auto & link_set = link_sets_[link_type][persona];
          std::vector<std::string> const personas{persona.string()};
          process_raw_link_ids(
              SST_TEV_ARG(tev),
              personas,
              {std::ref(link_set)},
              sdk_.getLinksForPersonas(SST_TEV_ARG(tev),
                                       personas,
                                       link_type.value()));
        }
      }
    }

    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                                 "event",
                                 "finished_link_polling_maintenance"));
  }

  //--------------------------------------------------------------------
  // Send retrying maintenance
  //--------------------------------------------------------------------

  if (send_retrying_maintenance_previous_ns_ == 0
      || std::chrono::nanoseconds(
             current_time_ns - send_retrying_maintenance_previous_ns_)
             >= send_retrying_maintenance_cooldown_) {
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                                 "event",
                                 "starting_send_retrying_maintenance"));
    send_retrying_maintenance_previous_ns_ = current_time_ns;

    // Probabilistically resend any outbox entries.
    std::uniform_int_distribution<decltype(sst::mono_time_ns())> dist(
        0,
        sst::checked_cast<decltype(sst::mono_time_ns())>(
            old_config_.send_retry_window() * 1000000000));
    for (auto it = outbox_.begin(); it != outbox_.end();) {
      auto & entry = **it;
      if (subtract(goodbox_,
                   entry.handles.begin(),
                   entry.handles.end())) {
        if (local.role() == role_t::client()) {
          try {
            if (entry.origin_handle() != race_handle_t::null()) {
              sdk_.onMessageStatusChanged(
                  SST_TEV_ARG(tev),
                  entry.origin_handle().value(),
                  message_status_t::sent().value());
            }
          } catch (...) {
          }
        }
        it = outbox_.erase(it);
      } else if (entry.attempts.size()
                 > old_config_.send_retry_count()) {
        if (local.role() == role_t::client()) {
          try {
            if (entry.origin_handle() != race_handle_t::null()) {
              sdk_.onMessageStatusChanged(
                  SST_TEV_ARG(tev),
                  entry.origin_handle().value(),
                  message_status_t::failed().value());
            }
          } catch (...) {
          }
        }
        it = outbox_.erase(it);
      } else if (entry.attempts.empty()) {
        send(SST_TEV_ARG(tev), entry);
        ++it;
      } else {
        auto const x = current_time_ns - entry.attempts.back()->time;
        if (dist(sst::crypto_rng()) < x) {
          send(SST_TEV_ARG(tev), entry);
        }
        ++it;
      }
    }

    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev,
                                 "event",
                                 "finished_send_retrying_maintenance"));
  }

  //--------------------------------------------------------------------
}

//----------------------------------------------------------------------

// TODO: This is left here commented out for reference regarding how to
//       use packet_packet. It should eventually be removed, as
//       registration servers are defunct.
/*
void plugin_t::processEncPkg_reg_server(
    tracing_event_t tev,
    process_message_context_t & pmc,
    deserialize_packet_result const & dpr) {
  SST_TEV_ADD(tev);
  try {

    if (dpr.packet_type != packet_type_t::packet_packet()) {
      CARMA_LOG_WARN(
          sdk_,
          0,
          SST_TEV_ARG(tev, "event", "unexpected_packet_type"));
      return;
    }

    auto const packet_ptr = acquire<packet_packet_t>();
    auto & packet = *packet_ptr;

    packet.init(pmc.origin_handle(), pmc.origin_span());
    packet_from_bytes_exact(*dpr.packet_data, packet);

    pmc.send_message(SST_TEV_ARG(tev),
                     packet.packet,
                     {},
                     packet.receiver);
  }
  SST_TEV_RETHROW(tev);
}
*/

//----------------------------------------------------------------------
// processEncPkg_idle_server
//----------------------------------------------------------------------

void plugin_t::processEncPkg_idle_server(
    tracing_event_t tev,
    process_message_context_t &,
    deserialize_packet_result const &) {
  SST_TEV_ADD(tev);
  try {
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// processEncPkg
//----------------------------------------------------------------------

void plugin_t::processEncPkg(
    tracing_event_t tev,
    race_handle_t const & handle,
    EncPkg const & ePkg,
    std::vector<ConnectionID> const &) {
  SST_TEV_ADD(tev);
  try {

    local_config_t & local = config().local();

    auto const encpkg_ptr = acquire<encpkg_t>();
    encpkg_ptr->init(ePkg);
    auto const & encpkg = *encpkg_ptr;

    SST_TEV_ADD(tev, "incoming_encpkg", encpkg.to_json());
    CARMA_LOG_INFO(sdk_,
                   0,
                   SST_TEV_ARG(tev, "event", "incoming_encpkg"));

    if (encpkg.blob().size() == 0) {
      CARMA_LOG_WARN(sdk_,
                     0,
                     SST_TEV_ARG(tev, "event", "zero_sized_encpkg"));
    }

    process_message_context_t pmc(sdk_, *this, handle, ePkg);

    unsigned char const * packet_src = encpkg.blob().data();
    auto avail = encpkg.blob().size();
    decltype(avail) old_avail = 0;
    while (avail != 0 && avail != old_avail) {
      old_avail = avail;
      try {
        deserialize_packet_result dpr =
            deserialize_packet(SST_TEV_ARG(tev),
                               pmc,
                               packet_src,
                               avail);

        SST_TEV_ADD(tev,
                      "incoming_psn",
                      dpr.psn->string(),
                      "incoming_packet_id",
                      sst::to_hex(*dpr.packet_id),
                      "incoming_packet_type",
                      dpr.packet_type.to_json());

        if (dpr.node_info == nullptr) {
          throw std::runtime_error("I don't know this node");
        }

        if (processed_packet_ids_.count(*dpr.packet_id) != 0) {
          CARMA_LOG_TRACE(
              sdk_,
              0,
              SST_TEV_ARG(tev, "event", "duplicate_packet_id"));
          return;
        }
        processed_packet_ids_.insert(*dpr.packet_id);

        switch (local.role()) {
          case role_t::client(): {
            client_processEncPkg(SST_TEV_ARG(tev), pmc, dpr);
          } break;
          case role_t::idle_server(): {
            processEncPkg_idle_server(SST_TEV_ARG(tev), pmc, dpr);
          } break;
          case role_t::mb_server(): {
            processEncPkg_mb_server(SST_TEV_ARG(tev), pmc, dpr);
          } break;
          case role_t::mc_leader():
          case role_t::mc_follower(): {
            mc_server_processEncPkg(SST_TEV_ARG(tev), pmc, dpr);
          } break;
          case role_t::rs_server(): {
            // TODO
            SST_UNREACHABLE();
          } break;
        }
      } catch (tracing_exception_t const & e) {
        if (avail == old_avail) {
          // avail == old_avail means deserialize_packet didn't consume
          // a complete packet, which means something is very wrong.
          // TODO: Should we search forward for another packet?
          throw;
        } else {
          // Otherwise, deserialize_packet did consume a complete
          // packet, so we can log the error that occurred while
          // processing it and move to the next packet.
          CARMA_LOG_ERROR(sdk_,
                          0,
                          SST_TEV_ARG(e.tev(),
                                        "event",
                                        "packet_processing_error",
                                        "exception",
                                        sst::what()));
        }
      } catch (...) {
        if (avail == old_avail) {
          throw;
        } else {
          CARMA_LOG_ERROR(sdk_,
                          0,
                          SST_TEV_ARG(tev,
                                        "event",
                                        "packet_processing_error",
                                        "exception",
                                        sst::what()));
        }
      }
    }
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// inner_onLinkPropertiesChanged
//----------------------------------------------------------------------

void plugin_t::inner_onLinkPropertiesChanged(
    tracing_event_t tev,
    link_id_t const & link_id,
    LinkProperties const & link_properties) {
  SST_TEV_ADD(tev);
  try {
    auto link_it = links_.find(link_id);
    if (link_it != links_.end()) {
      link_t & link = link_it->second;
      link.properties(link_properties);
    }
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// onUserInputReceived
//----------------------------------------------------------------------

void plugin_t::onUserInputReceived(tracing_event_t tev,
                                   race_handle_t const &,
                                   bool,
                                   std::string const &) {
  SST_TEV_ADD(tev);
  try {
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------

void plugin_t::onPersonaLinksChanged(
    tracing_event_t tev,
    psn_t const & persona,
    link_type_t const & link_type,
    std::vector<LinkID> const & link_ids) {
  SST_TEV_ADD(tev);
  try {
    // TODO: Validate link_type.
    auto & link_set = link_sets_[link_type][persona];
    link_set.clear();
    for (auto const & link_id : link_ids) {
      link_set.emplace(link_id);
    }
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------

race_handle_t plugin_t::send(tracing_event_t tev,
                                   outbox_entry_t & outbox_entry) {
  SST_TEV_ADD(tev);
  try {
    local_config_t & local = config().local();

    psn_t const & persona = outbox_entry.psn;
    SST_TEV_ADD(tev, "recipient", persona.string());

    auto const candidate_links_ptr =
        acquire<std::vector<std::reference_wrapper<link_t>>>();
    auto & candidate_links = *candidate_links_ptr;
    candidate_links.clear();
    for (auto const & link_type :
         {link_type_t::send(), link_type_t::bidi()}) {
      auto & link_set = link_sets_[link_type][persona];
      for (auto link_id_it = link_set.begin();
           link_id_it != link_set.end();) {
        auto const link_it = links_.find(*link_id_it);
        // If the link ID doesn't exist in the main links map, then it's
        // a stale entry in the link ID set. Remove it and continue to
        // the next entry in the link ID set.
        if (link_it == links_.end()) {
          link_id_it = link_set.erase(link_id_it);
          continue;
        }
        ++link_id_it;
        auto & link = link_it->second;
        // If the link has no connections open, then something is wrong
        // with it and we can't use it. It's the responsibility of the
        // network maintenance thread to handle this, not ours. Ignore
        // it and continue to the next entry in the link ID set.
        if (link.connections().empty()) {
          continue;
        }
        std::shared_ptr<phonebook_entry_t const> const node_ptr =
            config().phonebook().at(SST_TEV_ARG(tev), outbox_entry.psn);
        auto const & node = *node_ptr;
        if ((local.role() == role_t::client()
             || node.role() == role_t::client())
            && link.properties().connectionType != CT_INDIRECT) {
          continue;
        }
        if (old_config_.inter_server_direct_only
            && local.role() != role_t::client()
            && node.role() != role_t::client()
            && link.properties().connectionType != CT_DIRECT) {
          continue;
        }
        // Otherwise, the link is a candidate.
        candidate_links.emplace_back(link);
      }
    }

    if (candidate_links.empty()) {
      CARMA_LOG_WARN(sdk_,
                     0,
                     SST_TEV_ARG(tev, "event", "no_candidate_links"));
      return race_handle_t::null();
    }

    // Determine the subset of candidate_links we haven't attempted to
    // send this message over yet.
    auto const unattempted_links_ptr =
        acquire<std::vector<std::reference_wrapper<link_t>>>();
    auto & unattempted_links = *unattempted_links_ptr;
    unattempted_links.clear();
    for (auto const & link : candidate_links) {
      bool attempted = false;
      for (link_id_t const & id : outbox_entry.link_ids) {
        if (id == link.get().id()) {
          attempted = true;
          break;
        }
      }
      if (!attempted) {
        unattempted_links.emplace_back(link);
      }
    }

    // Pick the link to try send over.
    link_t & link = [&]() -> link_t & {
      using T = unsigned long long;
      // If we've already tried to send over all candidate links, just
      // choose a random candidate link to try again.
      if (unattempted_links.empty()) {
        auto const max1 = candidate_links.size() - 1;
        auto const max2 = sst::type_max<T>::value;
        auto const max = static_cast<T>(max1 < max2 ? max1 : max2);
        std::uniform_int_distribution<T> d(0, max);
        return candidate_links[d(sst::crypto_rng())];
      }
      // Otherwise, probabilistically choose from all unattempted links
      // based on their latency.
      auto const p_cumulative_sums = acquire<std::vector<double>>();
      auto & cumulative_sums = *p_cumulative_sums;
      cumulative_sums.clear();
      double total_sum = 0;
      for (auto const & link_ref : unattempted_links) {
        auto const & link = link_ref.get();
        // TODO: We need a more sophisticated probabilistic scoring
        // system. In particular, clamping (-inf,0] to 1 was mostly a
        // quick fix for certain channels reporting bad values in IE2.
        double score = link.properties().expected.send.latency_ms;
        if (score < 1) {
          score = 1;
        }
        score = 100100 / (score + 1000);
        total_sum += score;
        cumulative_sums.push_back(total_sum);
      }
      auto const max = sst::type_max<T>::value;
      std::uniform_int_distribution<T> d(0, max);
      double const r = d(sst::crypto_rng());
      auto const p = r / max * total_sum;
      auto const n = cumulative_sums.size();
      for (decltype(+n) i = 0; i != n; ++i) {
        if (p <= cumulative_sums[i]) {
          return unattempted_links[i];
        }
      }
      return unattempted_links.back();
    }();

    SST_TEV_ADD(tev, "outgoing_link", link.to_json());

    connection_t & connection = link.connections().begin()->second;

    // TODO: Use encpkg_t throughout the call chain. We want to use
    // encpkg_t as much as possible and only call .to_EncPkg() at the
    // last second when calling sendEncryptedPackage.
    auto const encpkg_ptr = acquire<encpkg_t>();
    auto & encpkg = *encpkg_ptr;
    encpkg.init(outbox_entry.message);

    auto const handle = connection.send(SST_TEV_ARG(tev), encpkg);

    auto attempt = acquire<outbox_entry_t::attempt_t>();
    attempt->time = sst::mono_time_ns();
    attempt->link_id = link.id();
    attempt->connection_id = connection.id();
    attempt->handle = handle;
    outbox_entry.attempts.emplace_back(std::move(attempt));
    outbox_entry.link_ids.emplace(link.id());
    outbox_entry.handles.emplace(handle);
    return handle;
  }
  SST_TEV_RETHROW(tev);
}

//----------------------------------------------------------------------
// IRacePluginNM function overrides
//----------------------------------------------------------------------
//
// TODO: Use templates to implement BODY, not a macro.
//
// TODO: Logging for incoming parameters, similar to how we have i_foo
//       for SDK calls.
//

#define BODY(plugin_function, failure_response, tev, next_call, ...)   \
  primary_lock_t const primary_lock(*this->primary_mutex());                   \
  PluginResponse plugin_response = failure_response;                   \
  tracing_event_t SST_TEV_DEF(tev);                                              \
  SST_TEV_ADD(tev, "local_psn", this->persona().value());         \
  SST_TEV_ADD(tev,                                                   \
                "plugin_function",                                     \
                #plugin_function,                                      \
                "plugin_call_id",                                      \
                sst::to_string(plugin_call_id_++));                    \
  CARMA_LOG_TRACE(                                                     \
      sdk_,                                                            \
      0,                                                               \
      SST_TEV_ARG(tev, "event", "plugin_function_called"));          \
  try {                                                                \
    if (plugin_state_ == plugin_state_t::broken) {                                    \
      plugin_response = PLUGIN_FATAL;                                  \
      try {                                                            \
        throw std::runtime_error("plugin called after PLUGIN_FATAL");  \
      }                                                                \
      SST_TEV_RETHROW(tev);                                          \
    }                                                                  \
    if (primary_mutex_entered_) {                                                  \
      plugin_response = PLUGIN_ERROR;                                  \
      try {                                                            \
        throw std::runtime_error("plugin called recursively");         \
      }                                                                \
      SST_TEV_RETHROW(tev);                                          \
    }                                                                  \
    if (sst::str_cmp(#plugin_function, "init") == 0) {                 \
      if (plugin_state_ == plugin_state_t::initialized) {                                \
        plugin_response = PLUGIN_ERROR;                                \
        try {                                                          \
          throw std::runtime_error("Plugin already started.");         \
        }                                                              \
        SST_TEV_RETHROW(tev);                                        \
      }                                                                \
    } else if (plugin_state_ != plugin_state_t::initialized) {                           \
      plugin_response = PLUGIN_ERROR;                                  \
      try {                                                            \
        throw std::runtime_error("Plugin not started.");               \
      }                                                                \
      SST_TEV_RETHROW(tev);                                          \
    }                                                                  \
    try {                                                              \
      primary_mutex_entered_ = true;                                               \
      next_call;                                                       \
      primary_mutex_entered_ = false;                                              \
    } catch (...) {                                                    \
      primary_mutex_entered_ = false;                                              \
      throw;                                                           \
    }                                                                  \
    if (sst::str_cmp(#plugin_function, "init") == 0) {                 \
      plugin_state_ = plugin_state_t::initialized;                                       \
    } else if (sst::str_cmp(#plugin_function, "shutdown") == 0) {      \
      plugin_state_ = plugin_state_t::uninitialized;                                       \
    }                                                                  \
    CARMA_LOG_TRACE(                                                   \
        sdk_,                                                          \
        0,                                                             \
        SST_TEV_ARG(tev, "event", "plugin_function_succeeded"));     \
    return PLUGIN_OK;                                                  \
  } catch (tracing_exception_t const & e) {                                \
    if (plugin_response == PLUGIN_FATAL) {                             \
      try {                                                            \
        inner_shutdown(SST_TEV_ARG(tev));                                  \
      } catch (...) {                                                  \
      }                                                                \
      plugin_state_ = plugin_state_t::broken;                                         \
      CARMA_LOG_FATAL(sdk_,                                            \
                      0,                                               \
                      SST_TEV_ARG(e.tev(),                           \
                                    "event",                           \
                                    "plugin_function_failed",          \
                                    "exception",                       \
                                    sst::what()));           \
    } else {                                                           \
      assert(plugin_response == PLUGIN_ERROR);                         \
      CARMA_LOG_ERROR(sdk_,                                            \
                      0,                                               \
                      SST_TEV_ARG(e.tev(),                           \
                                    "event",                           \
                                    "plugin_function_failed",          \
                                    "exception",                       \
                                    sst::what()));           \
    }                                                                  \
    return plugin_response;                                            \
  } catch (...) {                                                      \
    if (plugin_response == PLUGIN_FATAL) {                             \
      try {                                                            \
        inner_shutdown(SST_TEV_ARG(tev));                                  \
      } catch (...) {                                                  \
      }                                                                \
      plugin_state_ = plugin_state_t::broken;                                         \
      CARMA_LOG_FATAL(sdk_,                                            \
                      0,                                               \
                      SST_TEV_ARG(tev,                               \
                                    "event",                           \
                                    "plugin_function_failed",          \
                                    "exception",                       \
                                    sst::what()));                         \
    } else {                                                           \
      assert(plugin_response == PLUGIN_ERROR);                         \
      CARMA_LOG_ERROR(sdk_,                                            \
                      0,                                               \
                      SST_TEV_ARG(tev,                               \
                                    "event",                           \
                                    "plugin_function_failed",          \
                                    "exception",                       \
                                    sst::what()));                         \
    }                                                                  \
    return plugin_response;                                            \
  }

PluginResponse plugin_t::processEncPkg(
    RaceHandle handle,
    EncPkg const & ePkg,
    std::vector<ConnectionID> const & connIDs) {
  BODY(processEncPkg,
       PLUGIN_ERROR,
       tev,
       processEncPkg(SST_TEV_ARG(tev),
                     race_handle_t(std::move(handle)),
                     ePkg,
                     connIDs));
}

PluginResponse
plugin_t::onPersonaLinksChanged(std::string recipientPersona,
                                      LinkType linkType,
                                      std::vector<LinkID> const links) {
  BODY(onPersonaLinksChanged,
       PLUGIN_ERROR,
       tev,
       onPersonaLinksChanged(SST_TEV_ARG(tev),
                             psn_t(recipientPersona),
                             link_type_t(std::move(linkType)),
                             links));
}

PluginResponse
plugin_t::onUserInputReceived(RaceHandle handle,
                                    bool answered,
                                    std::string const & response) {
  BODY(onUserInputReceived,
       PLUGIN_ERROR,
       tev,
       onUserInputReceived(SST_TEV_ARG(tev),
                           race_handle_t(std::move(handle)),
                           answered,
                           response));
}

} // namespace carma
} // namespace kestrel
