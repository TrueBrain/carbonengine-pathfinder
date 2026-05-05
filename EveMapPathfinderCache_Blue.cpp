// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveMapPathfinderCache.h"

BLUE_DEFINE(EveMapPathfinderCache);

const Be::ClassInfo* EveMapPathfinderCache::ExposeToBlue()
{
	EXPOSURE_BEGIN( EveMapPathfinderCache, 
		"This cache contains all the working data and output for path finding "
		"this includes the open list (the best candidates to explore next) and "
		"the closed list (the explored nodes and their solutions). "
		"It is the only thing modified by the act of running the pathfinder.")

		MAP_METHOD_AND_WRAP( "ClearCache", ClearCache, "Clears the cache")
		MAP_METHOD_AND_WRAP( "GetSolutionSystem", GetSolutionSystem, "" )

		MAP_METHOD_AND_WRAP( "Initialize", Initialize, "" )
		MAP_METHOD_AND_WRAP( "Clear", ClearCache, "" )

		MAP_METHOD_AND_WRAP( "GetSystemsWithinJumpCount", GetSystemsWithinJumpCount, "" )
		MAP_METHOD_AND_WRAP( "GetRouteTo", GetRouteTo, "" )
		MAP_METHOD_AND_WRAP( "GetJumpCountTo", GetJumpCountTo, "" )

	EXPOSURE_END()
}