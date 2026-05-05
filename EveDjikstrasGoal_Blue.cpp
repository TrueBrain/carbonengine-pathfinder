// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveDjikstrasGoal.h"

BLUE_DEFINE(EveDjikstrasGoal);

const Be::ClassInfo* EveDjikstrasGoal::ExposeToBlue()
{
	EXPOSURE_BEGIN( EveDjikstrasGoal, "Implements a simple goal for testing algorithmic correctness")
	EXPOSURE_END()
}