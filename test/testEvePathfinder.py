import os
import sys
import unittest

def AbsJoin( *paths ):
    return os.path.abspath( os.path.join( *paths ))

thisFile = __file__
branchPath = AbsJoin( thisFile, '../../../..' )
carbonCommonLib = AbsJoin( branchPath, 'carbon/common/lib' )

if __name__ == '__main__':
    sys.path.append( carbonCommonLib )

import carbon.hackDistribute
carbon.hackDistribute.GetModuleJit( 'eve', 'pyEvePathfinder' )

try:
    import blue
except ImportError:
    # not under blue
    import pyEvePathfinder

def CreateStandardBidirectionalMap1():
    '''
    Creates a map like this:
           (0.3)
     <-----> C <-----> 
    A (1.0)            E (1.0)
     <--> B <--> D <->
        (0.8)  (0.45)

    Security ratings in parentheses
    '''
    m = pyEvePathfinder.Map(7,10)
    m.CreateRegion( 0 )
    m.CreateConstellation( 1, 0 )

    A = m.CreateSolarSystem( 2, 1, 1.0 )
    B = m.CreateSolarSystem( 3, 1, 0.8 )
    C = m.CreateSolarSystem( 4, 1, 0.3 )
    D = m.CreateSolarSystem( 5, 1, 0.45 )
    E = m.CreateSolarSystem( 6, 1, 1.0 )

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

    c = pyEvePathfinder.Cache()
    c.Initialize( m )

    return m, c, [A,B,C,D,E]

