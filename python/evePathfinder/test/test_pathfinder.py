import unittest
import unittest.mock as mock
import sys
from evePathfinder.pathfinder import ClientPathfinder
from evePathfinder.pathfinderconst import ROUTE_TYPE_UNSAFE_AND_NULL


class ClientPathfinderTestCase(unittest.TestCase):
    def setUp(self):
        self.pathfinderCore = mock.Mock()
        self.stateInterface = mock.Mock()
        self.stateInterface2 = mock.Mock()
        self.fromID = 12
        self.toID = 33
        # hook up utility methods
        self.convertStationIDToSolarSystemIDIfNecessaryMethod = lambda x: x
        self.getCurrentSystemMethod = lambda: self.fromID

        self.clientPathfinder = ClientPathfinder(
            self.pathfinderCore,
            self.stateInterface,
            self.stateInterface2,
            self.convertStationIDToSolarSystemIDIfNecessaryMethod,
            self.getCurrentSystemMethod
        )

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenInKnownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.pathfinderCore.GetPathBetween.assert_called_once_with(self.stateInterface, self.fromID, self.toID)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenFromWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, False]
        path = self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenToWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [False, True]
        path = self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountSameFromToInKnownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [False, False]
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.fromID)
        self.assertEqual(0, count)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountSameFromToInWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.fromID)
        self.assertEqual(0, count)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountInKnownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(
            self.stateInterface, self.fromID, self.toID
        )

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountInKnownSpaceWhenUnreachable(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.pathfinderCore.GetJumpCountBetween.return_value = sys.maxsize
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxsize, count)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(
            self.stateInterface, self.fromID, self.toID
        )

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountToWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, False]
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxsize, count)
        self.pathfinderCore.GetJumpCountBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetSystemsWithinJumpRangeFromWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = False
        result = self.clientPathfinder.GetSystemsWithinJumpRange(self.fromID, 0, 1)
        self.assertEqual({}, result)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetSystemsWithinJumpRange(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = True
        expectedResult = {1: 2}
        self.pathfinderCore.GetSystemsWithinJumpRange.return_value = expectedResult
        minCount, maxCount = 2, 4
        result = self.clientPathfinder.GetSystemsWithinJumpRange(self.fromID, minCount, maxCount)
        self.assertEqual(expectedResult, result)
        self.pathfinderCore.GetSystemsWithinJumpRange.assert_called_once_with(
            self.stateInterface, self.fromID,  minCount, maxCount
        )

    def CreateFakePathfinderCore(self, returnFunc):
        class FakePathfinderCore:
            def SetGetCachedEntryMethod(self, method):
                self.GetCachedEntry = method

            def CreateCacheEntry(self):
                return returnFunc()

        return FakePathfinderCore()

    def testGetCachedEntryWillDefaultValue(self):
        fakeReturnValue = "TEST"
        self.pathfinderCore.CreateCacheEntry.return_value = fakeReturnValue
        cache = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertEqual(fakeReturnValue, cache)

    def testGetCachedEntryWillReturnSameObjectForSameParameters(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        cache1 = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        cache2 = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertIs(cache1, cache2)

    def testGetCachedEntryWillReturnDifferentObjectForDifferentStateInterface(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        cache1 = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        cache2 = self.clientPathfinder.GetCachedEntry(self.stateInterface2, self.fromID)
        self.assertIsNot(cache1, cache2)

    def testGetCachedEntryWillReturnDifferentObjectForDifferentRouteType(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        self.stateInterface.GetRouteType.return_value = 101
        cache1 = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.stateInterface.GetRouteType.return_value = 102
        cache2 = self.clientPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertIsNot(cache1, cache2)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testGetWaypointPathMapsStations(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = False
        waypoints = [5, 10, 15]
        path = [[5, 3, 10], [10, 4, 11]]
        self.pathfinderCore.GetListOfWaypointPaths.return_value = path
        self.convertStationIDToSolarSystemIDIfNecessaryMethod.side_effect = lambda x: 11 if x == 15 else x
        path = self.clientPathfinder.GetWaypointPath(waypoints)
        self.assertEqual([5, 3, 10, 4, 11, 15], path)

    @mock.patch("evePathfinder.pathfinder.IsKnownSpaceSystem")
    def testWaypointPathRespectsAutopilot(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = False
        waypoints = [5, 10, 11]
        path = [[5, 3, 10], [10, 4, 11]]
        self.pathfinderCore.GetListOfWaypointPaths.return_value = path
        path = self.clientPathfinder.GetWaypointPath(waypoints)
        self.pathfinderCore.GetListOfWaypointPaths.assert_called_once_with(
            self.stateInterface2, self.fromID, waypoints
        )
        self.assertEqual([5, 3, 10, 4, 11], path)

    def testGetCompleteWaypointList_ReturnsSolarSystemWaypointsIfNoNonSystemWaypointsAreSet(self):
        systemIdWaypoints = [[1, 2, 3]]
        waypoints = [1, 3]

        self.assertEqual(systemIdWaypoints[0], self.clientPathfinder.GetCompleteWaypointList(systemIdWaypoints, waypoints))

    def testGetCompleteWaypointList_ReturnsWaypointWithAddedStations(self):
        origin = 1
        destinationSystem = 2
        destinationStation = 10

        systemIdWaypoints = [[origin, destinationSystem]]
        waypoints = [origin, destinationStation]

        self.clientPathfinder.ConvertStationIDToSolarSystemIDIfNecessary = lambda x: 2 if x == 10 else x

        self.assertEqual([origin, destinationSystem, destinationStation], self.clientPathfinder.GetCompleteWaypointList(systemIdWaypoints, waypoints))

    def testGetCompleteWaypointList_ReturnsWaypointWithAllStationsInSystem(self):
        originSolarSystem = 1
        destinationSolarSystem = 2
        destinationStation = 10
        secondDestinationStation = 11

        systemIdWaypoints = [[originSolarSystem, destinationSolarSystem], [destinationSolarSystem]]
        waypoints = [originSolarSystem, destinationStation, secondDestinationStation]

        self.clientPathfinder.ConvertStationIDToSolarSystemIDIfNecessary = lambda x: destinationSolarSystem if x in (destinationStation, secondDestinationStation) else x

        self.assertEqual([originSolarSystem, destinationSolarSystem, destinationStation, secondDestinationStation], self.clientPathfinder.GetCompleteWaypointList(systemIdWaypoints, waypoints))

    def testGetCompeleteWaypointList_DoesNotRepeatSameSolarsystemInRow(self):
        o = 1
        d1 = 3
        d2 = 5
        d3 = 7
        systemIdWaypoints = [[o, d1], [d1, d2], [d2, d1, d3]]
        waypoints = [o, d1, d2, d3]

        self.assertEqual([o, d1, d2, d1, d3], self.clientPathfinder.GetCompleteWaypointList(systemIdWaypoints, waypoints))

    def testGetCompleteWaypointList_ReturnsEmptyList_IfDestinationIsUnreachable(self):
        unreachablePath = [[]]
        self.assertEqual([], self.clientPathfinder.GetCompleteWaypointList(unreachablePath, [1, 2]))

    def testAddAvoidanceItemsSetsNewValue(self):
        avoided = [1, 2]
        self.stateInterface2.GetAvoidanceItems.return_value = avoided
        self.clientPathfinder.AddAvoidanceItem(4)
        self.stateInterface2.SetAvoidanceItems.assert_called_once_with([1, 2, 4])

    def testRemoveAvoidanceItemRemovesItem(self):
        avoided = [1, 2, 3]
        self.stateInterface2.GetAvoidanceItems.return_value = avoided
        self.clientPathfinder.RemoveAvoidanceItem(2)
        self.stateInterface2.SetAvoidanceItems.assert_called_once_with([1, 3])
