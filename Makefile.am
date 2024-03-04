##
## Copyright (C) 2019-2024 Stealth Software Technologies, Inc.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing,
## software distributed under the License is distributed on an "AS
## IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
## express or implied. See the License for the specific language
## governing permissions and limitations under the License.
##
## SPDX-License-Identifier: Apache-2.0
##

#
# Some parts of this file are from the CARMA package.
#
# The following copyright notice is generally applicable:
#
#      Copyright (C) Stealth Software Technologies, Inc.
#
# The full copyright information depends on the distribution
# of the package. For more information, see the COPYING file.
# However, depending on the context in which you are viewing
# this file, the COPYING file may not be available.
#

include $(srcdir)/build-aux/gatbps.am

DISTCLEANFILES =
check_PROGRAMS =
lib_LTLIBRARIES =
nodist_noinst_HEADERS =
noinst_LIBRARIES =
noinst_LTLIBRARIES =

AM_CPPFLAGS =
AM_CPPFLAGS += -Isrc/core -I$(srcdir)/src/core
AM_CPPFLAGS += -Isrc/misc -I$(srcdir)/src/misc
AM_CPPFLAGS += -Itest -I$(srcdir)/test
if WITH_MOCK_SDK
AM_CPPFLAGS += -Isrc/mock -I$(srcdir)/src/mock
AM_CPPFLAGS += -Isrc/sdk -I$(srcdir)/src/sdk
endif WITH_MOCK_SDK

#
# Kestrel only needs C++11, but the RACE SDK needs C++17.
#

AM_CXXFLAGS =
AM_CXXFLAGS += -std=c++17

data_carmadir = $(datadir)/carma
include_kestreldir = $(includedir)/kestrel

## begin_variables

ACLOCAL_AMFLAGS = -I m4

AM_MAKEINFOFLAGS = \
  -I doc/texi \
  -I $(srcdir)/doc/texi \
$(prevent_an_empty_line)

AM_MAKEINFOHTMLFLAGS = \
  $(AM_MAKEINFOFLAGS) \
  --css-include=texinfo.css \
  --no-headers \
  --no-split \
  --set-customization-variable TOP_NODE_UP=Top \
$(prevent_an_empty_line)

AM_TEXI2DVIFLAGS = \
  $(AM_MAKEINFOFLAGS) \
$(prevent_an_empty_line)

BUILT_SOURCES =
BUILT_SOURCES += src/misc/nlohmann/json.hpp

MAINTAINERCLEANFILES =

MOSTLYCLEANFILES =

SUBDIRS =

TESTS =

TMPEXT = .buildgenerated.tmp

XFAIL_TESTS =

dist_pkgdata_DATA =

gen_targets =

include_HEADERS =

include_kestrel_HEADERS =

## end_variables

## begin_variable_additions

## end_variable_additions

## begin_rules

$(gen_targets) DUMMY_gen_targets: FORCE
	$(AM_V_at)cd gen && $(MAKE) $(AM_MAKEFLAGS) $@

.PHONY: $(gen_targets)
.PHONY: DUMMY_gen_targets
.PHONY: docker

.PRECIOUS: $(gen_targets)

docker: FORCE
docker: src/docker/kestrel.tar.iid

## end_rules


##----------------------------------------------------------------------
## test_mock
##----------------------------------------------------------------------

test_mock: FORCE
test_mock: all
	@( \
	  x=src/docker/kestrel-loop.tar.iid; \
	  if test -f $$x; then \
	    x="src_docker_kestrel_loop_from_image= $$x"; \
	  fi; \
	  $(MAKE) $$x || exit $$?; \
	)
	src/core/bin/carma test \
	  --image=kestrel-loop:$(PACKAGE_VERSION_DOCKER) \
	;

##----------------------------------------------------------------------
## test_rib
##----------------------------------------------------------------------

test_rib: \
  FORCE \
  all \
  src/artifactory/kestrel-$(PACKAGE_VERSION)-artifactory.tar.xz \
