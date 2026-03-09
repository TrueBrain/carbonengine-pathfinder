# Copyright (c) CCP 2012

"""
Wrapper module to access blue exposed things from EVE's evePathfinder DLL.
"""

import blue
import sys

sys.modules[__name__] = blue.LoadExtension("_pyevepathfinder")
