import site, os
PACKAGE_ROOT = os.path.abspath(os.path.join(__file__, '../../..'))
site.addsitedir(PACKAGE_ROOT)


import binbootstrapper
binbootstrapper.update_binaries(__file__, binbootstrapper.DLL_EVEPATHFINDER)
