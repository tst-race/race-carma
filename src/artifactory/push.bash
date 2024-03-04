#! /bin/sh -
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
# This script uses the SST Bash library to bootstrap from /bin/sh into
# bash. See the Bash library > Bootstrapping section of the SST manual
# for more information.
#

#-----------------------------------------------------------------------
# Load the prelude
#-----------------------------------------------------------------------

case $0 in /*) x=$0 ;; *) x=./$0 ;; esac
x=`dirname "$x"` || exit $?
case $x in /) x= ;; esac
set -e || exit $?
. "$x"/../../src/bash/sst.bash

#-----------------------------------------------------------------------

sst_expect_argument_count $# 1

rt_env=$1
case $rt_env in
  dev | prod)
  ;;
  *)
    rt_env=$(sst_smart_quote "$rt_env")
    sst_barf "invalid rt_env: $rt_env"
  ;;
esac
readonly rt_env

sst_expect_source_path "$BASH_SOURCE"
d=$(sst_get_prefix $BASH_SOURCE)
readonly d

cd "$sst_tmpdir"
sst_extract_archive "$sst_rundir"/${d}kestrel-*-artifactory.tar.xz
cd *

readonly revision_regex='^(0|[1-9][0-9]*)$'

rt_repo=ta1_stealth-generic-$rt_env-local
readonly rt_repo

for race_version in race/*; do
  readonly race_version="${race_version##*/}"
done

for local_revision in race/"$race_version"/ta1/carma/r*; do
  readonly local_revision="${local_revision##*/r}"
done

if [[ ! "$local_revision" =~ $revision_regex ]]; then
  sst_barf "invalid local_revision: $local_revision"
fi

if [[ "${CI_PROJECT_PATH-}" == stealth/carma ]]; then
  in_ci=1
else
  in_ci=
fi
readonly in_ci

for latest_dir in race/$race_version/*/*/latest; do

  if [[ "${REVISION_PREFIX-}" ]]; then
    mv \
      "${latest_dir%/latest}/r$local_revision" \
      "${latest_dir%/latest}/${REVISION_PREFIX}-r$local_revision" \
    ;
    rm -f -r "$latest_dir"
  elif ((!in_ci)); then
    sst_barf "REVISION_PREFIX must be given oustide GitLab CI"
  fi

  for x in \
    rt_repo \
    race_version \
    latest_dir \
    RACE_JFROG_USER \
    RACE_JFROG_API_KEY \
  ; do
    eval 'q_'$x'=$(sst_quote "$'$x'")'
  done

  if ((in_ci)); then
    remote_revision=$(
      docker run --rm -i ${sst_is0atty:+-t} \
        releases-docker.jfrog.io/jfrog/jfrog-cli sh -c ': \
          && JFROG_CLI_LOG_LEVEL=ERROR jfrog rt search \
               --url="https://jfrog.race.twosixlabs.com/artifactory" \
               --user='"$q_RACE_JFROG_USER"' \
               --apikey='"$q_RACE_JFROG_API_KEY"' \
               --include-dirs \
               --recursive=false \
               '"$q_rt_repo"'/'"${latest_dir%/latest}"'/"r*" \
        ;' \
      | jq -r '.[] | .path' | sed 's|.*/r||' | sort -n -r | head -n 1
    )
    if [[ "$remote_revision" == "" ]]; then
      remote_revision=0
    fi

    if [[ ! "$remote_revision" =~ $revision_regex ]]; then
      sst_barf "invalid remote_revision: $remote_revision"
    fi

    if ((local_revision > remote_revision)); then
      docker run --rm -i ${sst_is0atty:+-t} \
        releases-docker.jfrog.io/jfrog/jfrog-cli sh -c ': \
          && jfrog rt delete \
               --url="https://jfrog.race.twosixlabs.com/artifactory" \
               --user='"$q_RACE_JFROG_USER"' \
               --apikey='"$q_RACE_JFROG_API_KEY"' \
               '"$q_rt_repo"'/'"$q_latest_dir"' \
        ;' \
      ;
    else
      rm -f -r "$latest_dir"
    fi
  fi

done

docker run --rm -i ${sst_is0atty:+-t} -v "$PWD":/x -w /x \
  releases-docker.jfrog.io/jfrog/jfrog-cli sh -c ': \
    && jfrog rt upload \
         --url="https://jfrog.race.twosixlabs.com/artifactory" \
         --user='"$q_RACE_JFROG_USER"' \
         --apikey='"$q_RACE_JFROG_API_KEY"' \
         --flat=false \
         --symlinks=true \
         "*" \
         '"$q_rt_repo"' \
  ;' \
;
