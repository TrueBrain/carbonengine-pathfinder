"""
Client implementation of a pathfinder.  Provides the interface to do pathfinding for UI and navigation.
Manages the client caching strategy
"""
from collections import defaultdict
import sys
from inventorycommon.util import IsWormholeSystem


class ClientPathfinder(object):
    def __init__(self, pathfinderCore, standardStateInterface, autopilotStateInterface,
                 convertStationIDToSolarSystemIDIfNecessaryMethod, getCurrentSystemMethod):
        # this is the standard pathfinding interface used when user autopilot settings are not relevant
        self._standardStateInterface = standardStateInterface
        # this is the interface used when user settings are taken into account (sec penalty, route settings)
        self._autopilotStateInterface = autopilotStateInterface
        self.pathfinderCore = pathfinderCore
        self.pathfinderCore.SetGetCachedEntryMethod(self.GetCachedEntry)
        self.pathfinderCacheByStateInterfaceAndRouteType = defaultdict(self.pathfinderCore.CreateCacheEntry)
        self.ConvertStationIDToSolarSystemIDIfNecessary = convertStationIDToSolarSystemIDIfNecessaryMethod
        self.GetCurrentSystem = getCurrentSystemMethod

    def GetCachedEntry(self, stateInterface, fromID):
        return self.pathfinderCacheByStateInterfaceAndRouteType[(id(stateInterface), stateInterface.GetRouteType())]

    def SetSecurityPenaltyFactor(self, securityPenalty):
        self._autopilotStateInterface.SetSecurityPenaltyFactor(securityPenalty)

    def SetPodKillAvoidance(self, pkAvoid):
        """
        Set pod kill avoidance to the specified value. If a change occurs,
        cache is invalidated, and will be refreshed
        """
        self._autopilotStateInterface.SetPodKillAvoidance(pkAvoid)

    def SetSystemAvoidance(self, pkAvoid=None):
        self._autopilotStateInterface.SetSystemAvoidance(pkAvoid)

    def GetAvoidanceItems(self):
        """
            returns a list of solar systems, constellations and regions on the character's autopilot avoidance list
            note: If the user has not changed his settings this function will return DEFAULT_AVOIDANCE
        """
        return self._autopilotStateInterface.GetAvoidanceItems(expandSystems=False)

    def GetExpandedAvoidanceItems(self):
        """
        Get avoidance items and expand solar systems and constellation to actual systems
        """
        return self._autopilotStateInterface.GetAvoidanceItems(expandSystems=True)

    def AddAvoidanceItem(self, itemID):
        """
            Adds a new solar system, constellation or region to the character's Avoidance List
        """
        items = self.GetAvoidanceItems()
        items.append(itemID)
        self._autopilotStateInterface.SetAvoidanceItems(items)

    def RemoveAvoidanceItem(self, itemID):
        """
            Removes a solar system, constellation or region from the character's Avoidance List
        """
        items = self.GetAvoidanceItems()

        if itemID in items:
            items.remove(itemID)
            self._autopilotStateInterface.SetAvoidanceItems(items)
            self.SetSystemAvoidance()

    def SetAutopilotRouteType(self, routeType):
        """
        Set the route type dictating what security rating is considered valid
        for pathfinding.
        """
        self._autopilotStateInterface.SetRouteType(routeType)

    def GetAutopilotRouteType(self):
        """
        Returns the current route type
        """
        return self._autopilotStateInterface.GetRouteType()

    def _AddDestinationToWaypointListIfNeeded(self, waypointList, destinationWaypoint):
        singlePathToWaypoint = waypointList

        # if we're going to a destination that is not a solarsystem, we need to add the destination to the
        # waypoint list (it the destination is reachable)
        if len(waypointList) > 0 and waypointList[-1] != destinationWaypoint:
            singlePathToWaypoint.append(destinationWaypoint)

        return singlePathToWaypoint

    def _GenerateWaypointListContainingSystemsAndStationIDs(self, listOfSystemIdWaypoints, waypoints):
        waypointIndex = 1
        waypointsWithSystemsAndStations = self._AddDestinationToWaypointListIfNeeded(listOfSystemIdWaypoints[0],
                                                                                     waypoints[waypointIndex])

        for systemIdWaypoints in listOfSystemIdWaypoints[1:]:
            waypointIndex += 1
            waypointsWithSystemsAndStations.extend(self._AddDestinationToWaypointListIfNeeded(systemIdWaypoints[1:],
                                                                                              waypoints[waypointIndex]))

        return waypointsWithSystemsAndStations

    def GetWaypointPath(self, waypoints):
        if IsWormholeSystem(self.GetCurrentSystem()):
            return []

        solarSystemWaypoints = map(self.ConvertStationIDToSolarSystemIDIfNecessary, waypoints)
        waypointListsContainingOnlySystems = self.pathfinderCore.GetListOfWaypointPaths(
            self._autopilotStateInterface,
            self.GetCurrentSystem(),
            solarSystemWaypoints
        )

        return self._GenerateWaypointListContainingSystemsAndStationIDs(waypointListsContainingOnlySystems, waypoints)

    def GetJumpCountsBetweenSystemPairs(self, sourceDestinationPairList):
        return self.pathfinderCore.GetJumpCountsBetweenSystemPairs(self._standardStateInterface, sourceDestinationPairList)

    def _GetPathBetween(self, stateInterface, fromID, toID):
        if IsWormholeSystem(fromID) or IsWormholeSystem(toID):
            return []

        return self.pathfinderCore.GetPathBetween(stateInterface, fromID, toID)

    def GetPathBetween(self, fromID, toID):
        return self._GetPathBetween(self._standardStateInterface, fromID, toID)

    def GetAutopilotPathBetween(self, fromID, toID):
        return self._GetPathBetween(self._autopilotStateInterface, fromID, toID)

    def _GetJumpCount(self, stateInterface, fromID, toID):
        if IsWormholeSystem(fromID) or IsWormholeSystem(toID):
            return sys.maxint

        return self.pathfinderCore.GetJumpCountBetween(stateInterface, fromID, toID)

    def GetJumpCount(self, fromID, toID):
        return self._GetJumpCount(self._standardStateInterface, fromID, toID)

    def GetAutopilotJumpCount(self, fromID, toID):
        return self._GetJumpCount(self._autopilotStateInterface, fromID, toID)

    def GetJumpCountFromCurrent(self, toID):
        return self._GetJumpCount(self._standardStateInterface, self.GetCurrentSystem(), toID)

    def GetSystemsWithinJumpRange(self, fromID, jumpCountMin, jumpCountMax):
        """
        Returns a map[jumpCount, list of systems] that have a jump count that is >= minCount and < maxCount
        """
        if IsWormholeSystem(fromID):
            return {}

        return self.pathfinderCore.GetSystemsWithinJumpRange(
            self._standardStateInterface, fromID, jumpCountMin, jumpCountMax)