class testEvePathfinder(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        '''
        Prevent tests from running under ExeFile
        '''
        try:
            import blue
            raise unittest.SkipTest("Tests not intended to run under blue")
        except ImportError:
            pass
    
    def testRegionCreate(self):
        m = pyEvePathfinder.Map(1,1)
        m.CreateRegion( 0 )

        m.Finalize()
        
        # Cant change the map after finalization
        self.assertRaises( lambda: m.CreateRegion( 1 ) )

    def testConstellationCreate(self):
        m = pyEvePathfinder.Map(1,1)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )

        # Region doesn't exist
        self.assertRaises( lambda: m.CreateConstellation( 5, 4 ) )

        m.Finalize()
        
        # Cant change the map after finalization
        self.assertRaises( lambda: m.CreateConstellation( 2, 0 ) )

    def testSolarSystemCreation(self):
        m = pyEvePathfinder.Map(1,1)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )

        self.assertIsNotNone( m.CreateSolarSystem( 2, 1, 1.0 ) )

        # Constellation doesn't exist
        self.assertRaises( lambda: m.CreateSolarSystem( 5, 4, 0.5 ) )

        m.Finalize()

        # Cant change the map after finalization
        self.assertRaises( lambda: m.CreateSolarSystem( 3, 1, 0.5 ) )

    def testAddJumps(self):
        m = pyEvePathfinder.Map(1,1)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )
        s1 = m.CreateSolarSystem( 2, 1, 1.0 )
        s2 = m.CreateSolarSystem( 3, 1, 1.0 )

        self.assertTrue( m.AddJump( s1, s2, 4 ) )

        m.Finalize()
        
        # Cant change the map after finalization
        self.assertRaises( lambda: m.AddJump( s2, s1, 0.5 ) )

    def testSimplePathfindOverOneJump(self):
        m = pyEvePathfinder.Map(1,1)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )
        s1 = m.CreateSolarSystem( 2, 1, 1.0 )
        s2 = m.CreateSolarSystem( 3, 1, 1.0 )
        m.AddJump( s1, s2, 4 )

        m.Finalize()

        c = pyEvePathfinder.Cache()
        c.Initialize( m )

        g = pyEvePathfinder.FloodFillGoal()
        g.AddOrigin( s1 )

        pyEvePathfinder.FindRoute( m, g, c )

        self.assertRaises( lambda: c.GetLastSystemIDInRouteTo( m, s1 ) )
        self.assertEquals( c.GetLastSystemIDInRouteTo( m, s2 ), 2 )


    def testSimplePathfindOverLinearPath2(self):
        '''
            (s1) --> s2 --> s3
        '''
        m = pyEvePathfinder.Map(5,2)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )

        s1 = m.CreateSolarSystem( 2, 1, 1.0 )
        s2 = m.CreateSolarSystem( 3, 1, 1.0 )
        s3 = m.CreateSolarSystem( 4, 1, 1.0 )
        
        m.AddJump( s1, s2, 5 )
        m.AddJump( s2, s3, 6 )

        m.Finalize()

        c = pyEvePathfinder.Cache()
        c.Initialize( m )

        g = pyEvePathfinder.FloodFillGoal()
        g.AddOrigin( s1 )

        pyEvePathfinder.FindRoute( m, g, c )

        self.assertRaises( lambda: c.GetLastSystemIDInRouteTo( m, s1 ) )
        self.assertEquals( c.GetLastSystemIDInRouteTo( m, s3 ), 3 )
        self.assertEquals( c.GetLastSystemIDInRouteTo( m, s2 ), 2 )

    def testSimplePathfindOverBifurcatedPath(self):
        '''
                  ---> C ---> 
               (A)             E
                 --> B --> D ->
        '''
        m = pyEvePathfinder.Map(7,4)
        m.CreateRegion( 0 )
        m.CreateConstellation( 1, 0 )

        A = m.CreateSolarSystem( 2, 1, 1.0 )
        B = m.CreateSolarSystem( 3, 1, 1.0 )
        C = m.CreateSolarSystem( 4, 1, 1.0 )
        D = m.CreateSolarSystem( 5, 1, 1.0 )
        E = m.CreateSolarSystem( 6, 1, 1.0 )

        m.AddJump( A, C, 7 )
        m.AddJump( A, B, 8 )
        m.AddJump( B, D, 9 )
        m.AddJump( C, E, 10 )
        m.AddJump( D, E, 11 )

        m.Finalize()

        c = pyEvePathfinder.Cache()
        c.Initialize( m )

        g = pyEvePathfinder.FloodFillGoal()
        g.AddOrigin( A )

        pyEvePathfinder.FindRoute( m, g, c )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, B ), A )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, C ), A )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, E ), C )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, D ), B )


    def testBiDirectionalPathfind(self):
        '''
            Tests pathfinding with biderectional jumps set up in 
            an arbitrary order.

                  <-----> C <-----> 
               (A)                  E
                  <--> B <--> D <->
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        g = pyEvePathfinder.FloodFillGoal()
        g.AddOrigin( A )

        pyEvePathfinder.FindRoute( m, g, c )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, B ), A )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, C ), A )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, E ), C )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, D ), B )

        for expected, got in zip( c.GetRouteTo( m, D ), [5,3,2] ):
            self.assertEquals( expected, got )

    def testSequentialPathfinding(self):
        '''
            First run a test on this:
            
                  <-----> C <-----> 
               (A)                  E
                  <--> B <--> D <->

            Next run a test on:

                  <-----> C <-----> 
                A                  (E)
                  <--> B <--> D <->
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        g = pyEvePathfinder.FloodFillGoal()
        g.AddOrigin( A )

        pyEvePathfinder.FindRoute( m, g, c )

        g.ClearOrigins()
        c.Clear()
        g.AddOrigin( E )

        pyEvePathfinder.FindRoute( m, g, c )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, C ), E )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, A ), C )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, D ), E )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, B ), D )

    def testPathfindingHardLimits(self):
        '''
                   (0.3)
             <-----> C <-----> 
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.StandardGoal()

        goal.AddOrigin(A)
        goal.DoNotVisitSystemsOutsideSecurityLimits( 0.4, 1.0 )

        pyEvePathfinder.FindRoute( m, goal, c )

        # cannot visit C, outside of allowed security rating
        self.assertRaises( lambda: c.GetLastSystemNodeIDInRouteTo( m, C ) )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, B ), A )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, D ), B )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, E ), D )

    def testPathfindingSoftLimits(self):
        '''
        Check that with soft pathfinding limits, C is traversed, but not
        used to travel to E

                   (0.3)
             <-----> C <-----> 
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.StandardGoal()

        goal.AddOrigin(A)
        goal.AvoidSystemsOutsideSecurityLimits( 0.4, 1.0 )

        pyEvePathfinder.FindRoute( m, goal, c )

        # C has a penalty of 3.0. Will be visited from A, but not traversed
        # to E
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, C ), A )

        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, B ), A )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, D ), B )
        self.assertEquals( c.GetLastSystemNodeIDInRouteTo( m, E ), D )


    def testPathfindingWithGoal(self):
        '''
        Check that with soft pathfinding limits, C is traversed, but not
        used to travel to E

                   (0.3)
             <-----> C <-----> 
            A (1.0)            E (1.0)
             <--> B <--> D <->
                (0.8)  (0.45)
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.DjikstrasGoal()

        goal.AddOrigin(A)
        goal.SetGoal( m, D )

        pyEvePathfinder.FindRoute( m, goal, c )

        for expected, got in zip( c.GetSolutionRoute( m ), [5,3,2] ):
            self.assertEquals( expected, got )


    def testAvoidSystems(self):
        '''
        Checks that systems flagged for avoidance are not used in the path
        '''
        m, c, (A,B,C,D,E) = CreateStandardBidirectionalMap1()

        goal = pyEvePathfinder.StandardGoal()

        goal.AddOrigin(A)
        goal.AddAvoidSystem(B)

        pyEvePathfinder.FindRoute( m, goal, c )

        for expected, got in zip( c.GetRouteTo( m, D ), [5,6,4,2] ):
            self.assertEquals( expected, got )

if __name__ == '__main__':    
    import sys
    suite = unittest.TestLoader().loadTestsFromTestCase(testEvePathfinder)
    unittest.TextTestRunner(stream = sys.stderr, verbosity = 2).run(suite)