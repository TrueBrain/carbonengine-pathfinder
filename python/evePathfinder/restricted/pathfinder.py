"""
A server implementation of a pathfinder service. Provides the interface and caching strategy.
"""
import sys
from copy import copy
from collections import defaultdict
from inventorycommon.util import IsWormholeSystem


class ServerPathfinder(object):
    def __init__(self, pathfinderCore, stateInterface, isWithinSecIntervalFunc):
        self._stateInterface = stateInterface
        self._pathfinderCore = pathfinderCore
        self._pathfinderCore.SetGetCachedEntryMethod(self.GetCachedEntry)
        self.pathfinderCacheByRouteTypeAndFromID = defaultdict(self._pathfinderCore.CreateCacheEntry)
        self.IsWithinSecInterval = isWithinSecIntervalFunc

    def GetCachedEntry(self, stateInterface, fromID):
        """
        We key the cache so the each source solar system has it's own cache for each route type
        Since most users of the pathfinder are location bound this should be a limited set matching the mapped systems on the relevant node.
        """
        return self.pathfinderCacheByRouteTypeAndFromID[(stateInterface.GetRouteType(), fromID)]

    def GetPathBetween(self, fromID, toID):
        if IsWormholeSystem(fromID) or IsWormholeSystem(toID):
            return []

        return self._pathfinderCore.GetPathBetween(self._stateInterface, fromID, toID)

    def GetPathBetweenForRouteType(self, fromID, toID, routeType):
        if IsWormholeSystem(fromID) or IsWormholeSystem(toID):
            return []
        # take a copy of state interface so we can safely swap the route type
        tempStateInterface = copy(self._stateInterface)
        tempStateInterface.SetRouteType(routeType)
        return self._pathfinderCore.GetPathBetween(tempStateInterface, fromID, toID)

    def GetJumpCount(self, fromID, toID):
        if IsWormholeSystem(fromID) or IsWormholeSystem(toID):
            return sys.maxint

        jc = self._pathfinderCore.GetJumpCountBetween(self._stateInterface, fromID, toID)
        if jc == -1:
            return sys.maxint
        else:
            return jc

    def FilterSystemsBySecurityLevel(self, secMax, secMin, systemsByJumpRange):
        if secMin is None:
            secMin = -1.0
        if secMax is None:
            secMax = 1.0
        filteredSystemsByJumpRange = {}

        for jumpRange, solarSystems in systemsByJumpRange.iteritems():
            filteredSystems = []
            for solarSystemID in solarSystems:
                if self.IsWithinSecInterval(solarSystemID, secMin, secMax):
                    filteredSystems.append(solarSystemID)
            filteredSystemsByJumpRange[jumpRange] = filteredSystems
        systemsByJumpRange = filteredSystemsByJumpRange
        return systemsByJumpRange

    def GetSystemsWithinJumpRange(self, fromID, jumpCountMin, jumpCountMax, secMin=None, secMax=None):
        """
        Returns a map[jumpCount, list of systems] that have a jump count that is >= minCount and < maxCount
        """
        if IsWormholeSystem(fromID):
            return {}

        systemsByJumpRange = self._pathfinderCore.GetSystemsWithinJumpRange(
            self._stateInterface, fromID, jumpCountMin, jumpCountMax
        )

        if secMin or secMax:
            systemsByJumpRange = self.FilterSystemsBySecurityLevel(secMax, secMin, systemsByJumpRange)

        return systemsByJumpRange
