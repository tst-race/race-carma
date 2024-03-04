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

case $0 in /*)
  x=$0
;; *)
  x=./$0
esac
r='\(.*/\)'
x=`expr "$x" : "$r"`generate_configs || exit $?
x=${x?}.`uname -m` || exit $?
readonly x

if test -x "$x"; then
  :
else
  chmod +rx "$x" || exit $?
  if test -x "$x"; then
    :
  else
    cat <<'EOF' || exit $?
{
  "attempt": 1,
  "status": "complete",
  "reason": "Error: Unknown error granting +x permission to the generate_configs executable file. The chmod +rx command succeeded, but the file is still not executable."
}
EOF
    exit 1
  fi
fi

case $# in 0)
  exec "$x" || exit $?
;; *)
  exec "$x" "$@" || exit $?
esac
