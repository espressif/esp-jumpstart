# -*- coding: utf-8 -*-
#
# English Language ESP-Jumpstart Documentation Configuration
#
# Uses ../conf_common.py for most non-language-specific settings.

# Importing conf_common adds all the non-language-specific
# parts to this conf module
try:
    from conf_common import *  # noqa: F403,F401
except ImportError:
    import os
    import sys
    sys.path.insert(0, os.path.abspath('../'))
    from conf_common import *  # noqa: F403,F401

import datetime

current_year = datetime.datetime.now().year

# -- Project information -----------------------------------------------------

project = u'ESP-Jumpstart Programming Guide'
copyright = u'2018-{}, Espressif Systems (Shanghai) CO., LTD'.format(current_year)
author = 'Espressif'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
language = 'en'

# Version information
version = run_cmd_get_output('git describe --always --tags --dirty')
release = version

print('ESP-Jumpstart Documentation - Version: {0}  Release: {1}'.format(version, release))

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
  ('index', 'ReadtheDocsTemplate.tex', u'ESP-Jumpstart',
   u'2018 - 2019, Espressif Systems (Shanghai) PTE LTD', 'manual'),
]
