// Copyright © 2014 CCP ehf.

#pragma once
#ifndef PResultBeResult_h
#define PResultBeResult_h

enum PRESULT {
	PRESULT_OK,
	PRESULT_NO_MAP,
	PRESULT_NO_CACHE,
	PRESULT_NO_GOAL,
	PRESULT_MAP_FINALIZED,
	PRESULT_CACHE_NOT_INITIALIZED,
	PRESULT_INVALID_ID,
	PRESULT_NO_SOLUTION_FOUND,

	// ----
	PRESULT_COUNT,
};

static const char* S_ERROR_STRINGS[PRESULT_COUNT]  = {
	"ok",
	"Failed: Map must be specified",
	"Failed: Cache must be specified",
	"Failed: Goal must be specified",
	"Failed: Map has not been finalized",
	"Failed: Cache has not been initialized",
	"Failed: Specified ID was not found",
	"Failed: Did not find a solution"
};

namespace Be
{
	template<>
	struct Result<PRESULT>
	{
		Result() : value( PRESULT_OK ) {}
		Result( PRESULT b ) : value( b ) {}
		PRESULT value;
	};
}

template <>
inline bool BeIsSuccess<PRESULT>( const Be::Result<PRESULT>& result )
{
	return result.value == PRESULT_OK;
}

template <>
inline const char* BeGetErrorMessage<PRESULT>( const Be::Result<PRESULT>& result )
{
	return S_ERROR_STRINGS[result.value];
}

template <>
inline PyObject* BeGetException<PRESULT>( const Be::Result<PRESULT>& result )
{
	return PyExc_RuntimeError;
}



#endif