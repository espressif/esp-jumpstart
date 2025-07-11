# -*- coding: utf-8 -*-
#
# Common (non-language-specific) configuration for ESP-Jumpstart Documentation
#

# type: ignore
# pylint: disable=wildcard-import
# pylint: disable=undefined-variable

from __future__ import print_function, unicode_literals

from esp_docs.conf_docs import *  # noqa: F403,F401
import os
import sys

sys.path.insert(0, os.path.abspath('.'))

# Utility function for version information
def run_cmd_get_output(cmd):
    return os.popen(cmd).read().strip()

extensions += [
    'sphinx_copybutton',
    # Note: Not using esp_docs.esp_extensions.dummy_build_system since ESP-Jumpstart
    # is a tutorial project without API documentation requirements
]

# Disable Doxygen for ESP-Jumpstart (tutorial project, no API docs needed)
extensions = [ext for ext in extensions if 'breathe' not in ext]

# Link roles config
github_repo = 'espressif/esp-jumpstart'

# Context used by sphinx_idf_theme
html_context['github_user'] = 'espressif'
html_context['github_repo'] = 'esp-jumpstart'
html_static_path = ['../_static']

# Extra options required by sphinx_idf_theme
html_context.update({
    'display_github': True, # Integrate GitHub
    'github_user': 'espressif', # Username
    'github_repo': 'esp-jumpstart', # Repo name
    'github_version': 'master', # Version
    'conf_py_path': '/docs/', # Path in the checkout to the docs root
})

# Final PDF filename will be <project_name>-<language>-<version>.pdf
project_name = 'esp-jumpstart'

# Version info
# The short X.Y version.
version = ''
# The full version, including alpha/beta/rc tags.
release = ''

# Disable Doxygen processing completely for ESP-Jumpstart
doxygen_build_dir = None

# Supported ESP32 targets for ESP-Jumpstart
idf_targets = ['esp32', 'esp32s2', 'esp32s3', 'esp32c2', 'esp32c3', 'esp32c6']
languages = ['en', 'zh_CN']

# ESP-Jumpstart specific settings
project_homepage = 'https://github.com/espressif/esp-jumpstart'
