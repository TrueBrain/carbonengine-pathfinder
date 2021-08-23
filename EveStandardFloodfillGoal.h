#pragma once
#ifndef EveStandardFloodFillGoal_H
#define EveStandardFloodFillGoal_H

#include "Include/IEvePathfinderGoal.h"
#include "EveMapNodes.h"
#include <vector>
#include <set>

BLUE_DECLARE(EveStandardFloodFillGoal);

// -------------------------------------------------------------
// Description:
//   EveStandardFloodFillGoal represents the standard path-finding behavior
//   based off the historical implementation that was done in destiny.
//
//   In general, the features are as follows:
//     * Three different strategies for dealing with security rating:
//        1) Ignoring it completely
//        2) Avoiding going out of a min/max limit by applying a cost
//        3) Preventing absolutely going out of a min/max limit
//     * "Avoidance", where systems are excluded completely, unless
//        they are also goals.
//     * Optional goals (flood fill is usually used)
//     * Security rating flattening - making different effective security 
//         bands in eve affect the penalty, rather than the precise value
// SeeAlso:
//   IEvePathfinderGoal
// -------------------------------------------------------------
BLUE_CLASS(EveStandardFloodFillGoal):
	public IEvePathfinderGoal
{
public:
	EXPOSE_TO_BLUE();

	EveStandardFloodFillGoal();

	//////////////////////////////////////////////////////////////////////////
	// IEvePathfinderGoal 
	//////////////////////////////////////////////////////////////////////////
	bool  IsGoal( const EveMap& universe, EveMapNodeID node ) const override;
	void  GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const override;
	float GetHeuristicEstimate( const EveMapNode& node ) const override;
	float GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const override;
	void  GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const override;
	//////////////////////////////////////////////////////////////////////////

	// Apply a penalty to systems outside of the security limits
	void AvoidSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity, float securityPenalty = 3.0f );

	// Prevent absolutely any travel outside of defined security limits
	void DoNotVisitSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity );

	// Make the traversal cost flat, purely 1.0 for any jump
	void IgnoreSecurityLimits();

	// Can start from multiple systems
	Be::Result<PRESULT> AddOrigin( const EveMap* map, unsigned originID );

	// clear the origin systems
	void ClearOrigins();

	// Prevent the pathfinder from going through this system
	Be::Result<PRESULT> AddAvoidSystem( const EveMap* map, unsigned systemID );
	void ClearAvoidSystems();

	// We can have goal systems. These do not cause the path-finding to terminate
	Be::Result<PRESULT> AddGoalSystem( const EveMap* map, unsigned goalID );
	void ClearGoalSystems();

private:

	std::vector<EveMapNodeID> m_originSystems;
	std::set<EveMapNodeID> m_avoidSystems;
	std::set<EveMapNodeID> m_goalSystems;

	enum {
		PATHFINDING_BEHAVIOUR_NONE,
		PATHFINDING_BEHAVIOUR_AVOID,
		PATHFINDING_BEHAVIOUR_STRICT
	} m_securityRatingBehaviour;

	// The maximum security rating when evaluating strict or penalty based
	// security level based avoidance of systems
	float m_maxSecurityRating;

	//
	//
	float m_minSecurityRating;
	
	//
	//
	float m_securityPenalty;

};

TYPEDEF_BLUECLASS(EveStandardFloodFillGoal);

#endif
