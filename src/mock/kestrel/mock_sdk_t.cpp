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
#include <kestrel/mock_sdk_t.hpp>
// Include twice to test idempotence.
#include <kestrel/mock_sdk_t.hpp>

#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <kestrel/post_json_t.hpp>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <mutex>
#include <new>
#include <kestrel/json_t.hpp>
#include <random>
#include <set>
#include <sst/catalog/SST_ASSERT.h>
#include <sst/catalog/SST_STATIC_ASSERT.h>
#include <sst/catalog/SST_THROW_UNIMPLEMENTED.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/checked_cast.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/path.hpp>
#include <sst/catalog/promote.hpp>
#include <sst/catalog/read_whole_file.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_f.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/type_max.hpp>
#include <sst/catalog/type_min.hpp>
#include <sst/catalog/write_whole_file.hpp>
#include <sst/checked.h>
#include <stdexcept>
#include <string>
#include <sys/time.h>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

// RACE SDK
#include <ClrMsg.h>
#include <ConnectionStatus.h>
#include <ConnectionType.h>
#include <EncPkg.h>
#include <IRacePluginNM.h>
#include <LinkStatus.h>
#include <PackageStatus.h>
#include <PluginConfig.h>
#include <PluginResponse.h>
#include <RaceLog.h>
#include <SdkResponse.h>
#include <TransmissionType.h>

#include <kestrel/normalize_path.hpp>
#include <kestrel/slugify.hpp>

// TODO: latency_ms and bandwidth are unaware of which sends were
// multicast. They just see a bunch of individual sends. Is this a
// problem?

