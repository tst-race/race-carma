{% comment %}
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
{% endcomment %}

{% comment %}
//
// This file should roughly mirror the
// doc/readme/artifact_links_fragment.adoc file. If you edit this file,
// you might need to edit that file as well.
//
{% endcomment %}

{% assign fl_view_latest_text = "&#91;View latest&#93;" %}
{% assign fl_view_pinned_text = "&#91;View pinned&#93;" %}
{% assign fl_download_latest_text = "&#91;Download latest&#93;" %}
{% assign fl_download_pinned_text = "&#91;Download pinned&#93;" %}

You can access
{%- if in_source_repo %}
the public version of
{% endif -%}
the {{ fp_package_name }} {{ fp_artifact }}
at any of the following links:

{% unless in_source_repo %}

{% comment %}
//----------------------------------------------------------------------
// Builtin (public)
//----------------------------------------------------------------------
//
// This section should be the same as the Development section except
// with "Stealth GitLab" replaced by "This site" and "_devel" replaced
// by "_builtin" everywhere.
//
{% endcomment %}

{% assign fl_no_builtin = false %}
{% if fp_builtin_view_latest_url == "" %}
{% if fp_builtin_view_pinned_url == "" %}
{% if fp_builtin_download_latest_url == "" %}
{% if fp_builtin_download_pinned_url == "" %}
{% assign fl_no_builtin = true %}
{% endif %}
{% endif %}
{% endif %}
{% endif %}

{% unless fl_no_builtin %}
* This site:
{%- if fp_builtin_view_latest_url != "" %}
  [{{ fl_view_latest_text }}]({{ fp_builtin_view_latest_url }})
{%- endif -%}
{%- if fp_builtin_view_pinned_url != "" %}
  [{{ fl_view_pinned_text }}]({{ fp_builtin_view_pinned_url }})
{%- endif -%}
{%- if fp_builtin_download_latest_url != "" %}
  [{{ fl_download_latest_text }}]({{ fp_builtin_download_latest_url }})
{%- endif -%}
{%- if fp_builtin_download_pinned_url != "" %}
  [{{ fl_download_pinned_text }}]({{ fp_builtin_download_pinned_url }})
{%- endif -%}
{% endunless %}

{% comment %}
//----------------------------------------------------------------------
{% endcomment %}

{% endunless %}

{% comment %}
//----------------------------------------------------------------------
// GitHub
//----------------------------------------------------------------------
//
// This section should be the same as the Development section except
// with "Stealth GitLab" replaced by "GitHub" and "_devel" replaced by
// "_github" everywhere.
//
{% endcomment %}

{% assign fl_no_github = false %}
{% if fp_github_view_latest_url == "" %}
{% if fp_github_view_pinned_url == "" %}
{% if fp_github_download_latest_url == "" %}
{% if fp_github_download_pinned_url == "" %}
{% assign fl_no_github = true %}
{% endif %}
{% endif %}
{% endif %}
{% endif %}

{% unless fl_no_github %}
* GitHub:
{%- if fp_github_view_latest_url != "" %}
  [{{ fl_view_latest_text }}]({{ fp_github_view_latest_url }})
{%- endif -%}
{%- if fp_github_view_pinned_url != "" %}
  [{{ fl_view_pinned_text }}]({{ fp_github_view_pinned_url }})
{%- endif -%}
{%- if fp_github_download_latest_url != "" %}
  [{{ fl_download_latest_text }}]({{ fp_github_download_latest_url }})
{%- endif -%}
{%- if fp_github_download_pinned_url != "" %}
  [{{ fl_download_pinned_text }}]({{ fp_github_download_pinned_url }})
{%- endif -%}
{% endunless %}

{% comment %}
//----------------------------------------------------------------------
// GitLab
//----------------------------------------------------------------------
//
// This section should be the same as the Development section except
// with "Stealth GitLab" replaced by "GitLab" and "_devel" replaced by
// "_gitlab" everywhere.
//
{% endcomment %}

{% assign fl_no_gitlab = false %}
{% if fp_gitlab_view_latest_url == "" %}
{% if fp_gitlab_view_pinned_url == "" %}
{% if fp_gitlab_download_latest_url == "" %}
{% if fp_gitlab_download_pinned_url == "" %}
{% assign fl_no_gitlab = true %}
{% endif %}
{% endif %}
{% endif %}
{% endif %}

