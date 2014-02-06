#include "stdafx.h"
#include "EveMapPathfinderCache.h"

EveMapPathfinderCache::EveMapPathfinderCache() :
	m_hasFoundSolution( false ),
	m_sortedListHeadSize( 15 ),
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
		i->m_isOrigin = false;
	}
}

void EveMapPathfinderCache::AddCandidate( EveMapNodeID node, EveMapNodeID from, float costToNode, float estimate )
{
	// TODO: pool?
	// TODO: don't sort everything, maintain an unsorted list
	OpenListNode* newNode = new OpenListNode();

	newNode->m_toNode = node;
	newNode->m_fromNode = from;
	newNode->m_costToNodeFromOrigin = costToNode;
	newNode->m_totalCostEstimate = costToNode + estimate;

	m_sortedOpenList.push( newNode );
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

	if( !s || !last )
	{
		return false;
	}

	c.m_costToNode = cost;
	c.m_isOrigin = false;
	c.m_lastClosedListNode = last->m_closedListID;
	c.m_visited = true;
	c.m_mapNodeID = system;

	return true;
}


void EveMapPathfinderCache::Initialize( const EveMap& mapData, unsigned int sortedListHeadSize )
{
	m_closedList.resize( mapData.GetRequiredClosedListSize() );

	ClearCache();

	m_sortedListHeadSize = sortedListHeadSize;
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

EveMapNodeID EveMapPathfinderCache::GetSolutionSystemID()
{
	return m_closedList[ m_solution.m_offsetInClosedList ].m_mapNodeID;
}

