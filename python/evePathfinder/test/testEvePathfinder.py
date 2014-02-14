import os
import sys
import unittest
import setupenv
import pyEvePathfinder

def PenultimateSystemInRoute(r):
    return r[-2]


def GetLastSystemInRouteFn(map, cache):
    return lambda s: PenultimateSystemInRoute(cache.GetRouteTo(map, s))


def CreateStandardBidirectionalMap1():
    '''
    Creates a map like this:
           (0.3)
     <-----> C <-----> 
    A (1.0)            E (1.0)
     <--> B <--> D <->
        (0.8)  (0.45)

    X (1.0) [disconnected system]

    Security ratings in parentheses
    '''
    m = pyEvePathfinder.EveMap(7, 10)

    REGION = 0
    m.CreateRegion( REGION )

    CONSTELLATION = 1
    m.CreateConstellation( CONSTELLATION, REGION )

    A = 2
    B = 3
    C = 4
    D = 5
    E = 6
    X = 7

    m.CreateSolarSystem(A, CONSTELLATION, 1.0)
    m.CreateSolarSystem(B, CONSTELLATION, 0.8)
    m.CreateSolarSystem(C, CONSTELLATION, 0.3)
    m.CreateSolarSystem(D, CONSTELLATION, 0.45)
    m.CreateSolarSystem(E, CONSTELLATION, 1.0)

    m.CreateSolarSystem(X, CONSTELLATION, 1.0)

    m.AddJump(A, C, 8)
    m.AddJump(C, A, 9)

    m.AddJump(A, B, 10)
    m.AddJump(B, A, 11)

    m.AddJump(B, D, 12)
    m.AddJump(D, B, 13)

    m.AddJump(C, E, 14)
    m.AddJump(E, C, 15)

    m.AddJump(D, E, 16)
    m.AddJump(E, D, 17)

    m.Finalize()

    c = pyEvePathfinder.EveMapPathfinderCache()
    c.Initialize(m)

    return m, c, [A, B, C, D, E, X]


