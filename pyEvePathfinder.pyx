#from libcpp.map cimport map
from cython.operator cimport dereference as deref
from cython.operator cimport preincrement  as preincrement 
from cython cimport address as address_of
from libc.stdlib cimport malloc, free

from libcpp cimport bool
ctypedef unsigned int uint32_t

cdef extern from "EveMapNodes.h":
    struct EveMapClosedListNodeID:
        pass

    struct EveMapNodeID:
        pass

    ctypedef EveMapNode* const_EveMapNode_ptr "EveMapNode const *"

    struct ClosedListNode:
        bool m_isOrigin
        bool m_visited
        float m_costToNode
        EveMapNodeID m_mapNodeID
        EveMapClosedListNodeID m_lastClosedListNode

    struct EveMapNode:
        uint32_t m_itemID
        EveMapNodeID m_parent
        EveMapClosedListNodeID m_closedListID
        float m_trueSecRating

cdef extern from "include/IEvePathfinderGoal.h":
    cdef cppclass IEvePathfinderGoal:
        pass

cdef class Goal:
    cdef IEvePathfinderGoal* GetGoal(self):
        pass

cdef extern from "EveMap.h":
    cdef cppclass EveMap:
        EveMap( uint32_t, uint32_t )
        bool CreateRegion( uint32_t regionID, EveMapNodeID* outNode )
        bool CreateConstellation( uint32_t constellationID, uint32_t regionID, EveMapNodeID* outNode )
        bool CreateSystem( uint32_t solarSystemID, uint32_t constellationID, float security, EveMapNodeID* outNodeL )
        bool AddJump( EveMapNodeID fromID, EveMapNodeID toID, uint32_t jumpGateID )
        bool AddJump( uint32_t fromSystemID, uint32_t toSystemID, uint32_t jumpGateID )
        void FinalizeMap()
        bool GetSolarSystemID( uint32_t ssID, EveMapNodeID& ss )
        EveMapNode* GetSolarSystem2( EveMapNodeID s )

cdef extern from "EveMapPathfinderCache.h":
    cdef cppclass EveMapPathfinderCache:
        void Initialize( EveMap& mapdata, uint32_t openListCount )
        void ClearCache()
        ClosedListNode& GetCurrentPath( EveMapClosedListNodeID& nodeID )
        bool IsComplete()
        EveMapNodeID GetSolutionSystemID()
        

cdef class MapNodeID:
    '''
    This is an opaque handle to a region, constellation or system in a Map
    Using this for lookups is much more efficient than doing a lookup by itemIDs
    '''
    cdef EveMapNodeID* m_node

    cdef SetMapNodeID( self, EveMapNodeID* n ):
        if self.m_node:
            free( self.m_node )

        self.m_node = n

    def __richcmp__(MapNodeID r, MapNodeID l, int op):
        if r.m_node and l.m_node:
            if op == 2:
                return deref(r.m_node) == deref(l.m_node)
            elif op == 3:
                return not(deref(r.m_node) == deref(l.m_node))
            else:
                raise NotImplemented("Only equality tests are valid")
        else:
            return False

    def __dealloc__(self):
        if self.m_node:
            free( self.m_node )

