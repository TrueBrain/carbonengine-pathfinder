////////////////////////////////////////////////////////////
//
//    Creator:   Daniel Speed
//    Created:   August 2012
//    Copyright: CCP 2010
//

#pragma once
#ifndef EveMapNodes_H
#define EveMapNodes_H

// -------------------------------------------------------------
// Description:
//		EveMapClosedListNodeID is an opaque lookup handle object
//   into the closed list. This allows better type safety than
//   using an integer index, since there is a difference between
//   the closed list ID and map ID.
// SeeAlso:
//		EveMapNodeID, EveMapPathfinderCache
// -------------------------------------------------------------
struct EveMapClosedListNodeID
{
	// The index of the map node in the closed list array
	unsigned int m_offsetInClosedList;

	bool operator==( const EveMapClosedListNodeID& other ) const
	{
		return m_offsetInClosedList == other.m_offsetInClosedList;
	}
};

// -------------------------------------------------------------
// Description:
//		EveMapNodeID is an opaque lookup handle object
//   into the nodes array in the map. This allows better type 
//   safety than using an integer index, since there is a difference 
//   between the closed list ID and map ID.
// SeeAlso:
//		EveMapClosedListNodeID, EveMap
// -------------------------------------------------------------
struct EveMapNodeID
{
public:

	// The offset within the map array
	size_t m_mapNodeOffset;

	bool operator==( const EveMapNodeID& other ) const
	{
		return m_mapNodeOffset == other.m_mapNodeOffset;
	}

	// For ordering only! does not compare the itemIDs etc
	bool operator<( const EveMapNodeID& other ) const
	{
		return m_mapNodeOffset < other.m_mapNodeOffset;
	}
};

// -------------------------------------------------------------
// Description:
//   EveSolarSystemJump is the generally used definition of
//   a jump going from one system to another. The sytem it goes to
//   is stored as a direct handle.
// SeeAlso:
//		EveMapNodeID, EveMap
// -------------------------------------------------------------
struct EveSolarSystemJump
{
	// The ID of the stargate for the jump
	unsigned int m_starGateItemID;

	// system that the jump goes to
	// the 'from' is implicit
	EveMapNodeID m_toSystemID;
};

// -------------------------------------------------------------
// Description:
//   EveMapNode describes regions, constellations, and
//   solar systems. For regions and constellations, only the
//   type, itemID and parent is relevant.
// SeeAlso:
//		EveMapNodeID, EveMap
// -------------------------------------------------------------
struct EveMapNode
{
	// The itemID in Eve
	unsigned int m_itemID;

	enum EveMapNodeType {
		SOLAR_SYSTEM,
		CONSTELLATION,
		REGION,
	} m_type;

	// Only set to sensible values for constellations and solar systems
	EveMapNodeID m_parent;

	// For looking up the map node in the closed list
	EveMapClosedListNodeID m_closedListID;

	// The true security rating of the system
	float m_trueSecRating;

	// number of jumps out of this solar system
	// if this is 0, do not look up any jumps!
	unsigned short m_jumpCount;

	// pointer to an array of jumps
	size_t m_jumpsOffset;
};


#endif //EveMapNodes_H