class testEvePathfinder(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        """
        Prevent tests from running under ExeFile
        """
        try:
            import blue

            raise unittest.SkipTest("Tests not intended to run under blue")
        except ImportError:
            pass

    def testRegionCreate(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)

        m.Finalize()

        # Cant change the map after finalization
        self.assertRaises(lambda: m.CreateRegion(1))

    def testConstellationCreate(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)

        # Region doesn't exist
        self.assertRaises(lambda: m.CreateConstellation(5, 4))

        m.Finalize()

        # Cant change the map after finalization
        self.assertRaises(lambda: m.CreateConstellation(2, 0))

    def testSolarSystemCreation(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)

        m.CreateSolarSystem(2, 1, 1.0)

        # Constellation doesn't exist
        self.assertRaises(lambda: m.CreateSolarSystem(5, 4, 0.5))

        m.Finalize()

        # Cant change the map after finalization
        self.assertRaises(lambda: m.CreateSolarSystem(3, 1, 0.5))

    def testAddJumps(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)
        m.CreateSolarSystem(2, 1, 1.0)
        m.CreateSolarSystem(3, 1, 1.0)

        m.AddJump(2, 3, 4)

        m.Finalize()

        # Cant change the map after finalization
        self.assertRaises(lambda: m.AddJump(2, 3, 0.5))

    def testAddNoneJumpsRaises(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)
        m.CreateSolarSystem(2, 1, 1.0)
        self.assertRaises(lambda: m.AddJump(None, 2, 0.5))

    def testCacheInitializeWithNoneRaises(self):
        c = pyEvePathfinder.EveMapPathfinderCache()
        self.assertRaises(lambda: c.Initialize(None))

    def testSimplePathfindOverOneJump(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)
        m.CreateSolarSystem(2, 1, 1.0)
        m.CreateSolarSystem(3, 1, 1.0)
        m.AddJump(2, 3, 4)

        m.Finalize()

        c = pyEvePathfinder.EveMapPathfinderCache()
        c.Initialize(m)

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, 2)

        pyEvePathfinder.FindRoute(m, g, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        self.assertRaises(lambda: lastSystemTo(3))
        self.assertEquals(lastSystemTo(3), 2)

    def testOriginAddWithNoneRaises(self):
        g = pyEvePathfinder.EveStandardFloodFillGoal()
        self.assertRaises(lambda: g.AddOrigin(None))

    def testPathfindingWithNoneValuesRaises(self):
        m = pyEvePathfinder.EveMap(1, 1)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)
        m.CreateSolarSystem(2, 1, 1.0)
        m.CreateSolarSystem(3, 1, 1.0)
        m.AddJump(2, 3, 4)

        m.Finalize()

        c = pyEvePathfinder.EveMapPathfinderCache()
        c.Initialize(m)

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, 2)

        self.assertRaises(lambda: pyEvePathfinder.FindRoute(None, g, c))
        self.assertRaises(lambda: pyEvePathfinder.FindRoute(m, None, c))
        self.assertRaises(lambda: pyEvePathfinder.FindRoute(m, g, None))

    def testSimplePathfindOverLinearPath2(self):
        """
            (s1) --> s2 --> s3
        """
        m = pyEvePathfinder.EveMap(5, 2)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)

        s1, s2, s3 = 2, 3, 4
        m.CreateSolarSystem(s1, 1, 1.0)
        m.CreateSolarSystem(s2, 1, 1.0)
        m.CreateSolarSystem(s3, 1, 1.0)


        m.AddJump(s1, s2, 5)
        m.AddJump(s2, s3, 6)

        m.Finalize()

        c = pyEvePathfinder.EveMapPathfinderCache()
        c.Initialize(m)

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, s1)

        pyEvePathfinder.FindRoute(m, g, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        self.assertRaises(lambda: c.GetRouteTo(m, s1))
        self.assertEquals(lastSystemTo(s3), s2)
        self.assertEquals(lastSystemTo(s2), s1)
        #self.assertEquals(lastSystemTo(s3), 2)

    def testSimplePathfindOverBifurcatedPath(self):
        """
                  ---> C --->
               (A)             E
                 --> B --> D ->
        """
        m = pyEvePathfinder.EveMap(7, 4)
        m.CreateRegion(0)
        m.CreateConstellation(1, 0)

        A = 2
        B = 3
        C = 4
        D = 5
        E = 6

        m.CreateSolarSystem(A, 1, 1.0)
        m.CreateSolarSystem(B, 1, 1.0)
        m.CreateSolarSystem(C, 1, 1.0)
        m.CreateSolarSystem(D, 1, 1.0)
        m.CreateSolarSystem(E, 1, 1.0)

        m.AddJump(A, C, 7)
        m.AddJump(A, B, 8)
        m.AddJump(B, D, 9)
        m.AddJump(C, E, 10)
        m.AddJump(D, E, 11)

        m.Finalize()

        c = pyEvePathfinder.EveMapPathfinderCache()
        c.Initialize(m)

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, g, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        self.assertEquals(lastSystemTo(B), A)
        self.assertEquals(lastSystemTo(C), A)
        self.assertEquals(lastSystemTo(E), C)
        self.assertEquals(lastSystemTo(D), B)

    def testBiDirectionalPathfind(self):
        """
            Tests pathfinding with biderectional jumps set up in
            an arbitrary order.

                  <-----> C <----->
               (A)                  E
                  <--> B <--> D <->
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, g, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        self.assertEquals(lastSystemTo(B), A)
        self.assertEquals(lastSystemTo(C), A)

        self.assertEquals(lastSystemTo(E), C)
        self.assertEquals(lastSystemTo(D), B)

        self.assertEquals(c.GetRouteTo(m, D), [A, B, D])

    def testSequentialPathfinding(self):
        """
            First run a test on this:

                  <-----> C <----->
               (A)                  E
                  <--> B <--> D <->

            Next run a test on:

                  <-----> C <----->
                A                  (E)
                  <--> B <--> D <->
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        g = pyEvePathfinder.EveStandardFloodFillGoal()
        g.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, g, c)

        g.ClearOrigins()
        c.Clear()
        g.AddOrigin(m, E)

        pyEvePathfinder.FindRoute(m, g, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        self.assertEquals(lastSystemTo(C), E)
        self.assertEquals(lastSystemTo(A), C)

        self.assertEquals(lastSystemTo(D), E)
        self.assertEquals(lastSystemTo(B), D)

    def testPathfindingHardLimits(self):
        """
                   (0.3)
             <-----> C <----->
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()

        goal.AddOrigin(m, A)
        goal.DoNotVisitSystemsOutsideSecurityLimits(0.4, 1.0)

        pyEvePathfinder.FindRoute(m, goal, c)
        lastSystemTo = GetLastSystemInRouteFn(m, c)

        # cannot visit C, outside of allowed security rating
        self.assertRaises(lambda: c.GetRouteTo(m, C))

        self.assertEquals(lastSystemTo(B), A)
        self.assertEquals(lastSystemTo(D), B)
        self.assertEquals(lastSystemTo(E), D)

    def testPathfindingSoftLimits(self):
        """
        Check that with soft pathfinding limits, C is traversed, but not
        used to travel to E

                   (0.3)
             <-----> C <----->
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()

        goal.AddOrigin(m, A)
        goal.AvoidSystemsOutsideSecurityLimits(0.4, 1.0, 3.0)

        pyEvePathfinder.FindRoute(m, goal, c)

        lastSystemTo = GetLastSystemInRouteFn(m, c)
        # C has a penalty of 3.0. Will be visited from A, but not traversed
        # to E
        self.assertEquals(lastSystemTo(C), A)

        self.assertEquals(lastSystemTo(B), A)
        self.assertEquals(lastSystemTo(D), B)
        self.assertEquals(lastSystemTo(E), D)

    def testStandardGoalWithNoneOriginRaises(self):
        goal = pyEvePathfinder.EveStandardFloodFillGoal()
        self.assertRaises(lambda: goal.AddOrigin(None))
        self.assertRaises(lambda: goal.AddAvoidSystem(None))

    def testPathfindingWithGoal(self):
        """
        Check that with soft pathfinding limits, C is traversed, but not
        used to travel to E

                   (0.3)
             <-----> C <----->
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()

        goal.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, goal, c)
        self.assertEquals(c.GetRouteTo(m, D), [2, 3, 5])

    def testAvoidSystems(self):
        """
        Checks that systems flagged for avoidance are not used in the path
        """
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()

        goal.AddOrigin(m, A)
        goal.AddAvoidSystem(m, B)

        pyEvePathfinder.FindRoute(m, goal, c)

        self.assertEquals(c.GetRouteTo(m, D), [2, 4, 6, 5])
        self.assertEquals(c.GetJumpCountTo(m, D), 3)

    def testDisconnectedSystem(self):
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()

        goal.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, goal, c)

        self.assertEquals(c.GetRouteTo(m, X), [])
        self.assertEquals(c.GetJumpCountTo(m, X), -1)

    def testGetSystemsWithinASingleJump(self):
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()
        goal.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, goal, c)

        systemDistances = c.GetSystemsWithinJumpCount(m, 1, 2)
        self.assertEquals(systemDistances, {3: 1, 4: 1})

    def testGetSystemsWithinTwoJumps(self):
        m, c, (A, B, C, D, E, X) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.EveStandardFloodFillGoal()
        goal.AddOrigin(m, A)

        pyEvePathfinder.FindRoute(m, goal, c)

        systemJumpCounts = c.GetSystemsWithinJumpCount(m, 1, 3)
        self.assertEquals(systemJumpCounts,
                          {3: 1, 4: 1, 5: 2, 6: 2})


if __name__ == '__main__':
    import sys

    suite = unittest.TestLoader().loadTestsFromTestCase(testEvePathfinder)
    unittest.TextTestRunner(stream=sys.stderr, verbosity=2).run(suite)