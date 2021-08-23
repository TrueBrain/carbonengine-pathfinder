////////////////////////////////////////////////////////////
//
//    Creator:   Daniel Speed
//    Created:   August 2012
//    Copyright: CCP 2010
//

#pragma once
#ifndef EveDjikstrasGoal_H
#define EveDjikstrasGoal_H

#include "Include/IEvePathfinderGoal.h"
#include "EveMapNodes.h"


BLUE_DECLARE(EveDjikstrasGoal);
// -------------------------------------------------------------
// Description:
//   This goal implementation defines a goal which uses a fixed cost to
//   traverse any jump, and has a number of goal systems.
//   
//   This class is mainly intended as an example and for testing.
// SeeAlso:
//   IEvePathfinderGoal
// -------------------------------------------------------------
BLUE_CLASS( EveDjikstrasGoal ):
	public IEvePathfinderGoal
{
public:
	EXPOSE_TO_BLUE();

	//////////////////////////////////////////////////////////////////////////
	// IEvePathfinderGoal 
	//////////////////////////////////////////////////////////////////////////
	bool  IsGoal( const EveMap& universe, EveMapNodeID node ) const override;
	void  GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const override;
	float GetHeuristicEstimate( const EveMapNode& node ) const override;
	float GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const override;
	void  GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const override;
	//////////////////////////////////////////////////////////////////////////

	// Set the goal system to be used when path-finding
	void SetGoal( const EveMap& universe, EveMapNodeID goal );

	// Add a system as an origin with distance 0.0
	void AddOrigin( EveMapNodeID origin );

	// Clear all origin systems
	void ClearOrigins();

private:
	// single system as goal
	EveMapNodeID m_goal;

	// The systems that are at distance 0.0 which we will use to start
	// pathfinding from
	std::vector<EveMapNodeID> m_originSystems;
};

TYPEDEF_BLUECLASS(EveDjikstrasGoal);

#endif // EveDjikstrasGoal_H