namespace kestrel {

namespace {

void string_to_bytes(std::vector<unsigned char> & dst,
                     std::string const & src) {
  auto const n = src.size();
  if (n > 0xFFFFFFFF) {
    throw std::length_error("message too big");
  }
  dst.push_back(static_cast<unsigned char>(n >> 7 >> 7 >> 7 >> 3));
  dst.push_back(static_cast<unsigned char>(n >> 7 >> 7 >> 2));
  dst.push_back(static_cast<unsigned char>(n >> 7 >> 1));
  dst.push_back(static_cast<unsigned char>(n >> 0 >> 0));
  dst.insert(dst.end(), src.cbegin(), src.cend());
}

bool handle_plugin_response(PluginResponse const pr) {
  switch (pr) {
    case PLUGIN_OK:
      return true;
    case PLUGIN_TEMP_ERROR:
    case PLUGIN_ERROR:
      return false;
    case PLUGIN_FATAL:
      throw std::runtime_error("PLUGIN_FATAL");
    default:
      throw std::runtime_error("invalid PluginResponse");
  }
}

nlohmann::json parse_config(nlohmann::json const & unparsed_config) {

  if (!unparsed_config.is_object()) {
    throw std::invalid_argument{"expected config.is_object()"};
  }

  nlohmann::json const config =
      [&](nlohmann::json && a) {
        a.update(unparsed_config);
        return a;
      }(nlohmann::json::object({
          {"rmq_host", "127.0.0.1"},
          {"rmq_port", 5672},
          {"rmq_username", "guest"},
          {"rmq_password", "guest"},
          {"rmq_heartbeat", 0},
          {"etcDirectory", "global.json"},
          {"pluginConfigFilePath", "/config"},
          {"bandwidth_bps", 1000000000000},
          {"latency_ms", 0},
      }));

  auto const expect = [](bool const x, char const * const s) {
    if (!x) {
      throw std::invalid_argument{std::string{"expected "} + s};
    }
  };
#define expect(x) expect(x, #x)

  expect(config.contains("pseudonym"));
  expect(config["pseudonym"].is_string());

  expect(config.contains("rmq_host"));
  expect(config["rmq_host"].is_string());

  expect(config.contains("rmq_port"));
  expect(config["rmq_port"].is_number_integer());
  expect(config["rmq_port"] >= 1);
  expect(config["rmq_port"] <= 65535);

  expect(config.contains("rmq_username"));
  expect(config["rmq_username"].is_string());

  expect(config.contains("rmq_password"));
  expect(config["rmq_password"].is_string());

  expect(config.contains("rmq_heartbeat"));
  expect(config["rmq_heartbeat"].is_number_integer());
  expect(config["rmq_heartbeat"] >= 0);
  expect(config["rmq_heartbeat"] <= 3600);

  expect(config.contains("etcDirectory"));
  expect(config["etcDirectory"].is_string());

  expect(config.contains("pluginConfigFilePath"));
  expect(config["pluginConfigFilePath"].is_string());

  expect(config.contains("bandwidth_bps"));
  expect(config["bandwidth_bps"].is_number_integer());
  expect(config["bandwidth_bps"] >= 1);

  expect(config.contains("latency_ms"));
  expect(config["latency_ms"].is_number_integer());
  expect(config["latency_ms"] >= 0);

  if (config.contains("received_json_post")) {
    expect(config["received_json_post"].contains("address"));
    expect(config["received_json_post"]["address"].is_string());

    expect(config["received_json_post"].contains("port"));
    expect(config["received_json_post"]["port"].is_number_integer());
    expect(config["received_json_post"]["port"] >= 1);
    expect(config["received_json_post"]["port"] <= 65535);

    expect(config["received_json_post"].contains("token"));
    expect(config["received_json_post"]["token"].is_string());
  }

#undef expect

  return config;
}

} // namespace

mock_sdk_t::mock_sdk_t(std::atomic_bool const & sigint,
                       nlohmann::json const & config,
                       parsed_tag)
    : sigint_(sigint),
      pseudonym{unslugify(config["pseudonym"])},
      rmq_host{config["rmq_host"]},
      rmq_port{config["rmq_port"]},
      rmq_username{config["rmq_username"]},
      rmq_password{config["rmq_password"]},
      rmq_heartbeat{config["rmq_heartbeat"]},
      bandwidth_bps_(
          config["bandwidth_bps"]
              .get_ref<nlohmann::json::number_integer_t const &>()),
      latency_ms_(
          config["latency_ms"]
              .get_ref<nlohmann::json::number_integer_t const &>()),
      etcDirectory{config["etcDirectory"]},
      pluginConfigFilePath{config["pluginConfigFilePath"]},
      post_json(std::nothrow),
      has_received_json_post_config{
          config.contains("received_json_post")},
      received_json_post_addr{
          config.contains("received_json_post") ?
              (std::string)config["received_json_post"]["address"] :
              ""},
      received_json_post_port{
          config.contains("received_json_post") ?
              (int)config["received_json_post"]["port"] :
              0},
      received_json_post_token{
          config.contains("received_json_post") ?
              (std::string)config["received_json_post"]["token"] :
              ""} {
}

mock_sdk_t::mock_sdk_t(std::atomic_bool const & sigint,
                       nlohmann::json const & config)
    : mock_sdk_t(sigint, parse_config(config), parsed_tag()) {
}

struct rmq_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

template<class T>
static amqp_bytes_t rmq_to_bytes(T const & x) {
  return {.len = sst::checked_t<size_t>(x.size()).value(),
          .bytes =
              const_cast<void *>(static_cast<void const *>(x.data()))};
}

//----------------------------------------------------------------------
//
// The documentation for amqp_destroy_connection() says that "If the
// connection with the broker is open, it will be implicitly closed with
// a reply code of 200 (success)". This makes it sound like it should be
// sufficient to just call amqp_destroy_connection() to shut down the
// connection. However, in practice, this sometimes causes pending
// outgoing messages to be dropped before they get to the server.
//
// The solution is to call all three of the shutdown functions: first
// amqp_channel_close(), then amqp_connection_close(), and finally
// amqp_destroy_connection(). This ensures that all outgoing messages
// finish sending.
//
// It's unclear exactly which of these functions are okay to call when
// an error occurs in the middle of the sequence of functions that get
// called to set up the connection, so we only call all three shutdown
// functions when the connection was fully set up successfully, i.e.,
// when everything up to and including amqp_channel_open() succeeded.
// Otherwise, we only call amqp_destroy_connection().
//

void mock_sdk_t::rmq_destroy() noexcept {
  if (rmq_connection_ == nullptr) {
    return;
  }
  if (rmq_channel_is_open_) {
    {
      amqp_rpc_reply_t const r = amqp_channel_close(rmq_connection_,
                                                    rmq_channel_,
                                                    AMQP_REPLY_SUCCESS);
      static_cast<void>(r);
    }
    {
      amqp_rpc_reply_t const r =
          amqp_connection_close(rmq_connection_, AMQP_REPLY_SUCCESS);
      static_cast<void>(r);
    }
  }
  {
    int const s = amqp_destroy_connection(rmq_connection_);
    static_cast<void>(s);
  }
  rmq_connection_ = nullptr;
  rmq_socket_ = nullptr;
  rmq_channel_is_open_ = false;
  rmq_is_consuming_ = false;
}

void mock_sdk_t::rmq_connect() {
  if (rmq_connection_ != nullptr) {
    return;
  }
  try {
    rmq_connection_ = amqp_new_connection();
    if (rmq_connection_ == nullptr) {
      throw rmq_error("amqp_new_connection() failed");
    }
    rmq_socket_ = amqp_tcp_socket_new(rmq_connection_);
    if (rmq_socket_ == nullptr) {
      throw rmq_error("amqp_tcp_socket_new() failed");
    }
    {
      int const s =
          amqp_socket_open(rmq_socket_, rmq_host.c_str(), rmq_port);
      if (s != AMQP_STATUS_OK) {
        throw rmq_error("amqp_socket_open() failed");
      }
    }
    {
      amqp_rpc_reply_t const r = amqp_login(rmq_connection_,
                                            "/",
                                            AMQP_DEFAULT_MAX_CHANNELS,
                                            AMQP_DEFAULT_FRAME_SIZE,
                                            rmq_heartbeat,
                                            AMQP_SASL_METHOD_PLAIN,
                                            rmq_username.c_str(),
                                            rmq_password.c_str());
      if (r.reply_type != AMQP_RESPONSE_NORMAL) {
        throw rmq_error("amqp_login() failed");
      }
    }
    {
      amqp_channel_open_ok_t * const ok =
          amqp_channel_open(rmq_connection_, rmq_channel_);
      static_cast<void>(ok);
      amqp_rpc_reply_t const r = amqp_get_rpc_reply(rmq_connection_);
      if (r.reply_type != AMQP_RESPONSE_NORMAL) {
        throw rmq_error("amqp_channel_open() failed");
      }
      rmq_channel_is_open_ = true;
    }
  } catch (...) {
    rmq_destroy();
    throw;
  }
}

void mock_sdk_t::rmq_queue_declare(std::string const & queue) {
  rmq_connect();
  try {
    amqp_queue_declare_ok_t * const ok =
        amqp_queue_declare(rmq_connection_,
                           rmq_channel_,
                           rmq_to_bytes(queue),
                           0, // passive
                           0, // durable
                           0, // exclusive
                           0, // auto-delete
                           amqp_empty_table // arguments
        );
    (void)ok;
    amqp_rpc_reply_t const r = amqp_get_rpc_reply(rmq_connection_);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) {
      throw rmq_error("amqp_queue_declare() failed");
    }
  } catch (...) {
    rmq_destroy();
    throw;
  }
}

