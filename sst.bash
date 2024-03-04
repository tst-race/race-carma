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
# This file provides a stable location from which the SST Bash library
# can be included by Bash scripts in this repository. This file should
# always exist in the root of the repository as sst.bash, and the "."
# command below should be adjusted to include the true sst.bash file
# from its true location in the repository.
#

set -e || exit $?
. src/bash/sst.bash
