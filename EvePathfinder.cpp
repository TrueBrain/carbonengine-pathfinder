#include "stdafx.h"

#include <memory>

#include "EvePathfinder.h"
#include "EveMapPathfinderCache.h"
#include "include/IEvePathfinderGoal.h"

typedef EveMapNode const * const constEveMapNodePtr;

#ifdef MSVC_LOCAL_TEST
#include "EveMap.h"
#include "EveFloodFillGoal.h"
#include "EveDjikstrasGoal.h"

int main(void)
{
	// BASIC INIT
	EveMap* universe = new EveMap( 7, 7 );
	EveDjikstrasGoal* goal = new EveDjikstrasGoal();
	EveMapPathfinderCache* cache = new EveMapPathfinderCache();

	unsigned int i = 0;

	// Init map
	universe->CreateRegion( i++ );
	universe->CreateConstellation( i, i-1 );
	i++;

	EveMapNodeID A,B,C,D,E;

	universe->CreateSystem( i++, 1, 1.0f, &A );
	universe->CreateSystem( i++, 1, 1.0f, &B );
	universe->CreateSystem( i++, 1, 1.0f, &C );
	universe->CreateSystem( i++, 1, 1.0f, &D );
	universe->CreateSystem( i++, 1, 1.0f, &E );

	universe->AddJump( A, B, i++ );
	universe->AddJump( B, A, i++ );

	universe->AddJump( B, D, i++ );
	universe->AddJump( D, B, i++ );
	
	universe->AddJump( D, E, i++ );
	universe->AddJump( E, D, i++ );
	
	universe->AddJump( A, C, i++ );
	universe->AddJump( C, A, i++ );
	
	universe->AddJump( C, E, i++ );
	universe->AddJump( E, C, i++ );

	universe->FinalizeMap();

	// Init cache for use with a map. There may be multiple caches
	cache->Initialize( *universe, 15 );

	// PER PATHFIND

	// Set origin
	cache->ClearCache();
	
	goal->AddOrigin( A );
	goal->SetGoal( *universe, C );

	RunPathfinder( *universe, *goal, *cache );

	delete universe;
	delete goal;
	delete cache;

	return 0;
}

#endif

void RunPathfinder( const EveMap& universeMap, const IEvePathfinderGoal& goal, EveMapPathfinderCache& cache )
{
	std::vector<EveMapNodeID> neighbours;
	neighbours.reserve(5);

	std::vector<EveMapNodeID> origins;
	goal.GetOriginSystems( origins );

	// Add all origin systems
	for( std::vector<EveMapNodeID>::iterator originIt = origins.begin(); originIt !=  origins.end(); ++originIt )
	{
		constEveMapNodePtr o = universeMap.GetSolarSystem( *originIt );
		ClosedListNode& originInClosedList = cache.GetCurrentPath( o->m_closedListID );

		originInClosedList.m_isOrigin = true;
		originInClosedList.m_visited = true;
		originInClosedList.m_mapNodeID = *originIt;
	}

	// Populate open list with the neighbours of the origin systems.
	for( std::vector<EveMapNodeID>::iterator originIt = origins.begin(); originIt !=  origins.end(); ++originIt )
	{
		constEveMapNodePtr originSystem = universeMap.GetSolarSystem( *originIt );
		EveMapNodeID origin = *originIt;

		if( originSystem )
		{
			goal.GetNeighbours( universeMap, origin, neighbours );

			for( std::vector<EveMapNodeID>::iterator neighbourIt = neighbours.begin(); neighbourIt != neighbours.end(); ++neighbourIt )
			{
				constEveMapNodePtr candidateSystem = universeMap.GetSolarSystem( *neighbourIt );

				if( !cache.GetCurrentPath( candidateSystem->m_closedListID ).m_isOrigin )
				{
					cache.AddCandidate( 
						*neighbourIt, 
						*originIt, 
						goal.GetTraversalCost( *originSystem,*candidateSystem ), 
						goal.GetHeuristicEstimate(*candidateSystem) );
				}
			}
		}
	}

	// While there are still candidates in the cache
	while( !cache.IsComplete() && !cache.AreAllOptionsExhausted() )
	{
		// delete when done
		std::auto_ptr<const OpenListNode> candidate( cache.GetBestCandidate() );

		if( !candidate.get() )
		{
			return;
		}

		EveMapNodeID candidateSystemID = candidate->m_toNode;
		constEveMapNodePtr candidateSystemNode = universeMap.GetSolarSystem( candidateSystemID );

		if( !candidateSystemNode )
		{
			return;
		}

		// Find the current path to the given node
		ClosedListNode& currentPath = cache.GetCurrentPath( candidateSystemNode->m_closedListID );

		if( currentPath.m_isOrigin )
		{
			// No point in revisiting an origin node
			continue;
		}
		else if( currentPath.m_visited && currentPath.m_costToNode <= candidate->m_costToNodeFromOrigin )
		{
			// the candidate path is longer than an existing path
			continue;
		}

		// Either the new path is shorter, or we haven't visited the node before
		// Set it into the closed list
		cache.SetPathToSystem( universeMap, candidateSystemID, candidate->m_fromNode, candidate->m_costToNodeFromOrigin );

		if( goal.IsGoal( universeMap, candidateSystemID ) )
		{
			// We are done
			cache.SetSolutionSystem( universeMap, candidateSystemID );
			return;
		}

		// Now we explore the neighbours and add them to the open list
		goal.GetNeighbours( universeMap, candidateSystemID, neighbours );

		for( std::vector<EveMapNodeID>::iterator neighbourIt = neighbours.begin(); neighbourIt != neighbours.end(); ++neighbourIt )
		{
			EveMapNodeID neighbourID = *neighbourIt;

			if( neighbourID == candidateSystemID )
			{
				// This is where we just came from. Skip It.
				continue;
			}

			// Generate new open list candidates
			constEveMapNodePtr neighbour = universeMap.GetSolarSystem( neighbourID );

			if( !neighbour )
			{
				continue;
			}

			cache.AddCandidate( 
				neighbourID, 
				candidate->m_toNode, 
				candidate->m_costToNodeFromOrigin + goal.GetTraversalCost( *candidateSystemNode, *neighbour ),
				goal.GetHeuristicEstimate( *neighbour ) );
		}

	}

}
