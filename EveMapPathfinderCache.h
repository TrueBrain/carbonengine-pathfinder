#pragma once
#ifndef EveMapPathfinderCache_h
#define EveMapPathfinderCache_h

#include <vector>
#include <map>
#include <queue>


#include "EveMapNodes.h"
#include "EveMap.h"

// -------------------------------------------------------------
// Description:
//   The closed list node contains information about explored
//   systems. It is held in a contiguous vector, with direct
//   lookups provided by the EveMapClosedListNodeID handle
// SeeAlso:
//   EveMapClosedListNodeID, OpenListNode, EveMapPathfinderCache
// -------------------------------------------------------------
struct ClosedListNode
{
	// Has this Node been visited yet
	// and therefore, is it initialized?
	bool m_visited;

	// Absolute cost to the node from the origin
	float m_costToNode;

	// The solar system that this closed list item represents
	EveMapNodeID m_mapNodeID;

	unsigned int m_jumpCountFromOrigin;

	// where we came from
	EveMapClosedListNodeID m_lastClosedListNode;
};

// -------------------------------------------------------------
// Description:
//   The open list node contains information about a system that
//   is a candidate for evaluation.
//   It is held in a prioritized queue, with the lowest cost candidate
//   always being the next to be evaluated.
// SeeAlso:
//   OpenListNodeComparison, EveMapPathfinderCache
// -------------------------------------------------------------
struct OpenListNode
{
	// cost to this node, plus the estimated
	// cost to a goal
	float m_totalCostEstimate;

	// The node that we would be navigating to
	EveMapNodeID m_toNode;
	
	// The node that we are coming from
	EveMapNodeID m_fromNode;

	// The cost to get to this node from the origin
	float m_costToNodeFromOrigin;	 
};

// -------------------------------------------------------------
// Description:
//  Storage class for how many jumps there are to a specific system
// SeeAlso:
//  GetSystemsWithinJumpCount
// -------------------------------------------------------------
struct JumpCountToSystem
{
    // The jump count from the origin to the node
    unsigned int m_jumpCount;
    // The nodeID of the system
    EveMapNodeID m_node;

    bool operator == ( const JumpCountToSystem rhs ) const
	{
		return m_jumpCount == rhs.m_jumpCount && m_node == rhs.m_node;
	};
};

// -------------------------------------------------------------
// Description:
//   Comparison class for OpenListNode
// SeeAlso:
//   OpenListNode, EveMapPathfinderCache
// -------------------------------------------------------------
class OpenListNodeComparison
{
public:
	bool operator()( OpenListNode* lhs, OpenListNode* rhs) const
	{
		return ( lhs->m_totalCostEstimate > rhs->m_totalCostEstimate );
	}
};


BLUE_DECLARE(EveMapPathfinderCache);

// -------------------------------------------------------------
// Description:
//   This cache contains all the working data and output for path finding
//	 this includes the open list (the best candidates to explore next)
//	 the closed list (the explored nodes and their solutions) any
// SeeAlso:
//   OpenListNode, ClosedListNode
// -------------------------------------------------------------
BLUE_CLASS(EveMapPathfinderCache):
	public IRoot
{
public:
	EXPOSE_TO_BLUE();

	EveMapPathfinderCache();
	~EveMapPathfinderCache();

	// EXPOSED

	// Primes a cache for use with a given map, reserving memory and a closed
	// list large enough to handle it
	void Initialize( const EveMap* mapData );

	// Reset all key information in the closed list
	// Clear and delete all open list nodes
	void ClearCache();
	
	// node ids that are within the jump count range [minJumpCount, maxJumpCount) (maxJumpCount not included)
	Be::Result<PRESULT> GetSystemsWithinJumpCount(
		const EveMap* mapData, 
		unsigned int minJumpCount, 
		unsigned int maxJumpCount,
		std::map<unsigned,unsigned>& result );

	Be::Result<PRESULT> GetRouteTo( 
		const EveMap* mapData, 
		unsigned destination, 
		std::vector<unsigned>& out_solarSystemIDs );

	Be::Result<PRESULT> GetJumpCountTo( 
		const EveMap* mapData, 
		unsigned destination, 
		int& result );

	// --- NOT EXPOSED ---

	// Get the node with the best estimate from the open list
	const OpenListNode* GetBestCandidate();

	// Get the closed list node information
	ClosedListNode& GetCurrentPath( const EveMapClosedListNodeID& nodeID );

	// Adds a solar-system to the potential starting points
	// for path-finding
	void AddOriginSystem( const EveMap& universe, EveMapNodeID startSystem );

	// Adds a candidate to the open list
	void AddCandidate( 
		EveMapNodeID node, 
		EveMapNodeID from, 
		float costToNode, 
		float estimate, 
		ClosedListNode& origin, 
		ClosedListNode& destination);

	// Used by the pathfinder to set the current best path to a system
	bool SetPathToSystem( 
		const EveMap& universe, 
		EveMapNodeID system, 
		EveMapNodeID fromSystem, 
		float cost );

	// True when the open list is empty
	bool AreAllOptionsExhausted() const;

	// Once a solution is found, we need a way to find it
	void SetSolutionSystem( const EveMap& universe, EveMapNodeID nodeID );

	// Only valid to call if IsComplete is True
	Be::Result<PRESULT> GetSolutionSystem( const EveMap* map, unsigned& result );

	// True when a goal node has been reached
	bool IsComplete() const;

private:

	bool m_hasFoundSolution;
	EveMapClosedListNodeID m_solution;

	// TODO: Using a fully sorted priority queue is
	// a little wasteful. Partially sorting a vector might be better.
	// TODO: Some sort of memory pooling to avoid excessive allocations

	// Unused 
	float m_maxValueinSortedList;

	// This is the sorted list of the top N candidates for evaluation
	// We do sorted inserts, pop from the front
	std::priority_queue<OpenListNode*, std::vector<OpenListNode*>, OpenListNodeComparison> m_sortedOpenList;

	// Closed list
	// This is the vector constraining the information about visited systems
	std::vector<ClosedListNode> m_closedList;
};

TYPEDEF_BLUECLASS( EveMapPathfinderCache );
#endif