{% unless fl_no_gitlab %}
* GitLab:
{%- if fp_gitlab_view_latest_url != "" %}
  [{{ fl_view_latest_text }}]({{ fp_gitlab_view_latest_url }})
{%- endif -%}
{%- if fp_gitlab_view_pinned_url != "" %}
  [{{ fl_view_pinned_text }}]({{ fp_gitlab_view_pinned_url }})
{%- endif -%}
{%- if fp_gitlab_download_latest_url != "" %}
  [{{ fl_download_latest_text }}]({{ fp_gitlab_download_latest_url }})
{%- endif -%}
{%- if fp_gitlab_download_pinned_url != "" %}
  [{{ fl_download_pinned_text }}]({{ fp_gitlab_download_pinned_url }})
{%- endif -%}
{% endunless %}

{% comment %}
//----------------------------------------------------------------------
{% endcomment %}

{% if fl_no_builtin %}
{% if fl_no_github %}
{% if fl_no_gitlab %}
* No links available
{% endif %}
{% endif %}
{% endif %}

{% if in_source_repo %}

You can access
the development version of
the {{ fp_package_name }} {{ fp_artifact }}
at any of the following links:

{% comment %}
//----------------------------------------------------------------------
// Builtin (development)
//----------------------------------------------------------------------
//
// This section should be the same as the Development section except
// with "Stealth GitLab" replaced by "This site" and "_devel" replaced
// by "_builtin" everywhere.
//
{% endcomment %}

{% assign fl_no_builtin = false %}
{% if fp_builtin_view_latest_url == "" %}
{% if fp_builtin_view_pinned_url == "" %}
{% if fp_builtin_download_latest_url == "" %}
{% if fp_builtin_download_pinned_url == "" %}
{% assign fl_no_builtin = true %}
{% endif %}
{% endif %}
{% endif %}
{% endif %}

{% unless fl_no_builtin %}
* This site:
{%- if fp_builtin_view_latest_url != "" %}
  [{{ fl_view_latest_text }}]({{ fp_builtin_view_latest_url }})
{%- endif -%}
{%- if fp_builtin_view_pinned_url != "" %}
  [{{ fl_view_pinned_text }}]({{ fp_builtin_view_pinned_url }})
{%- endif -%}
{%- if fp_builtin_download_latest_url != "" %}
  [{{ fl_download_latest_text }}]({{ fp_builtin_download_latest_url }})
{%- endif -%}
{%- if fp_builtin_download_pinned_url != "" %}
  [{{ fl_download_pinned_text }}]({{ fp_builtin_download_pinned_url }})
{%- endif -%}
{% endunless %}

{% comment %}
//----------------------------------------------------------------------
// Development
//----------------------------------------------------------------------
{% endcomment %}

{% assign fl_no_devel = false %}
{% if fp_devel_view_latest_url == "" %}
{% if fp_devel_view_pinned_url == "" %}
{% if fp_devel_download_latest_url == "" %}
{% if fp_devel_download_pinned_url == "" %}
{% assign fl_no_devel = true %}
{% endif %}
{% endif %}
{% endif %}
{% endif %}

{% unless fl_no_devel %}
* Stealth GitLab:
{%- if fp_devel_view_latest_url != "" %}
  [{{ fl_view_latest_text }}]({{ fp_devel_view_latest_url }})
{%- endif -%}
{%- if fp_devel_view_pinned_url != "" %}
  [{{ fl_view_pinned_text }}]({{ fp_devel_view_pinned_url }})
{%- endif -%}
{%- if fp_devel_download_latest_url != "" %}
  [{{ fl_download_latest_text }}]({{ fp_devel_download_latest_url }})
{%- endif -%}
{%- if fp_devel_download_pinned_url != "" %}
  [{{ fl_download_pinned_text }}]({{ fp_devel_download_pinned_url }})
{%- endif -%}
{% endunless %}

{% comment %}
//----------------------------------------------------------------------
{% endcomment %}

{% if fl_no_devel %}
* No links available
{% endif %}

{% endif %}
