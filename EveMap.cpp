// Copyright © 2014 CCP ehf.

#include "stdafx.h"
#include "EveMap.h"

EveMap::EveMap():
	m_systemCount( 0 )
{
}

// -------------------------------------------------------------
// Description:
//   Create a region in the map. This must be done prior to creating 
//	 children constellations or solar systems.
// Arguments:
//   regionID - itemID of the region
// Return Value:
//   Success or failure
// -------------------------------------------------------------
Be::Result<PRESULT> EveMap::CreateRegion( unsigned regionID )
{
	EveMapNode newNode;

	newNode.m_itemID = regionID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::REGION;
	m_nodes.push_back( newNode );
	
	AddLastEveMapNodeIDToLookup(regionID);

	return Be::Result<PRESULT>( PRESULT_OK );
}

// -------------------------------------------------------------
// Description:
//   Create a constellation in the map. This must be done prior to creating 
//	 children solar systems.
// Arguments:
//   constellationID - itemID of the constellation
//	 regionID - the itemID of the parent region. This must already have been created.
// Return Value:
//   Success or failure
// -------------------------------------------------------------
Be::Result<PRESULT> EveMap::CreateConstellation( unsigned constellationID, unsigned int regionID )
{
	EveMapNode newNode;

	newNode.m_itemID = constellationID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::CONSTELLATION;

	EveMapNodeID parent;

	if( !GetNodeID( regionID, parent ) )
	{
		return Be::Result<PRESULT>( PRESULT_INVALID_ID );
	}

	newNode.m_parent = parent;

	m_nodes.push_back( newNode );

	AddLastEveMapNodeIDToLookup(constellationID);

	return Be::Result<PRESULT>( PRESULT_OK );
}

// -------------------------------------------------------------
// Description:
//   Create a solar system in the map.
// Arguments:
//   solarSystemID - itemID of the solar system
//	 constellationID - the itemID of the parent constellation. This must already have been created.
// Return Value:
//   Success or failure
// -------------------------------------------------------------
Be::Result<PRESULT> EveMap::CreateSystem( unsigned solarSystemID, unsigned constellationID, float security )
{
	EveMapNode newNode;

	newNode.m_itemID = solarSystemID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::SOLAR_SYSTEM;
	newNode.m_jumpsOffset = 0;
	newNode.m_trueSecRating = security;

	if( !GetConstellationID( constellationID, newNode.m_parent ) )
	{
		return Be::Result<PRESULT>( PRESULT_INVALID_ID );
	}

	m_nodes.push_back( newNode );

	AddLastEveMapNodeIDToLookup(solarSystemID);
	
	return Be::Result<PRESULT>( PRESULT_OK );
}

// -------------------------------------------------------------
// Description:
//   Add a jump from one solar system to another.
// Arguments:
//   solarSystemID - EveMapNodeID of the solar system to jump from
//	 toSystemID - EveMapNodeID of the solar system to jump to
//	 jumpGateID - the itemID of the jumpgate
// Return Value:
//   Success or failure
// -------------------------------------------------------------
Be::Result<PRESULT> EveMap::AddJump( unsigned fromID, unsigned toID, unsigned jumpGateID )
{
	EveMapNodeID fromNodeID;
	CHECK_RETURN_GET_SYSTEM( GetSolarSystemID(fromID, fromNodeID) );

	EveMapNodeID toNodeID;
	CHECK_RETURN_GET_SYSTEM( GetSolarSystemID(toID, toNodeID) );

	EveMapNode& from = m_nodes[fromNodeID.m_mapNodeOffset];

	if( from.m_jumpCount == 0 )
	{
		// no jumps yet
		EveSolarSystemJump newJump;
		newJump.m_starGateItemID = jumpGateID;
		newJump.m_toSystemID = toNodeID;

		m_jumps.push_back( newJump );
		from.m_jumpsOffset = m_jumps.size() - 1;
		from.m_jumpCount = 1;

		return Be::Result<PRESULT>( PRESULT_OK );
	}
	else
	{
		// Not so efficient, unless the jumps for a system are all added in one block!
		EveSolarSystemJump newJump;
		newJump.m_starGateItemID = jumpGateID;
		newJump.m_toSystemID = toNodeID;

		// insert at the back of the current list of jumps for this system
		// Ideally, this is at the end of the jumps list
		m_jumps.insert( m_jumps.begin() + from.m_jumpsOffset + from.m_jumpCount, newJump );
		from.m_jumpCount += 1;

		// Shift any references down
		for( std::vector<EveMapNode>::iterator i = m_nodes.begin(); i != m_nodes.end(); i++ )
		{
			if( i->m_type == EveMapNode::SOLAR_SYSTEM && i->m_jumpsOffset > from.m_jumpsOffset )
			{
				i->m_jumpsOffset += 1;
			}
		}

		return Be::Result<PRESULT>( PRESULT_OK );
	}
}

// -------------------------------------------------------------
// Description:
//   Gets an EveMapNodeID for a given itemID, if it exists
// Arguments:
//   itemID - the itemID to look for
//   outNode - the EveMapNodeID to initialize
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::GetNodeID( unsigned itemID, EveMapNodeID& outNode ) const
{
	auto i = m_itemIDToNodeID.find(itemID);

	if( i != m_itemIDToNodeID.end() )
	{
		outNode = i->second;
		return true;
	}

	return false;
}

