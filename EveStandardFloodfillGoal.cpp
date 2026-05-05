// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveStandardFloodfillGoal.h"
#include "EveMapNodes.h"
#include "EveMap.h"

EveStandardFloodFillGoal::EveStandardFloodFillGoal():
	m_securityRatingBehaviour( PATHFINDING_BEHAVIOUR_NONE ),
	m_maxSecurityRating( 1.0f ),
	m_minSecurityRating( -1.0f ),
	m_securityPenalty( 3.0f )
{

}

// -------------------------------------------------------------
// Description:
//   Checks if a system is the goal. Never true for this class.
// Arguments:
//   universe - The map being used
//   nodeID - the nodeID to check
// Return Value:
//   true if the node is the goal
// -------------------------------------------------------------
bool EveStandardFloodFillGoal::IsGoal( const EveMap& universe, EveMapNodeID node ) const
{
	return false;
}

// -------------------------------------------------------------
// Description:
//   Gets the cost of going from one node to another. 
//	 If the behavior is to find the shortest path or strict avoidance, this cost is 1.0
//   If the behavior is avoidance, then apply penalty if it's outside the limits.
//   If it is outside the limits, and also below 0.0, then double the cost
//	 if it is within the limits, apply an arcane formula copied from destiny.
// Arguments:
//   from - the node from
//   to - the node that it's going to
// Return Value:
//   The cost
// -------------------------------------------------------------
float EveStandardFloodFillGoal::GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const
{
	// This is intended to be an equivalent implementation to the one in destiny
	if( m_securityRatingBehaviour != PATHFINDING_BEHAVIOUR_AVOID )
	{
		return 1.0f;
	}
	else
	{
		float systemSecurityRating = to.m_trueSecRating;

		if( systemSecurityRating <= 0.0f && systemSecurityRating <= m_minSecurityRating )
		{
			return 2.0f*m_securityPenalty;
		}
		else if( systemSecurityRating < m_minSecurityRating )
		{
			return m_securityPenalty;
		}
		else if( systemSecurityRating > m_maxSecurityRating)
		{
			return m_securityPenalty;
		}
		else
		{
			// Consider all empire space the same
			if( systemSecurityRating >= 0.45f )
			{
				systemSecurityRating = 1.0f;
			}
			else if( systemSecurityRating > 0.0f )
			{
				systemSecurityRating = 0.45f;
			}

			return 0.1f/(m_minSecurityRating-m_maxSecurityRating)*systemSecurityRating + 0.1f*(9.0f*m_minSecurityRating-10.0f*m_maxSecurityRating)/(m_minSecurityRating-m_maxSecurityRating);
		}
	}
}

// -------------------------------------------------------------
// Description:
//   Gets the estimate of how far a node is away from the goal in terms of
//   the sum traversal cost.
//   This function *should* be monotonic (See: http://en.wikipedia.org/wiki/A*_search_algorithm )
//   for optimal path-finding, meaning that it never over-estimates the distance to the goal.
//
//   All systems are considered equally likely to be a goal.
// Arguments:
//   node - the node to estimate
// Return Value:
//   The estimated distance to a goal
// -------------------------------------------------------------
float EveStandardFloodFillGoal::GetHeuristicEstimate( const EveMapNode& node ) const
{
	return 0.0f;
}

// -------------------------------------------------------------
// Description:
//   Get a vector of the validly traversable neighbors of a system
//   Any systems that must not be entered should be filtered here.
//
//   If the path-finding behavior is strict, it cannot traverse to
//   to systenms that fall outside of the security limits.
// Arguments:
//   universe - the map
//   node - the node from which we need to find neighbours
//   neighbours - (out) a vector of valid NodeIDs to traverse to
// -------------------------------------------------------------
void EveStandardFloodFillGoal::GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const
{
	neighbours.clear();

	std::vector<EveSolarSystemJump>::const_iterator i, end;

	if( universe.GetJumpsForSystem( node, i, end ) )
	{
		for( ; i != end; ++i )
		{
			// we will not traverse to a system that is set to be avoided, unless it is
			// also set as a goal
			if( m_avoidSystems.find(i->m_toSystemID) != m_avoidSystems.end() )
			{
				if( m_goalSystems.find(i->m_toSystemID) == m_goalSystems.end() )
				{
					continue;
				}
			}

			if( m_securityRatingBehaviour == PATHFINDING_BEHAVIOUR_STRICT )
			{
				// under strict path finding behavior, systems outside of the valid range are completely ignored
				float systemSecurityRating = universe.GetSolarSystem( i->m_toSystemID )->m_trueSecRating;

				if( systemSecurityRating >= m_minSecurityRating && systemSecurityRating <= m_maxSecurityRating )
				{
					neighbours.push_back( i->m_toSystemID );
				}
			}
			else
			{
				neighbours.push_back( i->m_toSystemID );
			}
		}
	}
}