void mock_sdk_t::rmq_basic_publish(
    std::string const & queue,
    std::vector<unsigned char> const & body) {
  rmq_queue_declare(queue);
  try {
    int const s = amqp_basic_publish(rmq_connection_,
                                     rmq_channel_,
                                     amqp_empty_bytes, // exchange
                                     rmq_to_bytes(queue),
                                     0, // mandatory
                                     0, // immediate
                                     nullptr, // properties
                                     rmq_to_bytes(body));
    if (s != AMQP_STATUS_OK) {
      throw rmq_error("amqp_basic_publish() failed");
    }
  } catch (...) {
    rmq_destroy();
    throw;
  }
}

void mock_sdk_t::rmq_basic_consume(std::string const & queue) {
  if (rmq_is_consuming_) {
    return;
  }
  rmq_queue_declare(queue);
  try {
    amqp_basic_consume_ok_t * const ok =
        amqp_basic_consume(rmq_connection_,
                           rmq_channel_,
                           rmq_to_bytes(queue),
                           amqp_empty_bytes, // consumer_tag
                           0, // no_local
                           1, // no_ack
                           1, // exclusive
                           amqp_empty_table // arguments
        );
    static_cast<void>(ok);
    amqp_rpc_reply_t const r = amqp_get_rpc_reply(rmq_connection_);
    if (r.reply_type != AMQP_RESPONSE_NORMAL) {
      throw rmq_error("amqp_basic_consume() failed");
    }
  } catch (...) {
    rmq_destroy();
    throw;
  }
  rmq_is_consuming_ = true;
}

bool mock_sdk_t::rmq_consume_message(std::string const & queue,
                                     amqp_envelope_t & envelope) {
  rmq_basic_consume(queue);
  try {
    amqp_maybe_release_buffers(rmq_connection_);
    auto const usec =
        std::chrono::duration_cast<std::chrono::microseconds>(
            work_loop_sleep)
            .count();
    timeval timeout{};
    timeout.tv_sec =
        sst::checked_t<decltype(timeout.tv_sec)>(usec / 1000000).value();
    timeout.tv_usec =
        static_cast<decltype(timeout.tv_usec)>(usec % 1000000);
    amqp_rpc_reply_t const r = amqp_consume_message(rmq_connection_,
                                                    &envelope,
                                                    &timeout,
                                                    0 // flags
    );
    if (r.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION
        && r.library_error == AMQP_STATUS_TIMEOUT) {
      return false;
    }
    if (r.reply_type != AMQP_RESPONSE_NORMAL) {
      throw rmq_error("amqp_consume_message() failed");
    }
    return true;
  } catch (...) {
    rmq_destroy();
    throw;
  }
}