// -------------------------------------------------------------
// Description:
//   Gets an EveMapNodeID for a given constellation itemID, if it exists
// Arguments:
//   itemID - the itemID to look for
//   outNode - the EveMapNodeID to initialize
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::GetConstellationID( unsigned constellationID, EveMapNodeID& constellation ) const
{
	EveMapNodeID test;

	if( GetNodeID( constellationID, test) )
	{
		if( m_nodes[ test.m_mapNodeOffset ].m_type == EveMapNode::CONSTELLATION )
		{
			constellation.m_mapNodeOffset = test.m_mapNodeOffset;
			return true;
		}
	}

	return false;
}

// -------------------------------------------------------------
// Description:
//   Gets an EveMapNodeID for a given constellation itemID, if it exists
// Arguments:
//   itemID - the itemID to look for
//   outNode - the EveMapNodeID to initialize
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::GetSolarSystemID( unsigned solarSystemID, EveMapNodeID& solarSystem ) const
{
	EveMapNodeID test;

	if( GetNodeID( solarSystemID, test) )
	{
		if( m_nodes[ test.m_mapNodeOffset ].m_type == EveMapNode::SOLAR_SYSTEM )
		{
			solarSystem.m_mapNodeOffset = test.m_mapNodeOffset;
			return true;
		}
	}

	return false;
}

// -------------------------------------------------------------
// Description:
//   Gets the node information for a solar system given an EveMapNodeID
// Arguments:
//   solarSystemID - the EveMapNodeID for the system
// Return Value:
//   Const pointer to the EveMapNode data
// -------------------------------------------------------------
EveMapNode const * EveMap::GetSolarSystem( EveMapNodeID solarSystemID ) const
{
	return &m_nodes[ solarSystemID.m_mapNodeOffset ];
}

// -------------------------------------------------------------
// Description:
//   Gets the node information for a constellation given an EveMapNodeID
// Arguments:
//   solarSystemID - the EveMapNodeID for the constellation
// Return Value:
//   Const pointer to the EveMapNode data
// -------------------------------------------------------------
EveMapNode const * EveMap::GetConstellation( EveMapNodeID solarSystemID ) const
{
	return &m_nodes[ solarSystemID.m_mapNodeOffset ];
}

// -------------------------------------------------------------
// Description:
//   Gets the node information for a region given an EveMapNodeID
// Arguments:
//   solarSystemID - the EveMapNodeID for the region
// Return Value:
//   Const pointer to the EveMapNode data
// -------------------------------------------------------------
EveMapNode const * EveMap::GetRegion( EveMapNodeID solarSystemID ) const
{
	return &m_nodes[ solarSystemID.m_mapNodeOffset ];
}

// -------------------------------------------------------------
// Description:
//   Gets the count of the number of systems initialized to
//   inform the creation of a packed closed list in a contiguous
//   vector
// See Also:
//   EveMapPathfinderCache
// -------------------------------------------------------------
unsigned int EveMap::GetRequiredClosedListSize() const
{
	return m_systemCount;
}

// -------------------------------------------------------------
// Description:
//   Gets the count of the number of systems initialized to
//   inform the creation of a packed closed list in a contiguous
//   vector
// See Also:
//   EveMapPathfinderCache
// -------------------------------------------------------------
void EveMap::FinalizeMap()
{
	m_systemCount = 0;

	std::vector<EveMapNode>::iterator i = m_nodes.begin();
	for(; i != m_nodes.end(); ++i )
	{
		if( i->m_type == EveMapNode::SOLAR_SYSTEM )
		{
			i->m_closedListID.m_offsetInClosedList = m_systemCount;
			++m_systemCount;
		}
	}
}

// -------------------------------------------------------------
// Description:
//   Sets a begin and end const_iterator for the jumps for the given
//   systemID. 
// -------------------------------------------------------------
bool EveMap::GetJumpsForSystem( EveMapNodeID systemID, std::vector<EveSolarSystemJump>::const_iterator& begin, std::vector<EveSolarSystemJump>::const_iterator& end ) const
{
	EveMapNode const * const system = GetSolarSystem( systemID );
	if( system )
	{
		begin = m_jumps.begin() + system->m_jumpsOffset;
		end = m_jumps.begin() + system->m_jumpsOffset + system->m_jumpCount;
		return true;
	}

	return false;
}

void EveMap::AddLastEveMapNodeIDToLookup( unsigned itemID )
{
	EveMapNodeID last;
	last.m_mapNodeOffset = m_nodes.size() - 1;
	m_itemIDToNodeID.insert( std::make_pair(itemID, last));
}

// Sets the security level for a solar system
void EveMap::SetSolarSystemSecurity(unsigned solarSystemID, float security)
{
	EveMapNodeID nodeID;

	if (GetNodeID(solarSystemID, nodeID))
	{
		if (m_nodes[nodeID.m_mapNodeOffset].m_type == EveMapNode::SOLAR_SYSTEM)
		{
			EveMapNode& node = m_nodes[nodeID.m_mapNodeOffset];
			node.m_trueSecRating = security;
		}
	}
}