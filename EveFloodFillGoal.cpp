#include "stdafx.h"
#include "EveFloodFillGoal.h"
#include "EveMapNodes.h"
#include "EveMap.h"

// -------------------------------------------------------------
// Description:
//   Checks if a system is the goal. Never true for this class.
// Arguments:
//   universe - The map being used
//   nodeID - the nodeID to check
// Return Value:
//   true if the node is the goal
// -------------------------------------------------------------
bool EveFloodFillGoal::IsGoal( const EveMap& universe, EveMapNodeID node ) const
{
	return false;
}

// -------------------------------------------------------------
// Description:
//   Gets the cost of going from one node to another. For this class,
//   the nodes are always separated by a single jump, and the cost is
//   always 1.0.
// Arguments:
//   from - the node from
//   to - the node that it's going to
// Return Value:
//   The cost (1.0f)
// -------------------------------------------------------------
float EveFloodFillGoal::GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const
{
	return 1.0f;
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
float EveFloodFillGoal::GetHeuristicEstimate( const EveMapNode& node ) const
{
	return 0.0f;
}

// -------------------------------------------------------------
// Description:
//   Get a vector of the validly traversable neighbors of a system
//   Any systems that must not be entered should be filtered here.
//
//   In this case, all jumps are valid.
// Arguments:
//   universe - the map
//   node - the node from which we need to find neighbours
//   neighbours - (out) a vector of valid NodeIDs to traverse to
// -------------------------------------------------------------
void EveFloodFillGoal::GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const
{
	neighbours.clear();

	std::vector<EveSolarSystemJump>::const_iterator i, end;

	if( universe.GetJumpsForSystem( node, i, end ) )
	{
		for( ; i != end; ++i )
		{
			neighbours.push_back( i->m_toSystemID );
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
void EveFloodFillGoal::GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const
{
	outOriginSystems.assign( m_originSystems.begin(), m_originSystems.end() );
}

// -------------------------------------------------------------
// Description:
//   Add an origin from which path-finding will start
// Arguments:
//   origin - a system that is considered to have a cost of 0.0
// -------------------------------------------------------------
void EveFloodFillGoal::AddOrigin( EveMapNodeID origin )
{
	m_originSystems.push_back( origin );
}

// -------------------------------------------------------------
// Description:
//   Clear all origin systems.
// -------------------------------------------------------------
void EveFloodFillGoal::ClearOrigins()
{
	m_originSystems.clear();
}

