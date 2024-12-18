import os
import sys
import unittest
import logging
import unittest.mock as mock
from collections import defaultdict

from . import setupenv

import evePathfinder.core
from evePathfinder.core import PathfinderCacheEntry
import pyEvePathfinder
import evePathfinder.cache
from evePathfinder.pathfinderconst import *
from evePathfinder.pathfinderconst import UNREACHABLE_JUMP_COUNT
import math
import hashlib

try:
    import gevent
    geventAvailable = True
except ImportError:
    geventAvailable = False


def CreateSimpleNewStyleMapData():
    '''
    Creates a map like this:
           [4] (0.3)
     <-----> C <-----> 
    [2] A (1.0)            [6] E (1.0)
     <--> [3] B <--> [5] D <->
        (0.8)  (0.45)

    Security ratings in parentheses
    '''
    m = pyEvePathfinder.EveMap(7,10)
    REGION = 0
    m.CreateRegion( REGION )

    CONSTELLATION = 1
    m.CreateConstellation( CONSTELLATION, REGION )

    A = 2
    B = 3
    C = 4
    D = 5
    E = 6

    m.CreateSolarSystem( A, CONSTELLATION, 1.0 )
    m.CreateSolarSystem( B, CONSTELLATION, 0.8 )
    m.CreateSolarSystem( C, CONSTELLATION, 0.3 )
    m.CreateSolarSystem( D, CONSTELLATION, 0.45 )
    m.CreateSolarSystem( E, CONSTELLATION, 1.0 )

    m.AddJump( A, C, 7 )
    m.AddJump( C, A, 8 )

    m.AddJump( A, B, 9 )
    m.AddJump( B, A, 10 )

    m.AddJump( B, D, 11 )
    m.AddJump( D, B, 12 )

    m.AddJump( C, E, 13 )
    m.AddJump( E, C, 14 )

    m.AddJump( D, E, 15 )
    m.AddJump( E, D, 16 )

    m.Finalize()

    return m

def CreateTShapedSolarSystemMap():
    '''
    Creates a map like this:

        A ---- B ----- C
               |
               |
               D
    '''
    m = pyEvePathfinder.EveMap()
    REGION = 0
    m.CreateRegion( REGION )

    CONSTELLATION = 1
    m.CreateConstellation( CONSTELLATION, REGION )

    A = 2
    B = 3
    C = 4
    D = 5

    m.CreateSolarSystem( A, CONSTELLATION, 1.0 )
    m.CreateSolarSystem( B, CONSTELLATION, 0.8 )
    m.CreateSolarSystem( C, CONSTELLATION, 0.3 )
    m.CreateSolarSystem( D, CONSTELLATION, 0.45 )

    m.AddJump( A, B, 7 )
    m.AddJump( B, A, 8 )

    m.AddJump( B, C, 9 )
    m.AddJump( C, B, 10 )

    m.AddJump( B, D, 11 )
    m.AddJump( D, B, 12 )

    m.Finalize()

    return m

def CreateSimpleMapWithUnreachableSystem():
    '''
            <--> (3) B <-->
        (2) A           (4) C
             <--------->

         (5) D
    '''
    m = pyEvePathfinder.EveMap()

    REGION = 0
    m.CreateRegion( REGION )

    CONSTELLATION = 1
    m.CreateConstellation( CONSTELLATION, REGION )

    A = 2
    B = 3
    C = 4
    D = 5

    m.CreateSolarSystem( A, CONSTELLATION, 1.0 )
    m.CreateSolarSystem( B, CONSTELLATION, 0.8 )
    m.CreateSolarSystem( C, CONSTELLATION, 0.3 )
    m.CreateSolarSystem( D, CONSTELLATION, 0.45 )

    m.AddJump( A, B, 7 )
    m.AddJump( B, A, 8 )

    m.AddJump( A, B, 9 )
    m.AddJump( B, A, 10 )

    m.AddJump( C, A, 11 )
    m.AddJump( A, C, 12 )

    m.Finalize()

    return m

