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
#include <kestrel/kestrel_stack_create.hpp>
// Include twice to test idempotence.
#include <kestrel/kestrel_stack_create.hpp>
//

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <sst/catalog/SST_ASSERT.hpp>
#include <sst/catalog/c_quote.hpp>
#include <sst/catalog/crypto_rng.hpp>
#include <sst/catalog/dir_it.hpp>
#include <sst/catalog/is_positive.hpp>
#include <sst/catalog/json/get_as_file.hpp>
#include <sst/catalog/json/set_from_file.hpp>
#include <sst/catalog/mkdir_p.hpp>
#include <sst/catalog/mkdir_p_new.hpp>
#include <sst/catalog/opt_arg.hpp>
#include <sst/catalog/optional.hpp>
#include <sst/catalog/parse_opt.hpp>
#include <sst/catalog/rm_f_r.hpp>
#include <sst/catalog/test_f.hpp>
#include <sst/catalog/to_string.hpp>
#include <sst/catalog/unknown_oper.hpp>
#include <sst/catalog/unknown_opt.hpp>

#include <kestrel/cluster_provider_t.hpp>
#include <kestrel/cluster_spec_t.hpp>
#include <kestrel/config.h>
#include <kestrel/json_t.hpp>
#include <kestrel/kestrel_cli_args_t.hpp>
#include <kestrel/kestrel_genesis.hpp>
#include <kestrel/slugify.hpp>
#include <kestrel/stack_spec_t.hpp>

namespace kestrel {

namespace {

void help(std::string const & argv0, std::ostream & cout) {
  cout << "Usage: " << argv0 << " ...";
  cout << R"(
)";
}

//----------------------------------------------------------------------
// sh_escape
//----------------------------------------------------------------------

std::string sh_escape(std::string const & x) {
  return std::regex_replace(x, std::regex("'"), "'\\\\''");
}

//----------------------------------------------------------------------
// tf_sh_escape
//----------------------------------------------------------------------
//
// Given a Terraform interpolation string, returns a new interpolation
// string that escapes the value to be suitable to insert into a
// single-quoted shell string.
//
// For example,
//
//       ${foo}    ->    ${replace(foo, "'", "'\\''")}
//

std::string tf_sh_escape(std::string const & x) {
  std::regex const r("^\\$\\{(.*)\\}$");
  SST_ASSERT((std::regex_match(x, r)));
  return std::regex_replace(x,
                            r,
                            "$${replace($1, \"'\", \"'\\\\''\")}");
}

//----------------------------------------------------------------------

struct stack_setup_t {

  json_t & data;
  json_t & provider;
  json_t & resource;
  json_t & terraform;

  json_t & local_file;
  json_t & local_sensitive_file;
  json_t & required_providers;
  json_t & terraform_data;
  json_t & tls_private_key;

