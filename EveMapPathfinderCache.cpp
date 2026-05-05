// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveMapPathfinderCache.h"
#include <algorithm>

bool CompareClosedListNodesByJumpCount( ClosedListNode cln1, ClosedListNode cln2 )
{
    return cln1.m_jumpCountFromOrigin < cln2.m_jumpCountFromOrigin;
}

EveMapPathfinderCache::EveMapPathfinderCache() :
	m_hasFoundSolution( false ),
	m_maxValueinSortedList( 0.0f )
{

}

bool EveMapPathfinderCache::AreAllOptionsExhausted() const
{
	return m_sortedOpenList.empty();
}

bool EveMapPathfinderCache::IsComplete() const
{
	return m_hasFoundSolution;
}

ClosedListNode& EveMapPathfinderCache::GetCurrentPath( const EveMapClosedListNodeID& nodeID )
{
	return m_closedList[ nodeID.m_offsetInClosedList ];
}

void EveMapPathfinderCache::ClearCache()
{
	m_maxValueinSortedList = 0.0f;
	m_hasFoundSolution =  false;

	while( !m_sortedOpenList.empty() )
	{
		delete m_sortedOpenList.top();
		m_sortedOpenList.pop();
	}

	for( std::vector<ClosedListNode>::iterator i = m_closedList.begin(); i != m_closedList.end() ;++i )
	{
		i->m_visited = false;
		i->m_jumpCountFromOrigin = -1;
	}
}

void EveMapPathfinderCache::AddCandidate( EveMapNodeID node, EveMapNodeID from, float costToNode, float estimate, ClosedListNode& origin, ClosedListNode& destination)
{
	// TODO: pool?
	// TODO: don't sort everything, maintain an unsorted list
	OpenListNode* newNode = new OpenListNode();

	newNode->m_toNode = node;
	newNode->m_fromNode = from;
	newNode->m_costToNodeFromOrigin = costToNode;
	newNode->m_totalCostEstimate = costToNode + estimate;
	m_sortedOpenList.push( newNode );

	if(destination.m_jumpCountFromOrigin == (unsigned int) -1)
    {
		destination.m_jumpCountFromOrigin = origin.m_jumpCountFromOrigin + 1;
    }
}

const OpenListNode* EveMapPathfinderCache::GetBestCandidate()
{
	if( m_sortedOpenList.empty() )
	{
		return NULL;
	}

	OpenListNode* top = m_sortedOpenList.top();
	m_sortedOpenList.pop();
	return top;
}

bool EveMapPathfinderCache::SetPathToSystem( const EveMap& universe, EveMapNodeID system, EveMapNodeID fromSystem, float cost )
{
	EveMapNode const * const s = universe.GetSolarSystem( system );
	ClosedListNode& c = m_closedList[ s->m_closedListID.m_offsetInClosedList ];

	EveMapNode const * const last = universe.GetSolarSystem( fromSystem );
	ClosedListNode& lastClosedListNode = m_closedList[ last->m_closedListID.m_offsetInClosedList ];

	if( !s || !last )
	{
		return false;
	}

	c.m_costToNode = cost;
	c.m_jumpCountFromOrigin = lastClosedListNode.m_jumpCountFromOrigin + 1;
	c.m_lastClosedListNode = last->m_closedListID;
	c.m_visited = true;
	c.m_mapNodeID = system;

	return true;
}


void EveMapPathfinderCache::Initialize( const EveMap* mapData )
{
	if( mapData != nullptr )
	{
		m_closedList.resize( mapData->GetRequiredClosedListSize() );

		ClearCache();
	}
}

EveMapPathfinderCache::~EveMapPathfinderCache()
{
	while( !m_sortedOpenList.empty() )
	{
		delete m_sortedOpenList.top();
		m_sortedOpenList.pop();
	}
}

void EveMapPathfinderCache::SetSolutionSystem( const EveMap& universe, EveMapNodeID nodeID )
{
	EveMapNode const * const endSystem = universe.GetSolarSystem( nodeID );

	if( !endSystem )
	{
		return;
	}

	EveMapClosedListNodeID closedSystemID = endSystem->m_closedListID;

	m_hasFoundSolution = true;
	m_solution = closedSystemID;

}