// -------------------------------------------------------------
// Description:
//   Get the systems from which we will start finding a path, which are
//   considered to be cost 0
// Arguments:
//   outOriginSystems - a vector to be filled with the node IDs
// -------------------------------------------------------------
void EveStandardFloodFillGoal::GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const
{
	outOriginSystems.assign( m_originSystems.begin(), m_originSystems.end() );
}

// -------------------------------------------------------------
// Description:
//   Add an origin from which path-finding will start
// Arguments:
//   origin - a system that is considered to have a cost of 0.0
// -------------------------------------------------------------
Be::Result<PRESULT> EveStandardFloodFillGoal::AddOrigin( const EveMap* map, unsigned originID )
{
	if( map == nullptr )
	{
		return Be::Result<PRESULT>( PRESULT_NO_MAP );
	}
	EveMapNodeID destination;

	if( !map->GetSolarSystemID( originID, destination ) )
	{
		return Be::Result<PRESULT>( PRESULT_INVALID_ID );
	}

	m_originSystems.push_back( destination );

	return Be::Result<PRESULT>();
}

// -------------------------------------------------------------
// Description:
//   Clear all origin systems.
// -------------------------------------------------------------
void EveStandardFloodFillGoal::ClearOrigins()
{
	m_originSystems.clear();
}

// -------------------------------------------------------------
// Description:
//   Set the path finding behavior to avoidance with the given limits
//   and penalty
// -------------------------------------------------------------
void EveStandardFloodFillGoal::AvoidSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity, float securityPenalty /*= 3.0f */ )
{
	m_securityRatingBehaviour = PATHFINDING_BEHAVIOUR_AVOID;
	m_minSecurityRating = minSecurity;
	m_maxSecurityRating = maxSecurity;
	m_securityPenalty = securityPenalty;
}

// -------------------------------------------------------------
// Description:
//   Set the path finding behavior to strictly avoid systems outside
//   the given security limits
// -------------------------------------------------------------
void EveStandardFloodFillGoal::DoNotVisitSystemsOutsideSecurityLimits( float minSecurity, float maxSecurity )
{
	m_securityRatingBehaviour = PATHFINDING_BEHAVIOUR_STRICT;
	m_minSecurityRating = minSecurity;
	m_maxSecurityRating = maxSecurity;
}

// -------------------------------------------------------------
// Description:
//   Set the path finding behavior to ignore security rating
//   and only work based on the shortest path
// -------------------------------------------------------------
void EveStandardFloodFillGoal::IgnoreSecurityLimits()
{
	m_securityRatingBehaviour = PATHFINDING_BEHAVIOUR_NONE;
}

Be::Result<PRESULT> EveStandardFloodFillGoal::AddAvoidSystem( const EveMap* map, unsigned systemID )
{
	CHECK_RETURN_MAP( map );
	EveMapNodeID avoidSystem;
	CHECK_RETURN_GET_SYSTEM( map->GetSolarSystemID(systemID, avoidSystem) );

	m_avoidSystems.insert( avoidSystem );

	return Be::Result<PRESULT>( PRESULT_OK );
}

void EveStandardFloodFillGoal::ClearAvoidSystems()
{
	m_avoidSystems.clear();
}

Be::Result<PRESULT> EveStandardFloodFillGoal::AddGoalSystem( const EveMap* map, unsigned goalID )
{
	CHECK_RETURN_MAP( map );
	EveMapNodeID goalSystem;
	CHECK_RETURN_GET_SYSTEM( map->GetSolarSystemID(goalID, goalSystem) );

	m_goalSystems.insert( goalSystem );

	return Be::Result<PRESULT>( PRESULT_OK );
}

void EveStandardFloodFillGoal::ClearGoalSystems()
{
	m_goalSystems.clear();
}
