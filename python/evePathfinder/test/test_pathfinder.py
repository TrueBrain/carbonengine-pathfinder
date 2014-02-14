import unittest
import mock
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

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetPathBetweenInKnownSpace(self, isWormholeSystem):
        isWormholeSystem.side_effect = [False, False]
        self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.pathfinderCore.GetPathBetween.assert_called_once_with(self.stateInterface, self.fromID, self.toID)

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetPathBetweenFromWormholeSpace(self, isWormholeSystem):
        isWormholeSystem.side_effect = [True, False]
        path = self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetPathBetweenToWormholeSpace(self, isWormholeSystem):
        isWormholeSystem.side_effect = [False, True]
        path = self.clientPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetJumpCountInKnownSpace(self, isWormholeSystem):
        isWormholeSystem.side_effect = [False, False]
        self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(
            self.stateInterface, self.fromID, self.toID
        )

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetJumpCountInKnownSpaceWhenUnreachable(self, isWormholeSystem):
        isWormholeSystem.side_effect = [False, False]
        self.pathfinderCore.GetJumpCountBetween.return_value = sys.maxint
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxint, count)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(
            self.stateInterface, self.fromID, self.toID
        )

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetJumpCountToWormholeSpace(self, isWormholeSystem):
        isWormholeSystem.side_effect = [True, False]
        count = self.clientPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxint, count)
        self.pathfinderCore.GetJumpCountBetween.assert_has_calls([])

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetSystemsWithinJumpRangeFromWormholeSpace(self, isWormholeSystem):
        isWormholeSystem.return_value = True
        result = self.clientPathfinder.GetSystemsWithinJumpRange(self.fromID, 0, 1)
        self.assertEqual({}, result)

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetSystemsWithinJumpRange(self, isWormholeSystem):
        isWormholeSystem.return_value = False
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

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetWaypointPathFromWormholeSystemReturnsEmptyList(self, isWormholeSystem):
        isWormholeSystem.return_value = True
        waypoints = [5, 10, 15]
        path = self.clientPathfinder.GetWaypointPath(waypoints)
        self.assertEqual(path, [])

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testGetWaypointPathMapsStations(self, isWormholeSystem):
        isWormholeSystem.return_value = False
        waypoints = [5, 10, 15]
        path = [[5, 3, 10], [10, 4, 11]]
        self.pathfinderCore.GetListOfWaypointPaths.return_value = path
        self.convertStationIDToSolarSystemIDIfNecessaryMethod.side_effect = lambda x: 11 if x == 15 else x
        path = self.clientPathfinder.GetWaypointPath(waypoints)
        self.assertEqual(path, [5, 3, 10, 4, 11, 15])

    @mock.patch("evePathfinder.pathfinder.IsWormholeSystem")
    def testWaypointPathRespectsAutopilot(self, isWormholeSystem):
        isWormholeSystem.return_value = False
        waypoints = [5, 10, 11]
        path = [[5, 3, 10], [10, 4, 11]]
        self.pathfinderCore.GetListOfWaypointPaths.return_value = path
        path = self.clientPathfinder.GetWaypointPath(waypoints)
        self.pathfinderCore.GetListOfWaypointPaths.assert_called_once_with(
            self.stateInterface2, self.fromID, waypoints
        )
        self.assertEqual(path, [5, 3, 10, 4, 11])


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