Be::Result<PRESULT> EveMapPathfinderCache::GetSolutionSystem( const EveMap* map, unsigned& result )
{
	CHECK_RETURN_MAP( map );

	if( !m_hasFoundSolution )
	{
		return Be::Result<PRESULT>( PRESULT_NO_SOLUTION_FOUND );
	}

	const EveMapNodeID solution = m_closedList[ m_solution.m_offsetInClosedList ].m_mapNodeID;

	result = map->GetSolarSystem(solution)->m_itemID;
	return Be::Result<PRESULT>();
}

Be::Result<PRESULT> EveMapPathfinderCache::GetSystemsWithinJumpCount( 
	const EveMap* mapData, 
	unsigned int minJumpCount, 
	unsigned int maxJumpCount, 
	std::map<unsigned, unsigned>& result )
{
    result.clear();

	CHECK_RETURN_MAP( mapData );

    for( std::vector<ClosedListNode>::iterator i = m_closedList.begin(); i != m_closedList.end() ;++i )
	{
        if( i->m_jumpCountFromOrigin >= minJumpCount && i->m_jumpCountFromOrigin < maxJumpCount )
        {
            result.insert( 
				std::make_pair( mapData->GetSolarSystem(i->m_mapNodeID)->m_itemID, i->m_jumpCountFromOrigin ) 
			);
        }
	}

	return Be::Result<PRESULT>( PRESULT_OK );
}

Be::Result<PRESULT> EveMapPathfinderCache::GetRouteTo( const EveMap* mapData, unsigned destinationID, std::vector<unsigned>& solarSystemIDs )
{
	CHECK_RETURN_MAP( mapData );

	solarSystemIDs.clear();
	
	EveMapNodeID destination;
	CHECK_RETURN_GET_SYSTEM( mapData->GetSolarSystemID(destinationID, destination) );

	const EveMapNode* sol = mapData->GetSolarSystem(destination);
	const bool initialIsVisited = m_closedList[sol->m_closedListID.m_offsetInClosedList].m_visited;

	if( !initialIsVisited )
	{
		return Be::Result<PRESULT>(  );
	}

	while( true )
	{
		solarSystemIDs.push_back( sol->m_itemID );

		const unsigned currentSolClosedOffset = sol->m_closedListID.m_offsetInClosedList;
		const ClosedListNode& currentClosed = m_closedList[currentSolClosedOffset];


		if( currentClosed.m_jumpCountFromOrigin == 0 )
		{
			break;
		}

		sol = mapData->GetSolarSystem( m_closedList[currentClosed.m_lastClosedListNode.m_offsetInClosedList].m_mapNodeID );
	}

	// current order is [last, ..., first]
	std::reverse( solarSystemIDs.begin(), solarSystemIDs.end() );

	return Be::Result<PRESULT>( PRESULT_OK );
}

Be::Result<PRESULT> EveMapPathfinderCache::GetJumpCountTo( const EveMap* mapData, unsigned destinationID, int& out )
{
	CHECK_RETURN_MAP( mapData );

	out = 0;
	
	EveMapNodeID destination;
	CHECK_RETURN_GET_SYSTEM( mapData->GetSolarSystemID(destinationID, destination) );

	const EveMapNode* dest_sol = mapData->GetSolarSystem(destination);
	const ClosedListNode* currentClosed = &m_closedList[ dest_sol->m_closedListID.m_offsetInClosedList ];

	if( !currentClosed->m_visited )
	{
		out = -1;
		return Be::Result<PRESULT>( PRESULT_OK );
	}

	while( true )
	{
		if( currentClosed->m_jumpCountFromOrigin == 0 )
		{
			break;
		}

		++out;
		currentClosed = &m_closedList[currentClosed->m_lastClosedListNode.m_offsetInClosedList];
	}


	return Be::Result<PRESULT>( PRESULT_OK );
}
