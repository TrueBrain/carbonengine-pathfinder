// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveMap.h"

BLUE_DEFINE(EveMap);

const Be::ClassInfo* EveMap::ExposeToBlue()
{
	EXPOSURE_BEGIN(EveMap, "The EveMap represents the map of the Eve universe, including regions, constellations, systems and jumps")

		MAP_METHOD_AND_WRAP("Finalize", FinalizeMap, "Prepare the map to be used. Must be called before using the map to find a path.")

		MAP_METHOD_AND_WRAP("CreateRegion", CreateRegion, "")
		MAP_METHOD_AND_WRAP("CreateConstellation", CreateConstellation, "")
		MAP_METHOD_AND_WRAP("CreateSolarSystem", CreateSystem, "")
		MAP_METHOD_AND_WRAP("SetSolarSystemSecurity", SetSolarSystemSecurity, "")

		MAP_METHOD_AND_WRAP("AddJump", AddJump, "")
	EXPOSURE_END()
}