cdef class Map:
    '''
    This represents an Eve Map, including regions, constellations, systems and jumps
    used for pathfinding. 
    '''
    cdef EveMap* m_mapPtr
    cdef bool m_isFinalized

    def __cinit__( self, int nodeCount, int jumpCount ):
        self.m_mapPtr = new EveMap( nodeCount, jumpCount )
        self.m_isFinalized = False

    def __dealloc__( self ):
        del self.m_mapPtr

    def CreateRegion( self, int regionID ):
        '''
        Creates a region with a given itemID

        Arguments:
            * regionID (an itemID)
        '''
        if self.m_isFinalized:
            raise Exception("Cannot change a finalized Map")

        self.m_mapPtr.CreateRegion( regionID, NULL )

    def CreateConstellation( self, int constellationID, int regionID ):
        '''
        Creates a constellation with a given itemID and links it to a regionID

        Arguments:
            * constellationID (an itemID)
            * regionID (an itemID)
        '''
        if self.m_isFinalized:
            raise Exception("Cannot change a finalized Map")

        self.m_mapPtr.CreateConstellation( constellationID, regionID, NULL )

    def CreateSolarSystem( self, int solarSystemID, int constellationID, float sec ):
        '''
        Creates a solarSystem in the map with a given security rating.
        A MapNodeID handle to the system is returned if succesfuly created, None returned
        on failure (due to a failure to find the constellationID)

        Arguments:
            * solarSystemID (an itemID)
            * constellationID (an itemID)
            * security rating (the true security rating of the system)
        '''
        if self.m_isFinalized:
            raise Exception("Cannot change a finalized Map")

        cdef EveMapNodeID* n = <EveMapNodeID*>malloc( sizeof(EveMapNodeID) )
        if self.m_mapPtr.CreateSystem( solarSystemID, constellationID, sec, n ):
            m = MapNodeID()
            m.SetMapNodeID( n )
            return m
        free( n )
        return None

    def AddJump( self, MapNodeID fromID, MapNodeID toID, int jumpGateID ):
        '''
        Adds a jump between two systems. Adding all jumps for a system in one block
        is significantly more efficient.

        Arguments:
            * (MapNodeID) from
            * (MapNodeID) to
            * jumpGateID (the itemID of the jumpgate)
        '''
        if self.m_isFinalized:
            raise Exception("Cannot change a finalized Map")

        return self.m_mapPtr.AddJump( deref(fromID.m_node), deref(toID.m_node), jumpGateID )

    def Finalize( self ):
        '''
        Prepare the map for use. Must be called before you can initialize a Cache with this map.
        '''
        self.m_isFinalized = True
        self.m_mapPtr.FinalizeMap()

cdef class Cache:
    '''
    The pathfinder cache contains all temporary memory for pathfinding, as well as the solution.
    '''
    cdef EveMapPathfinderCache* m_cache
    cdef bool m_isInitialized

    def __cinit__(self):
        self.m_cache = new EveMapPathfinderCache()
        self.m_isInitialized = False

    def __dealloc__(self):
        del self.m_cache

    def Initialize( self, Map m ):
        '''
        Initialized a cache for use with a particular map.
        '''
        if not m.m_isFinalized:
            raise Exception("Cannot initialize a Cache with an un-finalized Map")

        self.m_cache.Initialize( deref(m.m_mapPtr), 0 )
        self.m_isInitialized = True

    def Clear(self):
        '''
        Resets the cache, clearing any temporary data from it.
        '''
        self.m_cache.ClearCache()

    def GetLastSystemIDInRouteTo( self, Map m, MapNodeID n ):
        '''
        Returns the itemID of the system from which you jump to get to this system
        '''
        if not self.m_isInitialized:
            raise Exception("Cannot query without Initializing the cache first")

        cdef EveMapNode* node = m.m_mapPtr.GetSolarSystem2( deref(n.m_node) )
        cdef ClosedListNode cl = self.m_cache.GetCurrentPath( node.m_closedListID )
    
        if not cl.m_visited:
            return None

        cdef ClosedListNode cl2 = self.m_cache.GetCurrentPath( cl.m_lastClosedListNode )
        return m.m_mapPtr.GetSolarSystem2( cl2.m_mapNodeID ).m_itemID


    def GetLastSystemNodeIDInRouteTo( self, Map m, MapNodeID n ):
        if not self.m_isInitialized:
            raise Exception("Cannot query without Initializing the cache first")

        cdef EveMapNode* node = m.m_mapPtr.GetSolarSystem2( deref(n.m_node) )
        cdef ClosedListNode cl = self.m_cache.GetCurrentPath( node.m_closedListID )
    
        if not cl.m_visited:
            return None

        cdef ClosedListNode cl2 = self.m_cache.GetCurrentPath( cl.m_lastClosedListNode )
        n = MapNodeID()
        n.SetMapNodeID( address_of(cl2.m_mapNodeID) )
        return n

    def GetDistanceTo( self, Map m, MapNodeID n ):
        '''
        Looks up a map node in the cache, and returns the evaluated distance to it
        This distance is NOT the number of jumps.

        Returns None if the system was not reached by the pathfinder.
        '''
        if not self.m_isInitialized:
            raise Exception("Cannot query without Initializing the cache first")

        cdef EveMapNode* node = m.m_mapPtr.GetSolarSystem2( deref(n.m_node) )
        cdef ClosedListNode cl = self.m_cache.GetCurrentPath( node.m_closedListID )
    
        if not cl.m_visited:
            return None

        cdef ClosedListNode cl2 = self.m_cache.GetCurrentPath( cl.m_lastClosedListNode )
        return cl.m_costToNode

    def GetSolutionRoute( self, Map m ):
        '''

        '''
        if not self.m_cache.IsComplete():
            raise Exception("No route to solution found")

        cdef EveMapNodeID solution = self.m_cache.GetSolutionSystemID()

        cdef EveMapNode* node = m.m_mapPtr.GetSolarSystem2( solution )
        cdef ClosedListNode* cl = address_of(self.m_cache.GetCurrentPath( node.m_closedListID ))

        while not deref(cl).m_isOrigin:
            yield deref( m.m_mapPtr.GetSolarSystem2( deref(cl).m_mapNodeID ) ).m_itemID
            cl = address_of(self.m_cache.GetCurrentPath( deref(cl).m_lastClosedListNode ))

        yield deref( m.m_mapPtr.GetSolarSystem2( deref(cl).m_mapNodeID ) ).m_itemID

    def GetRouteTo( self, Map m, MapNodeID i ):
        '''

        '''
        cdef EveMapNode* node = m.m_mapPtr.GetSolarSystem2( deref(i.m_node) )
        cdef ClosedListNode* cl = address_of(self.m_cache.GetCurrentPath( node.m_closedListID ))

        while not deref(cl).m_isOrigin:
            yield deref( m.m_mapPtr.GetSolarSystem2( deref(cl).m_mapNodeID ) ).m_itemID
            cl = address_of(self.m_cache.GetCurrentPath( deref(cl).m_lastClosedListNode ))

        yield deref( m.m_mapPtr.GetSolarSystem2( deref(cl).m_mapNodeID ) ).m_itemID