$(prevent_an_empty_line)
	@( \
	  p=test-`date '+%Y-%m-%d-%H%M%S'` || exit $$?; \
	  set x $(OPENSSL) rand -hex 4 || exit $$?; \
	  shift; \
	  p=$$p-`"$$@"` || exit $$?; \
	  \
	    REVISION_PREFIX=$$p \
	    RACE_JFROG_USER=$${RACE_JFROG_USER:-$(RACE_JFROG_USER)} \
	    RACE_JFROG_API_KEY=$${RACE_JFROG_API_KEY:-$(RACE_JFROG_API_KEY)} \
	  src/artifactory/push.bash dev || exit $$?; \
	  src/core/bin/carma test rib \
	    --ta1-plugin=carma:$$p-r$(ARTIFACTORY_REVISION):dev \
	  || exit $$?; \
	)

##----------------------------------------------------------------------
## Custom version cache files
##----------------------------------------------------------------------

GATBPS_GEN_VERSION_CACHE_FILES += \
  build-aux/gatbps-gen-version.docker.sh.cache \
$(empty)

build-aux/ARTIFACTORY_REVISION:
	$(AM_V_GEN)$(GATBPS_RECIPE_MARKER_TOP)
	$(AM_V_at)rm -f -r ./$@ ./$@$(TSUF)*
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_at)sh $(srcdir)/build-aux/ARTIFACTORY_REVISION.sh >$@$(TSUF)
	$(AM_V_at)mv -f $@$(TSUF) $@
	$(AM_V_at)$(GATBPS_RECIPE_MARKER_BOT)

build-aux/ARTIFACTORY_REVISION/clean: FORCE
	-rm -f -r ./$(@D) ./$(@D)$(TSUF)*

maintainer-clean-local: build-aux/ARTIFACTORY_REVISION/clean

build-aux/RACE_VERSION:
	$(AM_V_GEN)$(GATBPS_RECIPE_MARKER_TOP)
	$(AM_V_at)rm -f -r ./$@ ./$@$(TSUF)*
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_at)sh $(srcdir)/build-aux/RACE_VERSION.sh >$@$(TSUF)
	$(AM_V_at)mv -f $@$(TSUF) $@
	$(AM_V_at)$(GATBPS_RECIPE_MARKER_BOT)

build-aux/RACE_VERSION/clean: FORCE
	-rm -f -r ./$(@D) ./$(@D)$(TSUF)*

maintainer-clean-local: build-aux/RACE_VERSION/clean

build-aux/RIB_VERSION:
	$(AM_V_GEN)$(GATBPS_RECIPE_MARKER_TOP)
	$(AM_V_at)rm -f -r ./$@ ./$@$(TSUF)*
	$(AM_V_at)$(MKDIR_P) $(@D)
	$(AM_V_at)sh $(srcdir)/build-aux/RIB_VERSION.sh >$@$(TSUF)
	$(AM_V_at)mv -f $@$(TSUF) $@
	$(AM_V_at)$(GATBPS_RECIPE_MARKER_BOT)

build-aux/RIB_VERSION/clean: FORCE
	-rm -f -r ./$(@D) ./$(@D)$(TSUF)*

maintainer-clean-local: build-aux/RIB_VERSION/clean

##----------------------------------------------------------------------

include $(srcdir)/src/core/libcarma_client_la.am
include $(srcdir)/src/core/libcarma_la.am
include $(srcdir)/src/core/libcarma_server_la.am
include $(srcdir)/src/core/libkestrel_rabbitmq_la.am
include $(srcdir)/src/mock/libcarmamock_la.am
include $(srcdir)/src/sdk/libsdk_la.am

include $(srcdir)/build-aux/GATBPS_CONFIG_FILE_RULES.am
include $(srcdir)/build-aux/GATBPS_CONFIG_LATER_RULES.am
include $(srcdir)/build-aux/GATBPS_CP_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_ARCHIVE_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_BUNDLE_RULES.am
include $(srcdir)/build-aux/GATBPS_GIT_CLONE_RULES.am
include $(srcdir)/build-aux/GATBPS_VPATH_SEARCH.am
include $(srcdir)/build-aux/GATBPS_V_PAD.am
include $(srcdir)/build-aux/GATBPS_WGET_RULES.am
include $(srcdir)/build-aux/cp.am
include $(srcdir)/build-aux/docker.am
include $(srcdir)/build-aux/doxygen.am
include $(srcdir)/build-aux/git.am
include $(srcdir)/build-aux/wget.am

include $(srcdir)/build-aux/autogen.am
