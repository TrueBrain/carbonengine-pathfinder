"""
These are common constants used by the evePathfinder modules
"""
import math
import sys

# pathfinder route types
ROUTE_TYPE_SAFE = "safe"  # sec level interval [0.45, 1.0]
ROUTE_TYPE_UNSAFE = "unsafe"  # sec level interval [0.0, 0.45]
ROUTE_TYPE_UNSAFE_AND_NULL = "unsafe + zerosec"  # sec level interval [-1.0, 0.45]
ROUTE_TYPE_SHORTEST = "shortest"  # sec level interval [-1.0, 1.0]

# pathfinder heuristic defaults
SECURITY_PENALTY_FACTOR = 0.15
DEFAULT_SECURITY_PENALTY = 50.0

DEFAULT_SECURITY_PENALTY_VALUE = math.exp(SECURITY_PENALTY_FACTOR * DEFAULT_SECURITY_PENALTY)

UNREACHABLE_JUMP_COUNT = sys.maxsize
