"""
Contains factory methods to construct a pathfinder core instance
"""

import pyEvePathfinder

from security.common.util import get_modified_security_level
from . import core
from inventorycommon.util import IsWormholeRegion


def CreatePathfinder(mapRegionCache, mapSystemCache, mapJumpCache):
    """
    returns pathinder initialized with an eve map and jump data
    """
    eveMap = pyEvePathfinder.EveMap()
    for regionID, regionItem in mapRegionCache.iteritems():
        eveMap.CreateRegion(regionID)

        for constellationID in regionItem.constellationIDs:
            eveMap.CreateConstellation(constellationID, regionID)

    for solarSystemID, ssInfo in mapSystemCache.iteritems():
        securityLevel = get_modified_security_level(solarSystemID)
        eveMap.CreateSolarSystem(solarSystemID, ssInfo.constellationID, securityLevel)

    # Once populated, create the jumps
    for jump in mapJumpCache:
        eveMap.AddJump(jump.fromSystemID, jump.toSystemID, jump.stargateID)
        eveMap.AddJump(jump.toSystemID, jump.fromSystemID, jump.stargateID)  # <-- TODO: stargateID wrong!)

    eveMap.Finalize()

    return core.EvePathfinderCore(eveMap)
