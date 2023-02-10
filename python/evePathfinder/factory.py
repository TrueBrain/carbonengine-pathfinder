"""
Contains factory methods to construct a pathfinder core instance
"""

import pyEvePathfinder
from carbon.common.lib import telemetry

from threadutils.be_nice import be_nice
from . import core


@telemetry.ZONE_METHOD
def CreatePathfinder(mapRegionCache, mapSystemCache, mapJumpCache, get_security_level_func, extraJumps=()):
    """
    returns pathinder initialized with an eve map and jump data
    """
    eveMap = pyEvePathfinder.EveMap()
    for regionID, regionItem in mapRegionCache.iteritems():
        eveMap.CreateRegion(regionID)

        for constellationID in regionItem.constellationIDs:
            eveMap.CreateConstellation(constellationID, regionID)
            be_nice()

    for solarSystemID, ssInfo in mapSystemCache.iteritems():
        securityLevel = get_security_level_func(solarSystemID)
        eveMap.CreateSolarSystem(solarSystemID, ssInfo.constellationID, securityLevel)
        be_nice()

    # Once populated, create the jumps
    for jump in mapJumpCache:
        eveMap.AddJump(jump.fromSystemID, jump.toSystemID, jump.stargateID)
        eveMap.AddJump(jump.toSystemID, jump.fromSystemID, jump.stargateID)  # <-- TODO: stargateID wrong!)
        be_nice()

    for fromSystemID, toSystemID, stargateID in extraJumps:
        eveMap.AddJump(fromSystemID, toSystemID, toSystemID) # <-- TODO: stargateID wrong! the function can't take bigint, doesn't seem to matter)
        be_nice()

    eveMap.Finalize()

    return core.EvePathfinderCore(eveMap)