//----------------------------------------------------------------------

mock_sdk_t::~mock_sdk_t() {
  rmq_destroy();
}

void mock_sdk_t::sdk_log(std::string const & msg) {
  RaceLog::logDebug("mock_sdk_t", msg, "(unknown stack trace)");
}

RawData mock_sdk_t::getEntropy(std::uint32_t const numBytes) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  RawData entropy;
  if (numBytes > 0) {
    entropy.reserve(sst::checked_cast<RawData::size_type>(numBytes));
    using T1 = RawData::value_type;
    using T2 = sst::promote<T1>::type;
    SST_STATIC_ASSERT((std::is_same<T2, int>::value
                       || std::is_same<T2, unsigned int>::value));
    std::uniform_int_distribution<T2> d(sst::type_min<T1>::value,
                                        sst::type_max<T1>::value);
    std::random_device r;
    for (decltype(+numBytes) i = 0; i < numBytes; ++i) {
      entropy.push_back(static_cast<T1>(d(r)));
    }
  }
  return entropy;
}

std::string mock_sdk_t::getActivePersona() {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  return pseudonym;
}

SdkResponse mock_sdk_t::asyncError(RaceHandle, PluginResponse) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  throw std::runtime_error("asyncError() not supported yet");
}

SdkResponse mock_sdk_t::makeDir(std::string const & path) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  sst::mkdir_p(pluginConfigFilePath + "/" + normalize_path(path));
  SdkResponse r{};
  r.status = SDK_OK;
  return r;
}

SdkResponse mock_sdk_t::removeDir(std::string const & path) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  sst::rm_f_r(pluginConfigFilePath + "/" + normalize_path(path));
  SdkResponse r{};
  r.status = SDK_OK;
  return r;
}

std::vector<std::string>
mock_sdk_t::listDir(std::string const & directoryPath) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  std::vector<std::string> list;
  auto it = sst::dir_it(pluginConfigFilePath + "/"
                                    + normalize_path(directoryPath));
  auto const end = it.end();
  for (; !(it == end); ++it) {
    sst::path & path = *it;
    list.emplace_back(path.str().substr(it.prefix()));
  }
  return list;
}

std::vector<std::uint8_t>
mock_sdk_t::readFile(std::string const & filepath) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  std::string const x =
      pluginConfigFilePath + "/" + normalize_path(filepath);
  if (sst::test_f(x)) {
    return sst::read_whole_file<std::vector<std::uint8_t>>(x);
  } else {
    return {};
  }
}

SdkResponse mock_sdk_t::appendFile(std::string const &,
                                   std::vector<std::uint8_t> const &) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  throw std::runtime_error("appendFile() not supported yet");
}

SdkResponse
mock_sdk_t::writeFile(std::string const & filepath,
                      std::vector<std::uint8_t> const & data) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  if (filepath.find("..") != filepath.npos) {
    throw std::runtime_error(
        "Path must not contain \"..\" as a substring: "
        + sst::c_quote(filepath) + ".");
  }
  sst::write_whole_file(data, pluginConfigFilePath + "/" + filepath);
  SdkResponse r{};
  r.status = SDK_OK;
  return r;
}

SdkResponse mock_sdk_t::onPluginStatusChanged(PluginStatus) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  SdkResponse r{};
  r.status = SDK_OK;
  return r;
}

static std::string pseudonym_to_queue(std::string const & pseudonym) {
  std::string queue;
  for (auto const & x : pseudonym) {
    unsigned char const y = static_cast<unsigned char>(x);
    queue += "0123456789ABCDEF"[(y >> 4) & 0xF];
    queue += "0123456789ABCDEF"[(y >> 0) & 0xF];
  }
  return queue;
}

//----------------------------------------------------------------------
// Links
//----------------------------------------------------------------------

