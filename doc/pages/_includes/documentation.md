{%- comment -%}
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
{%- endcomment -%}

{%- comment -%}
//
// This file should roughly mirror the doc/readme/documentation.adoc
// file. If you edit this file, you might need to edit that file as
// well.
//
{%- endcomment -%}

## Documentation

{% assign fp_artifact = "manual" %}
{% assign fp_builtin_view_latest_url = "manual" %}
{% assign fp_builtin_view_pinned_url = "" %}
{% assign fp_builtin_download_latest_url = PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_builtin_download_pinned_url = "" %}
{% assign fp_github_view_latest_url = "https://stealthsoftwareinc.github.io/" | append: PACKAGE_TARNAME | append: "/manual" %}
{% assign fp_github_view_pinned_url = "" %}
{% assign fp_github_download_latest_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/raw/master/doc/" | append: PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_github_download_pinned_url = "" %}
{% assign fp_gitlab_view_latest_url = "" %}
{% assign fp_gitlab_view_pinned_url = "" %}
{% assign fp_gitlab_download_latest_url = "" %}
{% assign fp_gitlab_download_pinned_url = "" %}
{% assign fp_devel_view_latest_url = "https://www.stealthsoftwareinc.io/" | append: PACKAGE_TARNAME | append: "/manual" %}
{% assign fp_devel_view_pinned_url = "" %}
{% assign fp_devel_download_latest_url = "https://www.stealthsoftwareinc.io/" | append: PACKAGE_TARNAME | append: "/" | append: PACKAGE_TARNAME | append: "-manual.tar.xz" %}
{% assign fp_devel_download_pinned_url = "" %}
{% unless in_source_repo %}
{% assign fp_github_download_pinned_url = "https://github.com/stealthsoftwareinc/" | append: PACKAGE_TARNAME | append: "/raw/v" | append: PACKAGE_VERSION | append: "/doc/" | append: PACKAGE_TARNAME | append: "-" | append: PACKAGE_VERSION | append: "-manual.tar.xz" %}
{% endunless %}

{% include artifact_links_fragment.md %}