class StatefulPathfinderTestInterface(object):

    def __init__(self):
        self.avoidanceList = []
        self.podKillList = []
        self.routeType = 'shortest'
        self.avoid = True
        self.podkillAvoid = False

    def GetPodkillSystemList(self):
        return self.podKillList

    def GetSecurityPenalty(self):
        return math.exp( 0.15 * 50 )

    def GetAvoidanceList(self):
        return self.avoidanceList

    def GetRouteType(self):
        return self.routeType

    def IsAvoidanceEnabled(self):
        return self.avoid

    def IsPodkillAvoidanceEnabled(self):
        return self.podkillAvoid

    def SetRouteType(self, routeType):
        self.routeType = routeType

    def GetCurrentStateHash(self, fromSolarSystemID):
        m = hashlib.md5()
        m.update(str(fromSolarSystemID).encode('utf-8'))
        m.update(self.routeType.encode('utf-8'))
        m.update(str(self.avoidanceList).encode('utf-8'))
        return m.hexdigest()

def CreateCacheEntry(newStyleMap):
        return PathfinderCacheEntry(None, evePathfinder.core.NewPathfinderCache(newStyleMap))


def CreateSimplePathfinderCore(newMapCreationFunction=None):

    if newMapCreationFunction:
        newStyleMap = newMapCreationFunction()
    else:
        newStyleMap = CreateSimpleNewStyleMapData()

    def GetCachedEntry(stateInterface, fromID):
        return PathfinderCacheEntry(None, evePathfinder.core.NewPathfinderCache(newStyleMap))

    pathfinderCore = evePathfinder.core.EvePathfinderCore(newStyleMap)
    pathfinderCore.SetGetCachedEntryMethod(GetCachedEntry)

    return pathfinderCore