LinkID
mock_sdk_t::always_create_link(std::set<std::string> const & personas) {
  std::string link_id = sst::to_string(race_handle_++) + ":";
  for (auto const & persona : personas) {
    for (auto const c : persona) {
      if (c == '%') {
        link_id += "%%";
      } else if (c == ',') {
        link_id += "%2C";
      } else {
        link_id += c;
      }
    }
    link_id += ',';
  }
  link_info_t link_info;
  link_info.personas = personas;
  link_info.link_properties.linkType = LT_SEND;
  link_info.link_properties.transmissionType = TT_UNICAST;
  link_info.link_properties.connectionType = CT_INDIRECT;
  link_info.link_properties.channelGid = channel_name_;
  link_info.link_properties.linkAddress = "";
  link_infos_.emplace(link_id, link_info);
  links_for_[personas].insert(link_id);
  return link_id;
}

//----------------------------------------------------------------------
// sendEncryptedPackage
//----------------------------------------------------------------------

SdkResponse
mock_sdk_t::sendEncryptedPackage(EncPkg const ePkg,
                                 ConnectionID const connectionId,
                                 uint64_t,
                                 int32_t) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  SdkResponse r{};
  r.status = SDK_OK;
  r.queueUtilization = 0;
  r.handle = race_handle_++;
  auto const & link_id = connections_.at(connectionId);
  auto const & link_info = link_infos_.at(link_id);
  auto blob = ePkg.getRawData();
  blob.insert(blob.begin(), 1, static_cast<unsigned char>(0));
  for (auto const & persona : link_info.personas) {
    std::string const rmq_queue = pseudonym_to_queue(persona);
    send_queues_[rmq_queue].entries.push_back({blob, r.handle});
  }
  return r;
}

//----------------------------------------------------------------------

void mock_sdk_t::postReceivedJson(ClrMsg const & msg) noexcept {
  /* Post a received message as a json.
  Uses received_json_post_X from the config.
  noexcept to ensure no issues are caused.

  Example json format:
  {"from": "Armadillo0", "to": "Armadillo1", "msg": "Foo", "token": "asdf"}
  */
  try {
    const nlohmann::json json_data =
        nlohmann::json({{"from", msg.getFrom()},
                        {"to", msg.getTo()},
                        {"msg", msg.getMsg()},
                        {"token", received_json_post_token}});
    const std::string url = "http://" + received_json_post_addr + ":"
                            + std::to_string(received_json_post_port);

    post_json(url, json_data);

  } catch (std::exception const & e) {
    // Failed for some other reason (e.g. string allocate).
    sdk_log(std::string{"postReceivedJson failed: "} + e.what());
  } catch (...) {
    sdk_log("postReceivedJson failed: unknown exception");
  }
}

SdkResponse mock_sdk_t::presentCleartextMessage(ClrMsg const msg) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  if (has_received_json_post_config) {
    postReceivedJson(msg);
  }
  std::cout << msg.getFrom() << ": " << msg.getMsg() << std::endl;
  SdkResponse r;
  r.status = SDK_OK;
  r.queueUtilization = 0;
  r.handle = NULL_RACE_HANDLE;
  return r;
}

SdkResponse mock_sdk_t::openConnection(LinkType const linkType,
                                       LinkID const linkId,
                                       std::string,
                                       int32_t,
                                       int32_t,
                                       int32_t const timeout) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);

  switch (linkType) {
    case LT_SEND:
    case LT_RECV:
    case LT_BIDI:
      break;
    default:
      throw std::runtime_error("invalid linkType");
  }

  if (timeout != 0) {
    throw std::runtime_error(
        "non-asynchronous calls are not supported");
  }

  auto const it = link_infos_.find(linkId);
  if (it == link_infos_.end()) {
    throw std::runtime_error("unknown linkId");
  }
  link_info_t & link_info = it->second;

  SdkResponse r;
  r.status = SDK_OK;
  r.queueUtilization = 0;
  r.handle = race_handle_++;

  ConnectionID const connection_id =
      sst::to_string(link_info.connection_count++) + ":" + linkId;

  connections_.emplace(connection_id, linkId);
  opening_connections_.emplace(connection_id, r.handle);

  return r;
}

SdkResponse mock_sdk_t::closeConnection(ConnectionID const connectionId,
                                        int32_t const timeout) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);

  if (timeout != 0) {
    throw std::runtime_error(
        "non-asynchronous calls are not supported");
  }

  auto const it = open_connections_.find(connectionId);
  if (it == open_connections_.end()) {
    throw std::runtime_error("unknown connectionId");
  }

  SdkResponse r;
  r.status = SDK_OK;
  r.queueUtilization = 0;
  r.handle = race_handle_++;

  open_connections_.erase(it);
  closing_connections_.emplace(connectionId, r.handle);

  return r;
}

