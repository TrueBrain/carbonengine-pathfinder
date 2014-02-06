#include "EveMap.h"

EveMap::EveMap( unsigned nodeCount, unsigned jumpCount ):
	m_systemCount( 0 )
{
	m_nodes.reserve( nodeCount );
	m_jumps.reserve( jumpCount );
}

// -------------------------------------------------------------
// Description:
//   Create a region in the map. This must be done prior to creating 
//	 children constellations or solar systems.
// Arguments:
//   regionID - itemID of the region
//	 outNode - an optional pointer to an EveMapNodeID to initialize for the created region, or NULL
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::CreateRegion( unsigned regionID, EveMapNodeID* outNode )
{
	EveMapNode newNode;

	newNode.m_itemID = regionID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::REGION;

	m_nodes.push_back( newNode );

	if( outNode )
	{
		outNode->m_mapNodeOffset = m_nodes.size() - 1;
	}

	return true;
}

// -------------------------------------------------------------
// Description:
//   Create a constellation in the map. This must be done prior to creating 
//	 children solar systems.
// Arguments:
//   constellationID - itemID of the constellation
//	 regionID - the itemID of the parent region. This must already have been created.
//	 outNode - an optional pointer to an EveMapNodeID to initialize for the created constellation, or NULL
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::CreateConstellation( unsigned constellationID, unsigned int regionID, EveMapNodeID* outNode )
{
	EveMapNode newNode;

	newNode.m_itemID = constellationID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::CONSTELLATION;

	EveMapNodeID parent;

	if( !GetNodeID( regionID, parent ) )
	{
		return false;
	}

	newNode.m_parent = parent;

	m_nodes.push_back( newNode );

	if( outNode )
	{
		outNode->m_mapNodeOffset = m_nodes.size() - 1;
	}

	return true;
}

// -------------------------------------------------------------
// Description:
//   Create a solar system in the map.
// Arguments:
//   solarSystemID - itemID of the solar system
//	 constellationID - the itemID of the parent constellation. This must already have been created.
//	 outNode - an optional pointer to an EveMapNodeID to initialize for the created system, or NULL
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::CreateSystem( unsigned solarSystemID, unsigned constellationID, float security, EveMapNodeID* outNode )
{
	EveMapNode newNode;

	newNode.m_itemID = solarSystemID;
	newNode.m_jumpCount = 0;
	newNode.m_type = EveMapNode::SOLAR_SYSTEM;
	newNode.m_jumpsOffset = 0;
	newNode.m_trueSecRating = security;

	if( !GetConstellationID( constellationID, newNode.m_parent ) )
	{
		return false;
	}

	m_nodes.push_back( newNode );

	if( outNode )
	{
		outNode->m_mapNodeOffset = m_nodes.size() - 1;
	}

	return true;
}

// -------------------------------------------------------------
// Description:
//   Add a jump from one solarsystem to another. This version is not
//   as efficient as specifying the systems using EveMapNodeIDs
// Arguments:
//   solarSystemID - itemID of the solar system to jump from
//	 toSystemID - itemID of the solar system to jump to
//	 jumpGateID - the itemID of the jumpgate
// Return Value:
//   Success or failure
// -------------------------------------------------------------
bool EveMap::AddJump( unsigned fromSystemID, unsigned toSystemID, unsigned jumpGateID )
{
	EveMapNodeID fromSystem, toSystem;

	if( !GetSolarSystemID( fromSystemID, fromSystem ) )
	{
		return false;
	}
	if( !GetSolarSystemID( toSystemID, toSystem ) )
	{
		return false;
	}

	return AddJump( fromSystem, toSystem, jumpGateID );
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
bool EveMap::AddJump( EveMapNodeID fromID, EveMapNodeID toID, unsigned jumpGateID )
{
	EveMapNode& from = m_nodes[fromID.m_mapNodeOffset];

	if( from.m_jumpCount == 0 )
	{
		// no jumps yet
		EveSolarSystemJump newJump;
		newJump.m_starGateItemID = jumpGateID;
		newJump.m_toSystemID = toID;

		m_jumps.push_back( newJump );
		from.m_jumpsOffset = m_jumps.size() - 1;
		from.m_jumpCount = 1;

		return true;
	}
	else
	{
		// Not so efficient, unless the jumps for a system are all added in one block!
		EveSolarSystemJump newJump;
		newJump.m_starGateItemID = jumpGateID;
		newJump.m_toSystemID = toID;

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

		return true;
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
	// TODO: This is the least efficient possible implementation!
	std::vector<EveMapNode>::const_iterator i = m_nodes.begin();
	for(; i != m_nodes.end(); ++i )
	{
		if( i->m_itemID == itemID )
		{
			outNode.m_mapNodeOffset = i - m_nodes.begin();
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

EveMapNode* EveMap::GetSolarSystem2( EveMapNodeID solarSystemID )
{
	return &m_nodes[ solarSystemID.m_mapNodeOffset ];
}
