#!/usr/bin/env python
""" Extract version number from pyproject.toml
"""

import os

pyproject = os.path.join(os.path.dirname(__file__), "../../pyproject.toml")

data = open(pyproject).readlines()
version_line = next(line for line in data if line.startswith("version"))

version = version_line.strip().split(" = ")[1].replace('"', "").replace("'", "")

print(version)