class testPathfinderCore(unittest.TestCase):

    def setUp(self):
        self.longMessage = True
        self.fromID = 2
        self.stateInterface = StatefulPathfinderTestInterface()
        self.newStyleMap = CreateSimpleNewStyleMapData()
        self.core = CreateSimplePathfinderCore(lambda: self.newStyleMap)

    def testInitialization(self):
        newStyleMap = CreateSimpleNewStyleMapData()
        inst = evePathfinder.core.EvePathfinderCore(newStyleMap)

        self.assertIsNotNone(inst)

    def testPairSequence(self):
        z = evePathfinder.core.PairSequence([1, 2, 3, 4])

        self.assertEqual(next(z), (1, 2))
        self.assertEqual(next(z), (2, 3))
        self.assertEqual(next(z), (3, 4))
        with self.assertRaises(StopIteration):
            next(z)

    def testFirstPathfindWithNewPathfinder(self):
        # Find the path from A -> C
        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 3)

        self.assertEqual(path, [2, 3])

    def testPathfinderCaching(self):
        newStyleMap = CreateSimpleNewStyleMapData()
        cacheDict = defaultdict(lambda: PathfinderCacheEntry(None, evePathfinder.core.NewPathfinderCache(newStyleMap)))

        def GetCachedEntry(stateInterface, fromID):
            return cacheDict[(stateInterface.GetRouteType(), fromID)]

        core = evePathfinder.core.EvePathfinderCore(newStyleMap)
        core.SetGetCachedEntryMethod(GetCachedEntry)

        path = core.GetPathBetween(self.stateInterface, self.fromID, 3)
        path = core.GetPathBetween(self.stateInterface, self.fromID, 6)

        self.assertEqual(core.newPathfinderExecutionCount, 1)

    def testPathfindRefreshWhenMovingCurrentLocation(self):
        # Find the path from A -> C
        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 3)
        self.assertEqual(path, [2, 3])

        fromID = 6
        # Find the path from E -> C
        path = self.core.GetPathBetween(self.stateInterface, fromID, 3)
        self.assertEqual(path, [6, 5, 3])

        self.assertEqual(self.core.newPathfinderExecutionCount, 2)

    def testFirstPathfindWithNewPathfinderAndAvoidance(self):
        self.stateInterface.avoidanceList = [4]

        # Find the path from A -> E
        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 6)

        # The path should avoid 4
        self.assertEqual(path, [2, 3, 5, 6])

    def testNewPathfinderWaypointPathfinding(self):
        # Find the path from A -> E -> A
        path = self.core.GetListOfWaypointPaths(self.stateInterface, self.fromID, [2,6,2])

        self.assertEqual(path, [[2,4,6], [6,4,2]])

    def testNewPathfinderRouteTypePathfinding(self):
        # Find the path from A -> E
        self.stateInterface.routeType = ROUTE_TYPE_SAFE
        path = self.core.GetListOfWaypointPaths(self.stateInterface, self.fromID, [2,6])
        self.assertEqual(path, [[2, 3, 5, 6]])

        self.stateInterface.SetRouteType(ROUTE_TYPE_SHORTEST)
        path = self.core.GetListOfWaypointPaths(self.stateInterface, self.fromID, [2, 6])
        self.assertEqual(path, [[2, 4, 6]])

        self.assertEqual(self.core.newPathfinderExecutionCount, 2)

    def testGetJumpCountBetween(self):
        # Find the path from A -> E
        self.stateInterface.SetRouteType(ROUTE_TYPE_SAFE)

        jumpsBetween = self.core.GetJumpCountBetween(self.stateInterface, self.fromID, 6)
        self.assertEqual(jumpsBetween, 3, "Should be 2->3->5->6 = 3 jumps")

        self.stateInterface.SetRouteType(ROUTE_TYPE_SHORTEST)
        path = self.core.GetJumpCountBetween(self.stateInterface, self.fromID, 6)
        self.assertEqual(path, 2, "Should be 2 -> 4 -> 6 = 2 jumps")

    def testNewPathfinderFindsRouteToAvoidedSystem(self):
        self.stateInterface.avoidanceList = [4]
        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 4)

        self.assertEqual(path, [2, 4])

    def testNewPathfinderClearsAvoids(self):
        core = CreateSimplePathfinderCore()

        self.stateInterface.avoidanceList = [4]
        path = core.GetPathBetween(self.stateInterface, self.fromID, 6)
        self.assertEqual(path, [2, 3, 5, 6])

        self.stateInterface.avoidanceList = []

        path = core.GetPathBetween(self.stateInterface, self.fromID, 6)
        self.assertEqual(path, [2, 4, 6])

        self.assertEqual(core.newPathfinderExecutionCount, 2)

    @unittest.skipIf(not geventAvailable,"Requires gevent")
    def testReentrantPathfindingOnAvoidanceLookup(self):

        def GetPathBetweenWithAvoidance(core, avoidanceList):
            self.stateInterface.avoidanceList = avoidanceList
            return core.GetPathBetween(self.stateInterface, self.fromID, 6)


        job1 = gevent.spawn(GetPathBetweenWithAvoidance, self.core, [4])
        job2 = gevent.spawn(GetPathBetweenWithAvoidance, self.core, [3])

        gevent.joinall([job1, job2])

        self.assertEqual(job1.value, [2, 3, 5, 6], "Should avoid (4) despite it being the shortest route" )
        self.assertEqual(job2.value, [2, 4, 6])

        self.assertEqual(self.core.newPathfinderExecutionCount, 2)

    def testNewPathfinderOnDisconnectedSystems(self):
        core = CreateSimplePathfinderCore(newMapCreationFunction=CreateSimpleMapWithUnreachableSystem)
        self.assertEqual(core.GetPathBetween(self.stateInterface, self.fromID, 5), [], "(5) is unreachable from (2)")

    def testWaypointPathWithAvoidedDestinations(self):
        self.stateInterface.avoidanceList = [6]

        self.assertEqual(
            self.core.GetListOfWaypointPaths(self.stateInterface, self.fromID, [2, 6, 2]),
            [[2, 4, 6], [6, 4, 2]],
            "Because (6) is in the waypoint list, we go through it despite it being avoided"
        )

    def testPathWithAvoidedDestinationsAndCaching(self):
        self.stateInterface.avoidanceList = [4, ]

        self.assertEqual(self.core.GetPathBetween(self.stateInterface, self.fromID, 4), [2, 4], "Because the destination is (4) it should be able to go to (4)" )
        self.assertEqual(self.core.GetPathBetween(self.stateInterface, self.fromID, 6), [2, 3, 5, 6], "Despite the earlier result and any caching, this should not go through (4)" )

    def testJumpCountBetweenConnectedSystems(self):
        self.assertEqual(self.core.GetJumpCountBetween(self.stateInterface, 2, 5), 2, "Should be 2 -> 3 -> 5 = 2 jumps")

    def testJumpCountBetweenDisconnectedSystems(self):
        core = CreateSimplePathfinderCore(newMapCreationFunction=CreateSimpleMapWithUnreachableSystem)

        self.assertEqual(core.GetJumpCountBetween(self.stateInterface, 2, 5 ), sys.maxsize, "You can't travel from A(2) to D(5)")

    def testSystemsWithinOneJump(self):
        systems = self.core.GetSystemsWithinJumpRange(self.stateInterface, 2, 1, 2)
        expectedSystems = {1: [3, 4]}

        for jumpCount in expectedSystems:
            self.assertSetEqual(
                set(systems[jumpCount]),
                set(expectedSystems[jumpCount]),
                "Travelling from A(2) in " + str(jumpCount) + " jumps should yield " + str(expectedSystems[jumpCount])
            )

    def testSystemsWithinTwoJumps(self):
        core = CreateSimplePathfinderCore()
        systems = core.GetSystemsWithinJumpRange(self.stateInterface, 2, 1, 3)
        expectedSystems = {1: [3, 4], 2: [5, 6] }

        for jumpCount in expectedSystems:
            self.assertSetEqual(
                set(systems[jumpCount]),
                set(expectedSystems[jumpCount]),
                "Travelling from A(2) in " + str(jumpCount) + " jumps should yield " + str(expectedSystems[jumpCount] )
            )

    def testGetJumpCountsBetweenSystemPairs(self):
        inputSystemPairs = [(2, 6), (4, 3), (2, 5)]

        expectedResult = {
            (2, 6): 2,
            (2, 5): 2,
            (4, 3): 2,
        }

        result = self.core.GetJumpCountsBetweenSystemPairs(self.stateInterface, inputSystemPairs)

        self.assertEqual(result, expectedResult)

    def testThatPathIsFoundToAvoidedSystemOnFirstPathfind(self):
        core = CreateSimplePathfinderCore()

        self.stateInterface.avoidanceList = [ 4, ]
        self.stateInterface.routeType = "safe"

        path = core.GetPathBetween(self.stateInterface, self.fromID, 4)
        self.assertEqual(path, [2, 4])

    def testThatPathIsFoundToAvoidedSystemAfterPathIsFoundToNonAvoidedSystem(self):
        self.stateInterface.avoidanceList = [4, ]
        self.stateInterface.routeType = "safe"

        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 3)
        self.assertEqual(path, [2, 3])

        path = self.core.GetPathBetween(self.stateInterface, self.fromID, 4)
        self.assertEqual(path, [2, 4])

    def testThatPathIsFoundToAnAvoidedSystem(self):
        core = CreateSimplePathfinderCore(newMapCreationFunction=CreateTShapedSolarSystemMap)

        self.stateInterface.avoidanceList = [3]
        path = core.GetPathBetween(self.stateInterface, self.fromID, 3)

        self.assertEqual(path, [2, 3])

    def testThatPathIsNotFoundToSystemThroughAnAvoidedSystem(self):
        core = CreateSimplePathfinderCore(
            newMapCreationFunction=CreateTShapedSolarSystemMap
        )

        self.stateInterface.avoidanceList = [3]
        path = core.GetPathBetween(self.stateInterface, self.fromID, 4)

        self.assertEqual(path, [])

    def testThatAvoidedSystemIsInGoalSystems(self):
        self.assertTrue(self.core.GoalSystemsContainAnyAvoidedSystem([1, 2, 3], [2, 6, 7]))

    def testThatNoAvoidedSystemsAreInGoalSystems(self):
        self.assertFalse(self.core.GoalSystemsContainAnyAvoidedSystem([1, 2, 3], [5, 6, 7]))

    def testSetGetCachedEntryMethod(self):
        getCacheEntryMethod = mock.Mock(return_value=PathfinderCacheEntry(None, evePathfinder.core.NewPathfinderCache(self.newStyleMap)))
        self.core.SetGetCachedEntryMethod(getCacheEntryMethod)
        self.core.GetJumpCountBetween(self.stateInterface, self.fromID, 4)
        getCacheEntryMethod.assert_called_once_with(self.stateInterface, self.fromID)

if __name__ == '__main__':
    import sys
    logging.basicConfig(stream=sys.stdout, level=logging.INFO)

    suite = unittest.TestLoader().loadTestsFromTestCase(testPathfinderCore)
    unittest.TextTestRunner(stream=sys.stdout, verbosity=2).run(suite)