# -----------------------------------------------------------------------------
cdef extern from "EveStandardFloodfillGoal.h":
    cdef cppclass EveStandardFloodFillGoal(IEvePathfinderGoal):
        void AvoidSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity, float securityPenalty )
        void DoNotVisitSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity )
        void IgnoreSecurityLimits()
        void AddOrigin( EveMapNodeID origin )
        void ClearOrigins()
        void AddAvoidSystem( EveMapNodeID s )
        void ClearAvoidSystems()

cdef class StandardGoal(Goal):
    '''
    Reimplements all logic from the pathfinding behaviour in destiny
    including security status avoidance (and flattening of security status ranges)
    TODO: include explicit system avoidance flags
    '''
    cdef EveStandardFloodFillGoal* m_goal

    cdef IEvePathfinderGoal* GetGoal(self):
        return self.m_goal

    def __cinit__(self):
        self.m_goal = new EveStandardFloodFillGoal()

    def __dealloc__(self):
        del self.m_goal

    def AddOrigin( self, MapNodeID o ):
        '''
        Adds a system as an origin (can have many). Use ClearOrigins to reset.

        Arguments:
            * MapNodeID originSystemID
        '''
        self.m_goal.AddOrigin( deref(o.m_node) )

    def ClearOrigins(self):
        '''
        Removes all systems from being used as points of origin in the next pathfind
        '''
        self.m_goal.ClearOrigins()

    def IgnoreSecurityLimits(self):
        '''
        Causes the pathfinder to treat all systems as equals
        '''
        self.m_goal.IgnoreSecurityLimits()

    def DoNotVisitSystemsOutsideSecurityLimits( self, float minSecurity, float maxSecurity ):
        '''
        Prevents the pathfinder from visiting systems outside the
        security limits
        '''
        self.m_goal.DoNotVisitSystemsOutsideSecurityLimits( minSecurity, maxSecurity )

    def AvoidSystemsOutsideSecurityLimits( self, float minSecurity, float maxSecurity, float securityPenalty = 3.0 ):
        '''
        Causes the pathfinder to assign a penalty to visiting systems outside of the
        security status range. The default penalty is 3.0, the penalty is doubled for systems
        with a security rating < 0.0 if the minimum security rating is higher
        '''
        self.m_goal.AvoidSystemsOutsideSecurityLimits( minSecurity, maxSecurity, securityPenalty )

    def AddAvoidSystem( self, MapNodeID o ):
        self.m_goal.AddAvoidSystem( deref(o.m_node) )

    def ClearAvoidSystems( self ):
        self.m_goal.ClearAvoidSystems()