std::vector<LinkID> mock_sdk_t::getLinksForPersonas(
    std::vector<std::string> const recipientPersonas,
    LinkType const linkType) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);

  switch (linkType) {
    case LT_SEND:
    case LT_RECV:
    case LT_BIDI:
      break;
    default:
      throw std::runtime_error("invalid linkType");
  }

  if (recipientPersonas.empty()) {
    return {};
  }

  if (linkType != LT_SEND) {
    return {};
  }

  std::set<std::string> personas;
  personas.insert(recipientPersonas.begin(), recipientPersonas.end());

  std::vector<LinkID> links;
  if (links_for_[personas].empty()) {
    links.push_back(always_create_link(personas));
  } else {
    links.insert(links.end(),
                 links_for_[personas].begin(),
                 links_for_[personas].end());
  }
  return links;
}

LinkID
mock_sdk_t::getLinkForConnection(ConnectionID const connectionId) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);

  if (open_connections_.find(connectionId) == open_connections_.end()
      && closing_connections_.find(connectionId)
             == closing_connections_.end()) {
    throw std::runtime_error("unknown connectionId");
  }

  return connections_[connectionId];
}

LinkProperties mock_sdk_t::getLinkProperties(LinkID const linkId) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);

  auto const it = link_infos_.find(linkId);
  if (it == link_infos_.end()) {
    throw std::runtime_error("unknown linkId");
  }
  link_info_t & link_info = it->second;

  return link_info.link_properties;
}

std::map<std::string, ChannelProperties>
mock_sdk_t::getSupportedChannels() {
  std::map<std::string, ChannelProperties> m;
  auto xs = getAllChannelProperties();
  for (auto & x : xs) {
    m.emplace(x.channelGid, std::move(x));
  }
  return m;
}

//----------------------------------------------------------------------
// getChannelProperties
//----------------------------------------------------------------------

ChannelProperties mock_sdk_t::getChannelProperties(std::string) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// getAllChannelProperties
//----------------------------------------------------------------------

std::vector<ChannelProperties> mock_sdk_t::getAllChannelProperties() {
  std::vector<ChannelProperties> v(1);
  v[0].linkDirection = LD_CREATOR_TO_LOADER;
  v[0].transmissionType = TT_UNICAST;
  v[0].connectionType = CT_INDIRECT;
  v[0].channelGid = channel_name_;
  v[0].roles.emplace_back();
  return v;
}

//----------------------------------------------------------------------
// deactivateChannel
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::deactivateChannel(std::string, std::int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// activateChannel
//----------------------------------------------------------------------

SdkResponse
mock_sdk_t::activateChannel(std::string, std::string, std::int32_t) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  SdkResponse r;
  r.status = SDK_OK;
  r.queueUtilization = 0;
  r.handle = NULL_RACE_HANDLE;
  return r;
}

//----------------------------------------------------------------------
// destroyLink
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::destroyLink(LinkID, std::int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// createLink
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::createLink(std::string channelGid,
                                   std::vector<std::string> personas,
                                   std::int32_t timeout) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  if (channelGid != channel_name_) {
    throw std::runtime_error("unknown channel name: " + channelGid);
  }
  if (timeout != 0) {
    throw std::runtime_error(
        "non-asynchronous calls are not supported");
  }
  SdkResponse r{};
  r.status = SDK_OK;
  r.handle = race_handle_++;
  creating_links_.emplace(
      r.handle,
      always_create_link(
          std::set<std::string>(personas.cbegin(), personas.cend())));
  return r;
}

//----------------------------------------------------------------------
// loadLinkAddress
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::loadLinkAddress(std::string,
                                        std::string,
                                        std::vector<std::string>,
                                        std::int32_t) {
  SdkResponse r{};
  r.status = SDK_OK;
  r.handle = race_handle_++;
  return r;
}

