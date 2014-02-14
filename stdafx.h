#pragma once

#include "BlueExposure/include/BlueExposure.h"
#include "PResultBeResult.h"
#include "EveMapNodes.h"

#define CHECK_RETURN_MAP(m) \
if( m == nullptr )\
{\
	return Be::Result<PRESULT>( PRESULT_NO_MAP );\
}

#define CHECK_RETURN_SYSTEM(s) \
if( s == nullptr )\
{\
	return Be::Result<PRESULT>( PRESULT_INVALID_ID );\
}

#define CHECK_RETURN_GET_SYSTEM(s) \
if( !s )\
{\
	return Be::Result<PRESULT>( PRESULT_INVALID_ID );\
}