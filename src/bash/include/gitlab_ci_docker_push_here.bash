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

#
# gitlab_ci_docker_push_here <saved_image> <suffix> <config_h>
#

gitlab_ci_docker_push_here() {

  case $# in
    3)
    ;;
    *)
      sst_barf 'invalid argument count: %d' $#
    ;;
  esac

  local saved_image
  saved_image=$1
  readonly saved_image

  local suffix
  suffix=$2
  readonly suffix

  local config_h
  config_h=$3
  readonly config_h

  local tag
  tag=$(
    config_h_get_string \
      $config_h \
      PACKAGE_VERSION_DOCKER \
    ;
  )
  readonly tag

  local src
  src=$(docker load -q <$saved_image | sed -n '1s/.*: //p')
  readonly src

  local dst
  dst=$CI_REGISTRY_IMAGE/$CI_COMMIT_REF_SLUG$suffix
  readonly dst

  docker tag $src $dst:$tag
  docker tag $src $dst:latest

  docker login -u $CI_REGISTRY_USER --password-stdin $CI_REGISTRY <<EOF
$CI_REGISTRY_PASSWORD
EOF

  docker push $dst:$tag
  docker push $dst:latest

}; readonly -f gitlab_ci_docker_push_here