  bool have_replay_file;
  json_t * dawn_depends_on;
  std::string random_name;
  bool create_registry;
};

stack_setup_t do_stack_setup(json_t & stack_tf,
                             bool const have_replay_file,
                             json_t & registry,
                             std::string const & registry_image_url,
                             std::string const & registry_username,
                             std::string const & registry_password) {

  json_t & data = stack_tf["data"];
  json_t & provider = stack_tf["provider"];
  json_t & resource = stack_tf["resource"];
  json_t & terraform = stack_tf["terraform"];

  json_t & local_file = resource["local_file"];
  json_t & local_sensitive_file = resource["local_sensitive_file"];
  json_t & random_string = resource["random_string"];
  json_t & required_providers = terraform["required_providers"];
  json_t & required_version = terraform["required_version"];
  json_t & terraform_data = resource["terraform_data"];
  json_t & tls_private_key = resource["tls_private_key"];

  json_t * const dawn =
      have_replay_file ? &local_file["dawn"] : nullptr;

  std::string const & random_name =
      "kestrel${random_string.random_name.result}";

  if (required_version.is_null()) {

    required_version = ">= 1.2.0";

    if (have_replay_file) {
      *dawn = {
          {"content", "${timestamp()}\n"},
          {"depends_on", json_t::array()},
          {"filename", "dawn.txt"},
      };
    }

    random_string["random_name"] = {
        {"length", 10},
        {"lower", false},
        {"numeric", true},
        {"special", false},
        {"upper", false},
    };

  } //

  bool const create_registry = registry.is_null();
  if (create_registry) {
    std::string command;
    command += "registry_image_url='" + tf_sh_escape(registry_image_url)
               + "'\n";
    command +=
        "(docker login --username '" + tf_sh_escape(registry_username)
        + "' --password-stdin <<EOF -- \"$${registry_image_url?}\"";
    command += " && docker tag -- kestrel \"$${registry_image_url?}\"";
    command += " && docker push -- \"$${registry_image_url?}\"";
    command += ")\n$${registry_password?}\nEOF\n";
    command += "s=$?\n";
    command += "docker logout \"$${registry_image_url?}\"\n";
    command += "docker rmi -f \"$${registry_image_url?}\"\n";
    command += "exit $${s?}\n";
    registry = {
        {"name", random_name},
        {"provisioner",
         {
             {
                 {"local-exec",
                  {
                      {"command", command},
                      {"environment",
                       {
                           {"registry_password", registry_password},
                       }},
                  }},
             },
         }},
    };
  }

  return {
      data,
      provider,
      resource,
      terraform,
      local_file,
      local_sensitive_file,
      required_providers,
      terraform_data,
      tls_private_key,
      have_replay_file,
      have_replay_file ? &(*dawn)["depends_on"] : nullptr,
      random_name,
      create_registry,
  };

} //

//----------------------------------------------------------------------

struct host_setup_t {
  std::string ssh_pk;
  std::string ssh_sk;
  json_t * provisioner0;
  json_t * provisioner1;
};

host_setup_t do_host_setup(stack_setup_t & stack_setup,
                           std::string const & provision_script,
                           std::string const & cluster_i_str,
                           std::string const & registry_image_url,
                           std::string const & registry_username,
                           std::string const & registry_password,
                           std::string const & host_id,
                           std::string const & provisioner0_ref,
                           std::string const & host_dir_rel,
                           std::string const & public_ip,
                           json_t & provisioner0,
                           json_t & provisioner1) {

  std::string const & ssh_pk =
      "${tls_private_key." + host_id + ".public_key_openssh}";

  std::string const & ssh_sk =
      "${tls_private_key." + host_id + ".private_key_openssh}";

  stack_setup.tls_private_key[host_id] = {
      {"algorithm", "RSA"},
      {"rsa_bits", 4096},
  };

  stack_setup.local_file[host_id + "_id_rsa_pub"] = {
      {"content", ssh_pk},
      {"file_permission", "0666"},
      {"filename", host_dir_rel + "/id_rsa.pub"},
  };

  stack_setup.local_sensitive_file[host_id + "_id_rsa"] = {
      {"content", ssh_sk},
      {"file_permission", "0600"},
      {"filename", host_dir_rel + "/id_rsa"},
  };

  stack_setup.local_file[host_id + "_env"] = {
      {"content",
       "KESTREL_DOCKER_IMAGE='" + tf_sh_escape(registry_image_url)
           + "'\n"},
      {"file_permission", "0666"},
      {"filename", host_dir_rel + "/.env"},
  };

  provisioner0.merge_patch({
      {"connection",
       {
           {"agent", false},
           {"host", public_ip},
           {"private_key", ssh_sk},
           {"type", "ssh"},
           {"user", "ubuntu"},
       }},
      {"depends_on",
       {
           "local_file." + host_id + "_env",
           "local_file." + host_id + "_id_rsa_pub",
           "local_sensitive_file." + host_id + "_id_rsa",
       }},
      {"provisioner",
       {
           {
               {"file",
                {
                    {"destination", "/home/ubuntu/provision.bash"},
                    {"source", provision_script},
                }},
           },
           {
               {"remote-exec",
                {
                    {"inline",
                     {"cluster_i='" + sh_escape(cluster_i_str)
                      + "' registry_password='"
                      + tf_sh_escape(registry_password)
                      + "' registry_image_url='"
                      + tf_sh_escape(registry_image_url)
                      + "' registry_username='"
                      + tf_sh_escape(registry_username)
                      + "' bash "
                        "/home/ubuntu/provision.bash"}},
                }},
           },
       }},
  });

  stack_setup.local_file[host_id + "_ip_txt"] = {
      {"content", public_ip + "\n"},
      {"file_permission", "0666"},
      {"filename", host_dir_rel + "/ip.txt"},
  };

  if (stack_setup.have_replay_file) {
    stack_setup.dawn_depends_on->push_back(provisioner0_ref);
    provisioner1.merge_patch({
        {"connection",
         {
             {"agent", false},
             {"host", public_ip},
             {"private_key", ssh_sk},
             {"type", "ssh"},
             {"user", "ubuntu"},
         }},
        {"provisioner",
         {
             {{"remote-exec",
               {
                   {"inline",
                    {"cd /home/ubuntu"
                     " && d=`date -d "
                     "${trimsuffix(local_file.dawn.content, \"\\n\")} "
                     "+%s`000000000"
                     " && jq .dawn=$d replay.json "
                     ">replay.json.tmp"
                     " && mv -f replay.json.tmp "
                     "replay.json"}},
               }}},
         }},
    });
  }

  return {
      ssh_pk,
      ssh_sk,
      &provisioner0,
      &provisioner1,
  };

} //

//----------------------------------------------------------------------

} // namespace