//----------------------------------------------------------------------
// loadLinkAddresses
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::loadLinkAddresses(std::string,
                                          std::vector<std::string>,
                                          std::vector<std::string>,
                                          std::int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// createLinkFromAddress
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::createLinkFromAddress(std::string,
                                              std::string,
                                              std::vector<std::string>,
                                              std::int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// bootstrapDevice
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::bootstrapDevice(RaceHandle,
                                        std::vector<std::string>) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// bootstrapFailed
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::bootstrapFailed(RaceHandle) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// setPersonasForLink
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::setPersonasForLink(std::string,
                                           std::vector<std::string>) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// getPersonasForLink
//----------------------------------------------------------------------

std::vector<std::string> mock_sdk_t::getPersonasForLink(std::string) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// onMessageStatusChanged
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::onMessageStatusChanged(RaceHandle,
                                               MessageStatus) {
  std::lock_guard<decltype(mutex_)> const lock(mutex_);
  SdkResponse r{};
  r.status = SDK_OK;
  return r;
}

//----------------------------------------------------------------------
// sendBootstrapPkg
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::sendBootstrapPkg(ConnectionID,
                                         std::string,
                                         RawData,
                                         int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// requestPluginUserInput
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::requestPluginUserInput(std::string const &,
                                               std::string const &,
                                               bool) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// requestCommonUserInput
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::requestCommonUserInput(std::string const &) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// flushChannel
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::flushChannel(std::string, uint64_t, int32_t) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// getLinksForChannel
//----------------------------------------------------------------------

std::vector<LinkID> mock_sdk_t::getLinksForChannel(std::string) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------
// displayInfoToUser
//----------------------------------------------------------------------

SdkResponse mock_sdk_t::displayInfoToUser(std::string const &,
                                          RaceEnums::UserDisplayType) {
  SST_ASSERT((false));
  SST_THROW_UNIMPLEMENTED();
}

//----------------------------------------------------------------------

struct malformed_envelope : std::runtime_error {
  explicit malformed_envelope(std::string const & what)
      : std::runtime_error{what} {
  }
};

template<class T>
static std::string bytes_to_string(unsigned char const *& p, T & n) {
  if (n < 4) {
    throw malformed_envelope{"insufficient length"};
  }
  unsigned long const k = static_cast<unsigned long>(p[0]) << 24
                          | static_cast<unsigned long>(p[1]) << 16
                          | static_cast<unsigned long>(p[2]) << 8
                          | static_cast<unsigned long>(p[3]) << 0;
  p += 4;
  n -= 4;
  if (sst::unsigned_lt(n, k)) {
    throw malformed_envelope{"insufficient length"};
  }
  p += k;
  n -= k;
  return std::string{reinterpret_cast<char const *>(p - k),
                     reinterpret_cast<char const *>(p)};
}

void mock_sdk_t::run(IRacePluginNM & plugin) {
  std::string const queue = pseudonym_to_queue(pseudonym);

  {
    PluginResponse r;
    try {
      PluginConfig x;
      x.etcDirectory = etcDirectory;
      r = plugin.init(x);
    } catch (...) {
      throw std::runtime_error("init() should never throw");
    }
    handle_plugin_response(r);
    if (r != PLUGIN_OK) {
      throw std::runtime_error("init() failed");
    }
  }

  try {
    auto gate{std::chrono::steady_clock::now()};
    while (!sigint_) {
      if (std::chrono::steady_clock::now() < gate) {
        std::this_thread::sleep_for(work_loop_sleep);
        continue;
      }

      for (auto it = opening_connections_.cbegin();
           it != opening_connections_.cend();) {
        ConnectionID const & connection_id = it->first;
        RaceHandle const & handle = it->second;
        LinkID const & link_id = connections_[connection_id];
        PluginResponse r;
        try {
          r = plugin.onConnectionStatusChanged(
              handle,
              connection_id,
              CONNECTION_OPEN,
              link_id,
              link_infos_[link_id].link_properties);
        } catch (...) {
          throw std::runtime_error(
              "onConnectionStatusChanged() should never throw");
        }
        if (handle_plugin_response(r)) {
          it = opening_connections_.erase(it);
          open_connections_.emplace(connection_id);
        } else {
          ++it;
        }
      }

      for (auto it = closing_connections_.cbegin();
           it != closing_connections_.cend();) {
        ConnectionID const & connection_id = it->first;
        RaceHandle const & handle = it->second;
        auto const it2 = connections_.find(connection_id);
        LinkID const & link_id = it2->second;
        PluginResponse r;
        try {
          r = plugin.onConnectionStatusChanged(
              handle,
              connection_id,
              CONNECTION_CLOSED,
              link_id,
              link_infos_[link_id].link_properties);
        } catch (...) {
          throw std::runtime_error(
              "onConnectionStatusChanged() should never throw");
        }
        if (handle_plugin_response(r)) {
          it = closing_connections_.erase(it);
          connections_.erase(it2);
        } else {
          ++it;
        }
      }

      for (auto it = creating_links_.cbegin();
           it != creating_links_.cend();) {
        RaceHandle const & handle = it->first;
        LinkID const & link_id = it->second;
        PluginResponse r;
        try {
          r = plugin.onLinkStatusChanged(
              handle,
              link_id,
              LINK_CREATED,
              link_infos_[link_id].link_properties);
        } catch (...) {
          throw std::runtime_error(
              "onLinkStatusChanged() should never throw");
        }
        if (handle_plugin_response(r)) {
          it = creating_links_.erase(it);
        } else {
          ++it;
        }
      }

      // TODO: We could clean up orphaned connections_ entries once in a
      // while (i.e., those that are not inside opening_connections_,
      // open_connections_, nor closing_connections_).

      {
        auto const t = sst::mono_time_ms();
        for (auto & kv : send_queues_) {
          auto const & rmq_queue = kv.first;
          auto & send_queue = kv.second;
          while (!send_queue.entries.empty()) {
            auto const & entry = send_queue.entries.front();
            if (send_queue.idle) {
              send_queue.wakeup =
                  t
                  + (sst::checked(entry.blob.size()) * 1000)
                        / bandwidth_bps_
                  + latency_ms_;
              send_queue.idle = false;
            }
            if (t < send_queue.wakeup) {
              break;
            }
            rmq_basic_publish(rmq_queue, entry.blob);
            PluginResponse r;
            try {
              r = plugin.onPackageStatusChanged(entry.handle,
                                                PACKAGE_SENT);
            } catch (...) {
              throw std::runtime_error(
                  "onPackageStatusChanged() should never throw");
            }
            handle_plugin_response(r);
            send_queue.entries.pop_front();
            send_queue.idle = true;
          }
        }
      }

      amqp_envelope_t envelope;
      try {
        if (!rmq_consume_message(queue, envelope)) {
          continue;
        }
      } catch (rmq_error const & e) {
        sdk_log(e.what());
        gate = std::chrono::steady_clock::now() + rmq_error_sleep;
        continue;
      }
      try {
        unsigned char const * const bytes =
            static_cast<unsigned char const *>(
                envelope.message.body.bytes);
        auto const len = envelope.message.body.len;
        if (len == 0) {
          sdk_log("empty envelope");
        } else if (bytes[0] == 0) {
          PluginResponse r;
          try {
            r = plugin.processEncPkg(
                NULL_RACE_HANDLE,
                EncPkg(RawData(
                    reinterpret_cast<RawData::value_type const *>(bytes
                                                                  + 1),
                    reinterpret_cast<RawData::value_type const *>(
                        bytes + len))),
                {});
          } catch (...) {
            throw std::runtime_error(
                "processEncPkg() should never throw");
          }
          handle_plugin_response(r);
        } else {
          unsigned char const * p = bytes + 1;
          auto n = len - 1;
          std::string const to = bytes_to_string(p, n);
          std::string const msg = bytes_to_string(p, n);
          if (n != 0) {
            throw malformed_envelope{"excess length"};
          }
          PluginResponse r;
          try {
            r = plugin.processClrMsg(
                NULL_RACE_HANDLE,
                ClrMsg(msg, pseudonym, to, 0, 0, 0, 0));
          } catch (...) {
            throw std::runtime_error(
                "processClrMsg() should never throw");
          }
          handle_plugin_response(r);
        }
      } catch (malformed_envelope const & e) {
        sdk_log(std::string{"malformed envelope: "} + e.what());
      } catch (...) {
        amqp_destroy_envelope(&envelope);
        throw;
      }
      amqp_destroy_envelope(&envelope);
    }
  } catch (...) {
    try {
      plugin.shutdown();
    } catch (...) {
    }
    throw;
  }

  {
    PluginResponse r;
    try {
      r = plugin.shutdown();
    } catch (...) {
      throw std::runtime_error("shutdown() should never throw");
    }
    handle_plugin_response(r);
  }
}

void mock_sdk_t::whisper(std::string const & from,
                         std::string const & to,
                         std::string const & msg) {
  std::string const queue = pseudonym_to_queue(from);
  std::vector<unsigned char> body;
  body.push_back(1);
  string_to_bytes(body, to);
  string_to_bytes(body, msg);
  auto gate{std::chrono::steady_clock::now()};
  while (!sigint_) {
    if (std::chrono::steady_clock::now() < gate) {
      std::this_thread::sleep_for(work_loop_sleep);
      continue;
    }
    try {
      rmq_basic_publish(queue, body);
      break;
    } catch (rmq_error const & e) {
      sdk_log(e.what());
      gate = std::chrono::steady_clock::now() + rmq_error_sleep;
    }
  }
}

} // namespace kestrel
