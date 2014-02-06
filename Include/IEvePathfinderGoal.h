////////////////////////////////////////////////////////////
//
//    Creator:   Daniel Speed
//    Created:   August 2012
//    Copyright: CCP 2010
//

#pragma once
#ifndef IEvePathfinderGoal_H
#define IEvePathfinderGoal_H

#include <vector>

class EveMap;
struct EveMapNode;
struct EveMapNodeID;

// -------------------------------------------------------------
// Description:
//   This interface describes *only* the common functions needed for implementing a goal
//   as required by the pathfinder. The goal is responsible for calculating the heuristic for a node,
//   enumerating the valid nodes that the pathfinder can traverse to and determining if path-finding is complete.
//
//   It should not make any assumptions about the number of possible goal nodes etc.
// SeeAlso:
//   RunPathfinder
// -------------------------------------------------------------
class IEvePathfinderGoal
{
public:
	
	// If the map node is one that we're looking for, we're done here
	virtual bool IsGoal( const EveMap& universe, EveMapNodeID node ) const = 0;

	// Get the IDs of the neighbours of a map node that this goal can traverse to
	virtual void GetNeighbours( const EveMap& universe, EveMapNodeID node, std::vector<EveMapNodeID>& neighbours ) const = 0;

	// Get the estimated distance to a goal node
	virtual float GetHeuristicEstimate( const EveMapNode& node ) const = 0;

	// Get the cost of traversing from A to B
	virtual float GetTraversalCost( const EveMapNode& from, const EveMapNode& to ) const = 0;

	// The implementation is responsible for figuring out how to set what the origin / origins are
	virtual void GetOriginSystems( std::vector<EveMapNodeID>& outOriginSystems ) const = 0;

};

#endif