void kestrel_stack_create(kestrel_cli_args_t const & kestrel_cli_args,
                          std::list<std::string> argv,
                          std::ostream & cout) {

  if (argv.empty()) {
    argv.push_back("kestrel stack create");
  }

  std::string const argv0 = std::move(argv.front());

  std::string const & work_dir = kestrel_cli_args.work_dir();
  sst::optional<std::string> stack_spec_file;

  bool parse_options = true;
  while (argv.pop_front(), !argv.empty()) {
    if (parse_options) {

      //----------------------------------------------------------------
      // Options terminator
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--", sst::opt_arg::forbidden)) {
        parse_options = false;
        continue;
      }

      //----------------------------------------------------------------
      // --help
      //----------------------------------------------------------------

      if (sst::parse_opt(argv, "--help", sst::opt_arg::forbidden)) {
        help(argv0, cout);
        return;
      }

      //----------------------------------------------------------------
      // Unknown options
      //----------------------------------------------------------------

      sst::unknown_opt(argv);

      //----------------------------------------------------------------
    }

    //------------------------------------------------------------------
    // Operands
    //------------------------------------------------------------------

    if (!stack_spec_file) {
      stack_spec_file.emplace(std::move(argv.front()));
    } else {
      sst::unknown_oper(argv);
    }

    //------------------------------------------------------------------
  }

  stack_spec_t stack_spec;
  if (stack_spec_file) {
    // TODO: Use sst::json::set_from_file_or_inline.
    sst::json::set_from_file<json_t>(*stack_spec_file, stack_spec);
  }

  if (stack_spec.genesis()) {
    kestrel_genesis(kestrel_cli_args, *stack_spec.genesis(), cout);
  }

  std::vector<cluster_spec_t> const & clusters = stack_spec.clusters();
  using cluster_index_t = decltype(clusters.size());

  struct host_info_t {
    cluster_index_t cluster_i;
    std::string dir;
    std::string dir_rel;
    std::vector<std::string> nodes;
    json_t * provisioner0;
    json_t * provisioner1;
    json_t * machine;
  };
  std::vector<host_info_t> hosts;
  using host_index_t = decltype(hosts.size());

  std::vector<std::string> all_nodes;
  for (auto it = sst::dir_it(work_dir + "/nodes"); it != it.end();
       ++it) {
    if (it->test_d()) {
      all_nodes.emplace_back(unslugify(it->c_str() + it.prefix()));
    }
  }
  std::sort(all_nodes.begin(), all_nodes.end());

  std::string const clusters_dir = work_dir + "/clusters";
  std::string const clusters_dir_rel = "./clusters";
  sst::rm_f_r(clusters_dir);
  sst::mkdir_p_new(clusters_dir);

  std::string const nodes_dir = work_dir + "/nodes";
  std::string const nodes_dir_rel = "./nodes";

  std::string const replay_file = work_dir + "/replay.json";
  std::string const replay_file_rel = "./replay.json";
  bool const have_replay_file = sst::test_f(replay_file);

  json_t stack_tf;

  struct cluster_info_t {
    std::vector<host_index_t> hosts;
  };
  std::vector<cluster_info_t> cluster_infos;

  std::string const provision_script =
      (std::getenv("USE_BUILD_TREE") ? std::string(KESTREL_ABS_BUILDDIR)
                                           + "/src/core/share/kestrel" :
                                       std::string(KESTREL_PKGDATADIR))
      + "/provision.bash";

  double total_host_capacity = 0.0;

  for (cluster_index_t cluster_i = static_cast<cluster_index_t>(0);
       cluster_i < clusters.size();
       ++cluster_i) {
    cluster_spec_t const & cluster = clusters[cluster_i];
    std::string const cluster_i_str = sst::to_string(cluster_i);

    {
      std::vector<std::string> const & a = all_nodes;
      std::vector<std::string> const & b = cluster.pin();
      auto p = a.begin();
      auto q = b.begin();
      while (q != b.end()) {
        bool bad = true;
        if (p != a.end()) {
          int const c = p->compare(*q);
          if (c <= 0) {
            bad = false;
            if (c == 0) {
              ++q;
            }
          }
        }
        if (bad) {
          throw std::runtime_error(
              "Pinned node does not exist in genesis: "
              + sst::c_quote(*q));
        }
        ++p;
      }
    }

    std::string const cluster_dir =
        clusters_dir + "/" + sst::to_string(cluster_i);
    std::string const cluster_dir_rel =
        clusters_dir_rel + "/" + sst::to_string(cluster_i);
    std::string const cluster_name =
        "cluster" + sst::to_string(cluster_i);
    sst::mkdir_p_new(cluster_dir);
    std::string const hosts_dir = cluster_dir + "/hosts";
    sst::mkdir_p_new(hosts_dir);

    cluster_infos.emplace_back();
    cluster_info_t & cluster_info = cluster_infos.back();

    switch (cluster.provider()) {

        //--------------------------------------------------------------

      case cluster_provider_t::aws(): {

        json_t & registry =
            stack_tf["resource"]["aws_ecr_repository"]["unique"];

        std::string const registry_image_url =
            "${aws_ecr_repository.unique.repository_url}";

        std::string const registry_username =
            "${data.aws_ecr_authorization_token.unique.user_name}";

        std::string const registry_password =
            "${nonsensitive(data.aws_ecr_authorization_token.unique."
            "password)}";

        stack_setup_t stack_setup = do_stack_setup(stack_tf,
                                                   have_replay_file,
                                                   registry,
                                                   registry_image_url,
                                                   registry_username,
                                                   registry_password);

        {
          json_t & j = stack_setup.required_providers["aws"];
          if (j.is_null()) {
            j = {
                {"source", "hashicorp/aws"},
                {"version", "~> 4.0"},
            };
          }
        }

        {
          json_t & j =
              stack_setup.data["aws_ecr_authorization_token"]["unique"];
          if (j.is_null()) {
            j = json_t::object();
          }
        }

        if (stack_setup.create_registry) {
          registry["force_delete"] = true;
          registry["image_tag_mutability"] = "IMMUTABLE";
          stack_setup.create_registry = false;
        }

        for (decltype(cluster.regions().size()) region_i = 0;
             region_i < cluster.regions().size();
             ++region_i) {

          std::string const & region = cluster.regions()[region_i];
          std::string const provider_alias =
              cluster_name + "_provider" + sst::to_string(region_i);
          std::string const provider_ref = "aws." + provider_alias;

          {
            json_t p = json_t::object();
            p["region"] = region;
            if (cluster.profile()) {
              p["profile"] = *cluster.profile();
            }
            json_t & ps = stack_setup.provider["aws"];
            if (ps.is_null()) {
              ps.push_back(p);
            }
            p["alias"] = provider_alias;
            ps.emplace_back(std::move(p));
          }

          {
            json_t & j =
                stack_tf["resource"]["aws_security_group"][region];
            if (j.is_null()) {
              j = {
                  {"egress",
                   {{
                       {"cidr_blocks", {"0.0.0.0/0"}},
                       {"description", "-"},
                       {"from_port", 0},
                       {"ipv6_cidr_blocks", {"::/0"}},
                       {"prefix_list_ids", json_t::array()},
                       {"protocol", "-1"},
                       {"security_groups", json_t::array()},
                       {"self", false},
                       {"to_port", 0},
                   }}},
                  {"ingress",
                   {{
                       {"cidr_blocks", {"0.0.0.0/0"}},
                       {"description", "-"},
                       {"from_port", 22},
                       {"ipv6_cidr_blocks", {"::/0"}},
                       {"prefix_list_ids", json_t::array()},
                       {"protocol", "tcp"},
                       {"security_groups", json_t::array()},
                       {"self", false},
                       {"to_port", 22},
                   }}},
                  {"provider", provider_ref},
              };
            }
          }
          std::string const security_group_id =
              "${aws_security_group." + region + ".id}";

          json_t & aws_ami = stack_tf["data"]["aws_ami"][region];
          if (aws_ami.is_null()) {
            if (cluster.os_version() != "22.04") {
              throw std::runtime_error(
                  "Unsupported os_version for aws: "
                  + sst::c_quote(cluster.os_version()));
            }
            aws_ami = {
                {"filter",
                 {
                     {
                         {"name", "name"},
                         {"values",
                          {"ubuntu/images/hvm-ssd/*-"
                           + cluster.os_version() + "-amd64-server-*"}},
                     },
                     {
                         {"name", "root-device-type"},
                         {"values", {"ebs"}},
                     },
                     {
                         {"name", "virtualization-type"},
                         {"values", {"hvm"}},
                     },
                 }},
                {"most_recent", true},
                {"owners", {"099720109477"}},
                {"provider", provider_ref},
            };
          }
          std::string const ami_id =
              "${data.aws_ami." + region + ".id}";

          for (decltype(cluster.count()) count_i = 0;
               count_i < cluster.count();
               ++count_i) {

            if (cluster.capacity() > 0.0) {
              total_host_capacity += cluster.capacity();
            }

            host_index_t const host_i = hosts.size();
            std::string const host_i_str = sst::to_string(host_i);
            std::string const host_id = "host" + host_i_str;

            std::string const instance_resource =
                cluster.spot() ? "aws_spot_instance_request" :
                                 "aws_instance";

            std::string const & provisioner0_id =
                host_id + "_provisioner0";

            std::string const & provisioner0_ref =
                "terraform_data." + provisioner0_id;

            json_t & provisioner0 =
                stack_setup.terraform_data[provisioner0_id];

            hosts.push_back(
                {cluster_i,
                 cluster_dir + "/hosts/" + host_i_str,
                 cluster_dir_rel + "/hosts/" + host_i_str,
                 {},
                 &provisioner0,
                 have_replay_file ?
                     &stack_setup
                          .terraform_data[host_id + "_provisioner1"] :
                     &provisioner0,
                 &stack_setup.resource[instance_resource][host_id]});
            host_info_t & host = hosts.back();
            cluster_info.hosts.push_back(host_i);
            sst::mkdir_p_new(host.dir);

            std::string const public_ip = "${" + instance_resource + "."
                                          + host_id + ".public_ip}";

            host_setup_t host_setup = do_host_setup(stack_setup,
                                                    provision_script,
                                                    cluster_i_str,
                                                    registry_image_url,
                                                    registry_username,
                                                    registry_password,
                                                    host_id,
                                                    provisioner0_ref,
                                                    host.dir_rel,
                                                    public_ip,
                                                    *host.provisioner0,
                                                    *host.provisioner1);

            json_t & resource = stack_tf["resource"];

            resource["aws_key_pair"][host_id] = {
                {"provider", provider_ref},
                {"public_key", host_setup.ssh_pk},
            };
            std::string const key_pair =
                "${aws_key_pair." + host_id + ".key_name}";

            json_t & machine = *host.machine;

            machine.merge_patch({
                {"ami", ami_id},
                {"instance_type", cluster.type()},
                {"key_name", key_pair},
                {"provider", provider_ref},
                {"root_block_device", {{"volume_type", "gp3"}}},
                {"vpc_security_group_ids", {security_group_id}},
            });

            if (cluster.spot()) {
              machine["wait_for_fulfillment"] = true;
            }

          } //
        }

      } break;

        //--------------------------------------------------------------

      case cluster_provider_t::azure(): {

        json_t & registry =
            stack_tf["resource"]["azurerm_container_registry"]
                    ["unique"];

        std::string const registry_image_url =
            "${join(\"\", "
            "[azurerm_container_registry.unique.login_server, "
            "\"/image\"])}";

        std::string const registry_username =
            "${azurerm_container_registry.unique.admin_username}";

        std::string const registry_password =
            "${nonsensitive(azurerm_container_registry.unique.admin_"
            "password)}";

        stack_setup_t stack_setup = do_stack_setup(stack_tf,
                                                   have_replay_file,
                                                   registry,
                                                   registry_image_url,
                                                   registry_username,
                                                   registry_password);

        {
          json_t & j = stack_setup.required_providers["azurerm"];
          if (j.is_null()) {
            j = {
                {"source", "hashicorp/azurerm"},
                {"version", "~> 3.0"},
            };
          }
        }

        for (decltype(cluster.regions().size()) region_i = 0;
             region_i < cluster.regions().size();
             ++region_i) {

          std::string const & region = cluster.regions()[region_i];
          std::string const provider_alias =
              cluster_name + "_provider" + sst::to_string(region_i);
          std::string const provider_ref = "azurerm." + provider_alias;

          {
            json_t p = json_t::object();
            p["features"] = json_t::object();
            if (cluster.subscription()) {
              p["subscription_id"] = *cluster.subscription();
            }
            json_t & ps = stack_setup.provider["azurerm"];
            if (ps.is_null()) {
              ps.push_back(p);
            }
            p["alias"] = provider_alias;
            ps.emplace_back(std::move(p));
          }

          {
            json_t & j =
                stack_setup.resource["azurerm_resource_group"][region];
            if (j.is_null()) {
              j = {
                  {"location", region},
                  {"name", stack_setup.random_name + "_" + region},
                  {"provider", provider_ref},
              };
            }
          }
          std::string const resource_group =
              "${azurerm_resource_group." + region + ".name}";

          if (stack_setup.create_registry) {
            registry["admin_enabled"] = true;
            registry["location"] = region;
            registry["provider"] = provider_ref;
            registry["resource_group_name"] = resource_group;
            registry["sku"] = "Basic";
            stack_setup.create_registry = false;
          }

          {
            json_t & j =
                stack_setup.resource["azurerm_virtual_network"][region];
            if (j.is_null()) {
              j = {
                  {"address_space", {"10.0.0.0/16"}},
                  {"location", region},
                  {"name", stack_setup.random_name},
                  {"provider", provider_ref},
                  {"resource_group_name", resource_group},
              };
            }
          }
          std::string const virtual_network =
              "${azurerm_virtual_network." + region + ".name}";

          {
            json_t & j = stack_setup.resource["azurerm_subnet"][region];
            if (j.is_null()) {
              j = {
                  {"address_prefixes", {"10.0.0.0/16"}},
                  {"name", stack_setup.random_name},
                  {"provider", provider_ref},
                  {"resource_group_name", resource_group},
                  {"virtual_network_name", virtual_network},
              };
            }
          }
          std::string const & subnet =
              "${azurerm_subnet." + region + ".id}";

          for (decltype(cluster.count()) count_i = 0;
               count_i < cluster.count();
               ++count_i) {

            if (cluster.capacity() > 0.0) {
              total_host_capacity += cluster.capacity();
            }

            host_index_t const host_i = hosts.size();
            std::string const host_i_str = sst::to_string(host_i);
            std::string const host_id = "host" + host_i_str;

            std::string const & provisioner0_id =
                host_id + "_provisioner0";

            std::string const & provisioner0_ref =
                "terraform_data." + provisioner0_id;

            json_t & provisioner0 =
                stack_setup.terraform_data[provisioner0_id];

            hosts.push_back(
                {cluster_i,
                 cluster_dir + "/hosts/" + host_i_str,
                 cluster_dir_rel + "/hosts/" + host_i_str,
                 {},
                 &provisioner0,
                 have_replay_file ?
                     &stack_setup
                          .terraform_data[host_id + "_provisioner1"] :
                     &provisioner0,
                 &stack_setup.resource["azurerm_linux_virtual_machine"]
                                      [host_id]});
            host_info_t & host = hosts.back();
            cluster_info.hosts.push_back(host_i);
            sst::mkdir_p_new(host.dir);

            std::string const & public_ip =
                "${data.azurerm_public_ip." + host_id + ".ip_address}";

            host_setup_t host_setup = do_host_setup(stack_setup,
                                                    provision_script,
                                                    cluster_i_str,
                                                    registry_image_url,
                                                    registry_username,
                                                    registry_password,
                                                    host_id,
                                                    provisioner0_ref,
                                                    host.dir_rel,
                                                    public_ip,
                                                    *host.provisioner0,
                                                    *host.provisioner1);

            stack_setup.resource["azurerm_public_ip"][host_id] = {
                {"allocation_method", "Dynamic"},
                {"location", region},
                {"name", host_id},
                {"provider", provider_ref},
                {"resource_group_name", resource_group},
            };
            std::string const & public_ip_id =
                "${azurerm_public_ip." + host_id + ".id}";

            stack_setup
                .resource["azurerm_network_interface"][host_id] = {
                {"ip_configuration",
                 {
                     {"name", "internal"},
                     {"private_ip_address_allocation", "Dynamic"},
                     {"public_ip_address_id", public_ip_id},
                     {"subnet_id", subnet},
                 }},
                {"location", region},
                {"name", host_id},
                {"provider", provider_ref},
                {"resource_group_name", resource_group},
            };
            std::string const & network_interface =
                "${azurerm_network_interface." + host_id + ".id}";

            json_t & machine = *host.machine;

            machine.merge_patch({
                {"admin_ssh_key",
                 {
                     {"public_key", host_setup.ssh_pk},
                     {"username", "ubuntu"},
                 }},
                {"admin_username", "ubuntu"},
                {"location", region},
                {"name", host_id},
                {"network_interface_ids", {network_interface}},
                {"os_disk",
                 {
                     {"caching", "ReadWrite"},
                     {"storage_account_type", "Standard_LRS"},
                 }},
                {"provider", provider_ref},
                {"resource_group_name", resource_group},
                {"size", cluster.type()},
            });
            if (cluster.os_version() == "22.04") {
              machine.merge_patch({
                  {"source_image_reference",
                   {
                       {"offer", "0001-com-ubuntu-server-jammy"},
                       {"publisher", "Canonical"},
                       {"sku", "22_04-lts"},
                       {"version", "22.04.202304280"},
                   }},
              });
            } else {
              throw std::runtime_error(
                  "Unsupported os_version for azure: "
                  + sst::c_quote(cluster.os_version()));
            }

            stack_setup.data["azurerm_public_ip"][host_id] = {
                {"name", host_id},
                {"provider", provider_ref},
                {"resource_group_name",
                 "${azurerm_linux_virtual_machine." + host_id
                     + ".resource_group_name}"},
            };

          } //

        } //

      } break;

        //--------------------------------------------------------------

      case cluster_provider_t::manual(): {

        for (decltype(cluster.count()) count_i = 0;
             count_i < cluster.count();
             ++count_i) {

          if (cluster.capacity() > 0.0) {
            total_host_capacity += cluster.capacity();
          }

          host_index_t const host_i = hosts.size();
          std::string const host_i_str = sst::to_string(host_i);
          hosts.push_back({
              cluster_i,
              cluster_dir + "/hosts/" + host_i_str,
              cluster_dir_rel + "/hosts/" + host_i_str,
              {},
              nullptr,
              nullptr,
              nullptr,
          });
          host_info_t & host = hosts.back();
          cluster_info.hosts.push_back(host_i);
          sst::mkdir_p_new(host.dir);

        } //

      } break;

        //--------------------------------------------------------------
    }

  } //

  //--------------------------------------------------------------------

  // TODO: Can we do better than all nodes consuming the same capacity?
  double const capacity_per_node =
      total_host_capacity / all_nodes.size();

  //--------------------------------------------------------------------
  // Assign the nodes to the hosts
  //--------------------------------------------------------------------
  //
  // At the end of this section, every hosts[i].nodes vector will be
  // filled in and ready to go.
  //

  {
    std::multimap<double, host_index_t, std::greater<double>> heap1;
    std::set<std::string> nodes1(all_nodes.begin(), all_nodes.end());
    for (cluster_index_t cluster_i{}; cluster_i < clusters.size();
         ++cluster_i) {
      cluster_spec_t const & cluster = clusters[cluster_i];
      cluster_info_t const & cluster_info = cluster_infos[cluster_i];
      decltype(heap1) heap2;
      for (host_index_t const host_i : cluster_info.hosts) {
        heap2.emplace(cluster.capacity(), host_i);
      }
      SST_ASSERT((!heap2.empty()));
      for (std::string const & psn : cluster.pin()) {
        auto const it = heap2.begin();
        double const capacity = it->first;
        host_index_t const host_i = it->second;
        host_info_t & host = hosts[host_i];
        host.nodes.emplace_back(psn);
        nodes1.erase(psn);
        heap2.erase(it);
        heap2.emplace(capacity - capacity_per_node, host_i);
      }
      heap1.insert(heap2.begin(), heap2.end());
    }
    std::vector<std::string> nodes2(nodes1.begin(), nodes1.end());
    std::shuffle(nodes2.begin(), nodes2.end(), sst::crypto_rng());
    for (std::string const & psn : nodes2) {
      auto const it = heap1.begin();
      double const capacity = it->first;
      host_index_t const host_i = it->second;
      host_info_t & host = hosts[host_i];
      host.nodes.emplace_back(psn);
      heap1.erase(it);
      heap1.emplace(capacity - capacity_per_node, host_i);
    }
  }

  //--------------------------------------------------------------------
  // Adjust the host disk sizes
  //--------------------------------------------------------------------

  for (decltype(hosts.size()) host_i{}; host_i < hosts.size();
       ++host_i) {

    host_info_t const & host = hosts[host_i];
    cluster_spec_t const & cluster_spec = clusters[host.cluster_i];

    if (cluster_spec.provider() == cluster_provider_t::aws()) {
      host.machine->merge_patch({
          {"root_block_device",
           {
               {"volume_size",
                static_cast<int>(8.0 + host.nodes.size() * 0.5)},
           }},
      });
    }
  }

  //--------------------------------------------------------------------
  // Finish setting up the Terraform-managed hosts
  //--------------------------------------------------------------------
  //
  // For each Terraform-managed host, this section:
  //
  //    1. Generates a Bash script
  //       clusters/<cluster_i>/hosts/<host_i>/nodes.tar.xz.bash
  //       that constructs
  //       clusters/<cluster_i>/hosts/<host_i>/nodes.tar.xz.
  //
  //    2. Generates Terraform code to locally run
  //       clusters/<cluster_i>/hosts/<host_i>/nodes.tar.xz.bash
  //       to construct
  //       clusters/<cluster_i>/hosts/<host_i>/nodes.tar.xz.
  //
  //    3. Generates Terraform code to upload
  //       clusters/<cluster_i>/hosts/<host_i> to the host.
  //
  //    4. Generates Terraform code to extract
  //       clusters/<cluster_i>/hosts/<host_i>/nodes.tar.xz on
  //       the host.
  //
  //    5. If replay.json exists, generates Terraform code to
  //       upload it to the host.
  //
  //    6. Generates Terraform code to run "docker compose up"
  //       on the host.
  //

  for (decltype(hosts.size()) host_i{}; host_i < hosts.size();
       ++host_i) {

    host_info_t const & host = hosts[host_i];

    if (host.provisioner0 == nullptr) {
      // This is not a Terraform-managed host.
      continue;
    }

    // (1)
    {
      std::ofstream f;
      f.exceptions(f.badbit | f.failbit);
      f.open(host.dir + "/nodes.tar.xz.bash");
      f << "set -E -e -u -o pipefail || exit $?\n";
      f << "trap exit ERR\n";
      f << "rm -f -r nodes\n";
      f << "mkdir nodes\n";
      for (std::string const & psn : host.nodes) {
        std::string const psn_slug = slugify(psn);
        f << "ln -s ../../../../../nodes/'" << psn_slug << "' nodes\n";
      }
      f << "tar ch nodes | xz -T 0 >nodes.tar.xz\n";
      f << "rm -f -r nodes\n";
      f.close();
    }

    json_t & provisioner0 = (*host.provisioner0)["provisioner"];
    json_t & provisioner1 = (*host.provisioner1)["provisioner"];

    // (2)
    provisioner0.push_back({
        {"local-exec",
         {
             {"command",
              "cd " + host.dir_rel + " && bash nodes.tar.xz.bash"},
         }},
    });

    // (3)
    {
      std::string const src = host.dir_rel;
      std::string const dst = "/home/ubuntu/" + src;
      provisioner0.push_back({
          {"file", {{"source", src}, {"destination", dst}}},
      });
    }

    // (4)
    provisioner0.push_back({
        {"remote-exec",
         {
             {"inline",
              {"cd /home/ubuntu/" + host.dir_rel
               + " && tar xJf nodes.tar.xz"}},
         }},
    });

    // (5)
    if (have_replay_file) {
      std::string const src = replay_file_rel;
      std::string const dst = "/home/ubuntu/" + src;
      provisioner0.push_back({
          {"file", {{"source", src}, {"destination", dst}}},
      });
    }

    // (6)
    provisioner1.push_back({
        {"remote-exec",
         {
             {"inline",
              {"cd /home/ubuntu/" + host.dir_rel
               + " && docker compose up -d"}},
         }},
    });

  } //

  //--------------------------------------------------------------------
  // Output the .tf.json files
  //--------------------------------------------------------------------

  if (!stack_tf.is_null()) {
    sst::json::get_as_file(stack_tf, work_dir + "/stack.tf.json");
  }

  //--------------------------------------------------------------------
  // Output the docker-compose.yml files
  //--------------------------------------------------------------------

  for (host_index_t host_i{}; host_i < hosts.size(); ++host_i) {
    std::string const host_i_str = sst::to_string(host_i);
    host_info_t const & host = hosts[host_i];
    json_t docker_compose;
    json_t & services = docker_compose["services"];
    for (decltype(host.nodes.size()) node_i{};
         node_i < host.nodes.size();
         ++node_i) {
      std::string const node_i_str = sst::to_string(node_i);
      std::string const & psn = host.nodes[node_i];
      std::string const psn_slug = slugify(psn);
      std::string const container_name =
          "host" + host_i_str + "_node" + node_i_str;
      services[container_name] = {
          {"command",
           {"/bin/sh",
            "-c",
            std::string() + "kestrel node"
                + (have_replay_file ? " --replay /replay.json" : "")
                + " 2>&1 >>race.log"}},
          {"container_name", container_name},
          {"image", "${KESTREL_DOCKER_IMAGE:-kestrel}"},
          {"network_mode", "host"},
          {"restart", "always"},
          {"volumes",
           {"./nodes/" + psn_slug + ":/data",
            "../../../../replay.json:/replay.json"}},
          {"working_dir", "/data"},
      };
    }
    if (services.is_null()) {
      services["noop"] = {
          {"container_name", "noop"},
          {"image", "hello-world"},
      };
    }
    sst::json::get_as_file(docker_compose,
                           host.dir + "/docker-compose.yml");
  }

  //--------------------------------------------------------------------
}

} // namespace kestrel
