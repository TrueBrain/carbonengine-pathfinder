#include "stdafx.h"
#include "EveStandardFloodfillGoal.h"
#include "EveMap.h"

BLUE_DEFINE(EveStandardFloodFillGoal);

const Be::ClassInfo* EveStandardFloodFillGoal::ExposeToBlue()
{
	EXPOSURE_BEGIN( EveStandardFloodFillGoal, 
		   "EveStandardFloodFillGoal represents the standard path-finding behavior "
		   "based off the historical implementation that was done in destiny.")

		MAP_METHOD_AND_WRAP( "AvoidSystemsOutsideSecurityLimits", AvoidSystemsOutsideSecurityLimits, "Sets the pathfinder to avoidance mode outside of the specified security ranges" )
		MAP_METHOD_AND_WRAP( "DoNotVisitSystemsOutsideSecurityLimits", DoNotVisitSystemsOutsideSecurityLimits, "Sets the pathfinder to not visit systems outside the specified ranges" )
		MAP_METHOD_AND_WRAP( "IgnoreSecurityLimits", IgnoreSecurityLimits, "Sets the pathfinder to ignore security status")
		
		MAP_METHOD_AND_WRAP( "AddOrigin", AddOrigin, "" )
		MAP_METHOD_AND_WRAP( "ClearOrigins", ClearOrigins, "")
		MAP_METHOD_AND_WRAP( "AddAvoidSystem", AddAvoidSystem, "")
		MAP_METHOD_AND_WRAP( "ClearAvoidSystems", ClearAvoidSystems, "")
		MAP_METHOD_AND_WRAP( "ClearGoalSystems", ClearGoalSystems, "")
		MAP_METHOD_AND_WRAP( "AddGoalSystem", AddGoalSystem, "")

	EXPOSURE_END()
}