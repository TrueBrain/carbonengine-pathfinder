import hashlib

from evePathfinder.pathfinderconst import ROUTE_TYPE_SHORTEST
from evePathfinder.pathfinderconst import DEFAULT_SECURITY_PENALTY_VALUE


class ServerPathfinderInterface(object):
    """
    This class defines the interface from the pathfinder to the server implementation
    """
    def __init__(self):
        self.routeType = ROUTE_TYPE_SHORTEST

    def GetSecurityPenalty(self):
        # we the same base default as client pathfinder does
        return DEFAULT_SECURITY_PENALTY_VALUE

    def GetAvoidanceList(self):
        return []

    def SetRouteType(self, routeType):
        self.routeType = routeType

    def GetRouteType(self):
        return self.routeType

    def GetCurrentStateHash(self, fromSolarSystemID):
        m = hashlib.md5()
        m.update(str(fromSolarSystemID))
        m.update(self.GetRouteType())
        return m.hexdigest()
