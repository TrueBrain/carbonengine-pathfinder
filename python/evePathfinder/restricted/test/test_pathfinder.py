import sys
import unittest
import unittest.mock as mock

from evePathfinder.restricted.pathfinder import ServerPathfinder
from evePathfinder.pathfinderconst import ROUTE_TYPE_UNSAFE_AND_NULL


class ServerPathfinderTestCase(unittest.TestCase):
    def setUp(self):
        self.pathfinderCore = mock.Mock()
        self.stateInterface = mock.Mock()
        self.isWithinSecInterval = mock.Mock()
        self.serverPathfinder = ServerPathfinder(self.pathfinderCore, self.stateInterface, self.isWithinSecInterval)
        self.fromID = 12
        self.toID = 33

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenInKnownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.serverPathfinder.GetPathBetween(self.fromID, self.toID)
        self.pathfinderCore.GetPathBetween.assert_called_once_with(self.stateInterface, self.fromID, self.toID)

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenFromWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, False]
        path = self.serverPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenToWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [False, True]
        path = self.serverPathfinder.GetPathBetween(self.fromID, self.toID)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenForRouteType(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.serverPathfinder.GetPathBetweenForRouteType(self.fromID, self.toID, ROUTE_TYPE_UNSAFE_AND_NULL)
        calls = self.pathfinderCore.GetPathBetween.call_args
        (stateInterface, fromID, toID), b = calls
        stateInterface.SetRouteType.assert_called_once_with(ROUTE_TYPE_UNSAFE_AND_NULL)

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetPathBetweenForRouteTypeToWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [False, True]
        path = self.serverPathfinder.GetPathBetweenForRouteType(self.fromID, self.toID, ROUTE_TYPE_UNSAFE_AND_NULL)
        self.assertEqual([], path)
        self.pathfinderCore.GetPathBetween.assert_has_calls([])

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountInKnownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.serverPathfinder.GetJumpCount(self.fromID, self.toID)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(self.stateInterface, self.fromID, self.toID)

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountInKnownSpaceWhenUnreachable(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, True]
        self.pathfinderCore.GetJumpCountBetween.return_value = -1
        count = self.serverPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxsize, count)
        self.pathfinderCore.GetJumpCountBetween.assert_called_once_with(self.stateInterface, self.fromID, self.toID)

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetJumpCountToWormholeSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.side_effect = [True, False]
        count = self.serverPathfinder.GetJumpCount(self.fromID, self.toID)
        self.assertEqual(sys.maxsize, count)
        self.pathfinderCore.GetJumpCountBetween.assert_has_calls([])

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetSystemsWithinJumpRangeFromUnknownSpace(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = False
        result = self.serverPathfinder.GetSystemsWithinJumpRange(self.fromID, 0, 1)
        self.assertEqual({}, result)

    @mock.patch("evePathfinder.restricted.pathfinder.IsKnownSpaceSystem")
    def testGetSystemsWithinJumpRange(self, isKnownSpaceSystem):
        isKnownSpaceSystem.return_value = True
        resID = 2
        expectedResult = {3: [resID, ]}
        self.pathfinderCore.GetSystemsWithinJumpRange.return_value = expectedResult
        minCount, maxCount = 2, 4
        secMin, secMax = 5, 8
        result = self.serverPathfinder.GetSystemsWithinJumpRange(
            self.fromID, minCount, maxCount, secMin, secMax
        )
        self.assertEqual(expectedResult, result)
        self.pathfinderCore.GetSystemsWithinJumpRange.assert_called_once_with(
            self.stateInterface, self.fromID,  minCount, maxCount
        )
        self.isWithinSecInterval.assert_called_once_with(resID, secMin, secMax)

    def testGetCachedEntryWillDefaultValue(self):
        fakeReturnValue = "TEST"
        self.pathfinderCore.CreateCacheEntry.return_value = fakeReturnValue
        cache = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertEqual(fakeReturnValue, cache)

    def testGetCachedEntryWillReturnSameObjectForSameParameters(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        cache1 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        cache2 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertIs(cache1, cache2)

    def testGetCachedEntryWillReturnDifferentObjectForDifferentFromID(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        cache1 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        cache2 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID+1)
        self.assertIsNot(cache1, cache2)

    def testGetCachedEntryWillReturnDifferentObjectForDifferentRouteType(self):
        self.pathfinderCore.CreateCacheEntry.side_effect = [1, 2]
        self.stateInterface.GetRouteType.return_value = 101
        cache1 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.stateInterface.GetRouteType.return_value = 102
        cache2 = self.serverPathfinder.GetCachedEntry(self.stateInterface, self.fromID)
        self.assertIsNot(cache1, cache2)