# -----------------------------------------------------------------------------
cdef extern from "EveDjikstrasGoal.h":
    cdef cppclass EveDjikstrasGoal(IEvePathfinderGoal):
        void SetGoal( EveMap& universe, EveMapNodeID goal )
        void AddOrigin( EveMapNodeID goal )
        void ClearOrigins()

cdef class DjikstrasGoal(Goal):
    '''
    This is a simple goal mainly intended as an example and to be
    used for testing the pathfinder behaviour.

    This goal treats all paths as the same cost, but unlike FloodFillGoal
    also includes a termination condition.
    '''
    cdef EveDjikstrasGoal* m_goal

    def __cinit__(self):
        self.m_goal = new EveDjikstrasGoal()

    def __dealloc__(self):
        del self.m_goal

    cdef IEvePathfinderGoal* GetGoal(self):
        return self.m_goal

    def SetGoal( self, Map m, MapNodeID o ):
        self.m_goal.SetGoal( deref(m.m_mapPtr), deref(o.m_node) )

    def AddOrigin( self, MapNodeID o ):
        '''
        Add a system as an origin. Many systems can be origins.

        Arguments:
            * MapNodeID originSystemID
        '''
        self.m_goal.AddOrigin( deref(o.m_node) )

    def ClearOrigins( self ):
        '''
        Removes all systems from being used as points of origin in the next pathfind
        '''
        self.m_goal.ClearOrigins()

# -----------------------------------------------------------------------------
cdef extern from "EveFloodFillGoal.h":
    cdef cppclass EveFloodFillGoal(IEvePathfinderGoal):
        void AddOrigin( EveMapNodeID goal )
        void ClearOrigins()      

cdef class FloodFillGoal(Goal):
    '''
    This is a simple goal mainly intended as an example and to be used
    for testing the pathfinder behaviour.

    As the name implies, this goal causes the pathfinder to explore
    everything that it can without stopping. Any visitable system will
    be explored.
    '''
    cdef EveFloodFillGoal* m_goal

    def __cinit__(self):
        self.m_goal = new EveFloodFillGoal()

    def __dealloc__(self):
        del self.m_goal

    cdef IEvePathfinderGoal* GetGoal(self):
        return self.m_goal

    def AddOrigin( self, MapNodeID o ):
        '''
        Add a system as an origin. Many systems can be origins.

        Arguments:
            * MapNodeID originSystemID
        '''
        self.m_goal.AddOrigin( deref(o.m_node) )

    def ClearOrigins( self ):
        '''
        Removes all systems from being used as points of origin in the next pathfind
        '''
        self.m_goal.ClearOrigins()


# -----------------------------------------------------------------------------
cdef extern from "EvePathfinder.h":
    void RunPathfinder( EveMap& universeMap, IEvePathfinderGoal& goal, EveMapPathfinderCache& cache )

def FindRoute( Map m, Goal g, Cache c ):
    '''
    Runs the pathfinder on a given Map, using a given Goal, and storing results in a cache.
    The cache should be initialized to work with the given map, and cleared prior to re-use.

    Arguments:
        * Map
        * Goal
        * Cache 
    '''
    if not m.m_isFinalized:
        raise Exception("Map must be finalized")

    if not c.m_isInitialized:
        raise Exception("Cache must be initialized against the Map")

    RunPathfinder( deref(m.m_mapPtr), deref(g.GetGoal()), deref(c.m_cache) )