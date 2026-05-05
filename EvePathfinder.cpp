// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include <functional>

BLUE_REGISTER_GLOBAL_AS_MODULE_OBJECT( "classes", BeClasses );
BLUE_STANDARD_MODULE_INIT( _pyevepathfinder );

#include <memory>
#include "EvePathfinder.h"
#include "EveMapPathfinderCache.h"
#include "Include/IEvePathfinderGoal.h"
#include "EveMap.h"

typedef EveMapNode const * const constEveMapNodePtr;

void RunPathfinder( const EveMap* universeMap, const IEvePathfinderGoal* goal, EveMapPathfinderCache* cache )
{
	if( universeMap == nullptr || goal == nullptr || cache == nullptr )
	{
		return;
	}

	std::vector<EveMapNodeID> neighbours;
	neighbours.reserve(5);

	std::vector<EveMapNodeID> origins;
	goal->GetOriginSystems( origins );

	// Add all origin systems
	for( std::vector<EveMapNodeID>::iterator originIt = origins.begin(); originIt !=  origins.end(); ++originIt )
	{
		constEveMapNodePtr o = universeMap->GetSolarSystem( *originIt );
		ClosedListNode& originInClosedList = cache->GetCurrentPath( o->m_closedListID );

		originInClosedList.m_jumpCountFromOrigin = 0;
		originInClosedList.m_visited = true;
		originInClosedList.m_mapNodeID = *originIt;
	}

	// Populate open list with the neighbours of the origin systems.
	for( std::vector<EveMapNodeID>::iterator originIt = origins.begin(); originIt !=  origins.end(); ++originIt )
	{
		constEveMapNodePtr originSystem = universeMap->GetSolarSystem( *originIt );
		EveMapNodeID origin = *originIt;
		ClosedListNode& originPath = cache->GetCurrentPath( originSystem->m_closedListID );

		if( originSystem )
		{
			goal->GetNeighbours( *universeMap, origin, neighbours );

			for( std::vector<EveMapNodeID>::iterator neighbourIt = neighbours.begin(); neighbourIt != neighbours.end(); ++neighbourIt )
			{
				constEveMapNodePtr candidateSystem = universeMap->GetSolarSystem( *neighbourIt );
				ClosedListNode& candidatePath = cache->GetCurrentPath( candidateSystem->m_closedListID );
				
				if( candidatePath.m_jumpCountFromOrigin != 0 )
				{
					cache->AddCandidate( 
						*neighbourIt, 
						*originIt, 
						goal->GetTraversalCost( *originSystem,*candidateSystem ), 
						goal->GetHeuristicEstimate(*candidateSystem),
						originPath,
						candidatePath);
				}
			}
		}
	}

	// While there are still candidates in the cache
	while( !cache->IsComplete() && !cache->AreAllOptionsExhausted() )
	{
		// delete when done
		std::unique_ptr<const OpenListNode> candidate( cache->GetBestCandidate() );

		if( !candidate.get() )
		{
			return;
		}

		EveMapNodeID candidateSystemID = candidate->m_toNode;
		constEveMapNodePtr candidateSystemNode = universeMap->GetSolarSystem( candidateSystemID );

		if( !candidateSystemNode )
		{
			return;
		}

		// Find the current path to the given node
		ClosedListNode& currentPath = cache->GetCurrentPath( candidateSystemNode->m_closedListID );

		if( currentPath.m_jumpCountFromOrigin == 0 )
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
		cache->SetPathToSystem( *universeMap, candidateSystemID, candidate->m_fromNode, candidate->m_costToNodeFromOrigin );

		if( goal->IsGoal( *universeMap, candidateSystemID ) )
		{
			// We are done
			cache->SetSolutionSystem( *universeMap, candidateSystemID );
			return;
		}

		// Now we explore the neighbours and add them to the open list
		goal->GetNeighbours( *universeMap, candidateSystemID, neighbours );

		for( std::vector<EveMapNodeID>::iterator neighbourIt = neighbours.begin(); neighbourIt != neighbours.end(); ++neighbourIt )
		{
			EveMapNodeID neighbourID = *neighbourIt;

			if( neighbourID == candidateSystemID )
			{
				// This is where we just came from. Skip It.
				continue;
			}

			// Generate new open list candidates
			constEveMapNodePtr neighbour = universeMap->GetSolarSystem( neighbourID );
			
			if( !neighbour )
			{
				continue;
			}
						
			// Find the current path to the given node
			ClosedListNode& neighbourPath = cache->GetCurrentPath( neighbour->m_closedListID );

			cache->AddCandidate( 
				neighbourID, 
				candidate->m_toNode, 
				candidate->m_costToNodeFromOrigin + goal->GetTraversalCost( *candidateSystemNode, *neighbour ),
				goal->GetHeuristicEstimate( *neighbour ),
				currentPath,
				neighbourPath
			);
		}
	}
}


MAP_FUNCTION_AND_WRAP( "FindRoute", RunPathfinder, "TODO: Docstring");
