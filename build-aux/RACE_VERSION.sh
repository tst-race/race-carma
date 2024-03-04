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

readonly git="${GIT:-git}"
readonly sed="${SED:-sed}"

if test -f build-aux/RACE_VERSION; then

  cat build-aux/RACE_VERSION || exit $?

elif eval " $git"' ls-files --error-unmatch "$0"' >/dev/null 2>&1; then

  d=`eval " $git"' describe --match="race-*" --candidates=1 --long'` || exit $?
  readonly d

  v='1s/^race-\([^-]*\).*/\1/p'
  v=`eval " $sed"' -n "$v"' <<====
$d
====
` || exit $?
  readonly v

  case $v in
    '')
  cat <<==== >&2
$0: parsing error
====
      exit 1
    ;;
  esac

  cat <<==== || exit $?
$v
====

else

  cat <<==== >&2
$0: no cache file or repository found
====
  exit 1

fi
