#pragma once
#ifndef EveStandardFloodFillGoal_H
#define EveStandardFloodFillGoal_H

#include "include/IEvePathfinderGoal.h"
#include "EveMapNodes.h"
#include <vector>
#include <set>

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
class EveStandardFloodFillGoal:
	public IEvePathfinderGoal
{
public:
	EveStandardFloodFillGoal();

	//////////////////////////////////////////////////////////////////////////
	// IEvePathfinderGoal 
	//////////////////////////////////////////////////////////////////////////
	virtual bool  IsGoal( const EveMap& universe, EveMapNodeID node ) const;
	virtual void  GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const;
	virtual float GetHeuristicEstimate( const EveMapNode& node ) const;
	virtual float GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const;
	virtual void  GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const;
	//////////////////////////////////////////////////////////////////////////

	// Apply a penalty to systems outside of the security limits
	void AvoidSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity, float securityPenalty = 3.0f );

	// Prevent absolutely any travel outside of defined security limits
	void DoNotVisitSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity );

	// Make the traversal cost flat, purely 1.0 for any jump
	void IgnoreSecurityLimits();

	// Can start from multiple systems
	void AddOrigin( EveMapNodeID origin );

	// clear the origin systems
	void ClearOrigins();

	// Prevent the pathfinder from going through this system
	void AddAvoidSystem( EveMapNodeID s );
	void ClearAvoidSystems();

private:

	std::vector<EveMapNodeID> m_originSystems;

	// TODO: Set of "avoidance" systems
	std::set<EveMapNodeID> m_avoidSystems;

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

#endif