# Copyright © 2012 CCP ehf.

"""
Wrapper module to access blue exposed things from EVE's pyEvePathfinder DLL.
"""

import blue
import sys

sys.modules[__name__] = blue.LoadExtension("_pyevepathfinder")
