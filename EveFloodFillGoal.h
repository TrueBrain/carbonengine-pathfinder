// Copyright © 2014 CCP ehf.

#pragma once
#ifndef EveFloodFillGoal_H
#define EveFloodFillGoal_H

#include "Include/IEvePathfinderGoal.h"

// -------------------------------------------------------------
// Description:
//   This goal implementation defines a goal which uses a fixed cost to
//   traverse any jump, and has a number of goal systems. There are no
//	 goal systems that cause termination, so it only finishes when it has
//   explored all reachable systems
//   
//   This class is mainly intended as an example and for testing.
// SeeAlso:
//   IEvePathfinderGoal
// -------------------------------------------------------------
class EveFloodFillGoal:
	public IEvePathfinderGoal
{
public:

	//////////////////////////////////////////////////////////////////////////
	// IEvePathfinderGoal 
	//////////////////////////////////////////////////////////////////////////

	// If the map node is one that we're looking for, we're done here
	virtual bool IsGoal( const EveMap& universe, EveMapNodeID node ) const;

	// Get the IDs of the neighbours of a map node that this goal can traverse to
	virtual void GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const;

	// Get the estimated distance to a goal node
	virtual float GetHeuristicEstimate( const EveMapNode& node ) const;

	// Get the cost of traversing from A to B
	virtual float GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const;

	virtual void GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const;

	//////////////////////////////////////////////////////////////////////////

	void AddOrigin( EveMapNodeID origin );
	void ClearOrigins();

private:
	std::vector<EveMapNodeID> m_originSystems;
};

#endif
