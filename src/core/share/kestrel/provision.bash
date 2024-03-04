#! /bin/bash -
#
# Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an "AS
# IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
# express or implied. See the License for the specific language
# governing permissions and limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0
#

set -x

set -E -e -u -o pipefail || exit $?
trap exit ERR

#-----------------------------------------------------------------------
# Sleep a bit before doing anything else
#-----------------------------------------------------------------------
#
# Terraform sometimes gets into the host so fast that "apt-get update"
# fails with weird "No such file or directory" errors. Doing a sleep
# here before doing anything else seems to help.
#

sleep 30

#-----------------------------------------------------------------------
# Script parameters
#-----------------------------------------------------------------------
#
# The script parameters are passed in as environment variables.
#
# The "eval :" construct checks whether the variable is set without
# echoing it with set -x.
#

for x in \
  cluster_i \
  registry_password \
  registry_image_url \
  registry_username \
; do
  eval : "<<<\"\$$x\""
  export -n $x
  readonly $x
done

#-----------------------------------------------------------------------
# Ensure that apt-get doesn't block on stdin
#-----------------------------------------------------------------------

DEBIAN_FRONTEND=noninteractive
readonly DEBIAN_FRONTEND
export DEBIAN_FRONTEND

#-----------------------------------------------------------------------
# Install some basic packages
#-----------------------------------------------------------------------

sudo apt-get -q -y update

sudo apt-get -q -y install \
  ca-certificates \
  curl \
  gnupg \
  jq \
  lsb-release \
  xz-utils \
;

#-----------------------------------------------------------------------
# Install Docker
#-----------------------------------------------------------------------

sudo install -d -m 0755 /etc/apt/keyrings

curl -L -S -f -s \
  https://download.docker.com/linux/ubuntu/gpg \
| sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg

sudo chmod a+r /etc/apt/keyrings/docker.gpg

x="deb [arch=$(dpkg --print-architecture)"
x+=" signed-by=/etc/apt/keyrings/docker.gpg]"
x+=" https://download.docker.com/linux/ubuntu"
x+=" $(lsb_release --codename --short) stable"
sudo tee /etc/apt/sources.list.d/docker.list <<<"$x" >/dev/null

sudo apt-get -q -y update

sudo apt-get -q -y install \
  containerd.io \
  docker-buildx-plugin \
  docker-ce \
  docker-ce-cli \
  docker-compose-plugin \
;

x=$(sed -n '/^docker:/ p' /etc/group)
if [[ ! "$x" ]]; then
  sudo groupadd docker
fi

sudo usermod -G docker -a "$USER"

#-----------------------------------------------------------------------
# Log in to the container registry and pull the image
#-----------------------------------------------------------------------

registry_image_url_sh=\'${registry_image_url//\'/\'\\\'\'}\'
readonly registry_image_url_sh

registry_username_sh=\'${registry_username//\'/\'\\\'\'}\'
readonly registry_username_sh

sg docker 'docker login \
  --username '"$registry_username_sh"' \
  --password-stdin \
  -- \
  '"$registry_image_url_sh"' \
;' <<EOF
$registry_password
EOF

sg docker "docker pull $registry_image_url_sh"

#-----------------------------------------------------------------------
# Create the directories that Terraform will copy into
#-----------------------------------------------------------------------
#
# We create the ~/foo directories with plain mkdir instead of mkdir -p
# just to sanity check that they don't already exist (plain mkdir will
# fail in that case).
#

mkdir ~/clusters
mkdir ~/nodes

mkdir -p ~/clusters/$cluster_i/hosts
