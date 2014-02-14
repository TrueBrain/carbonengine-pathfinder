#pragma once
#ifndef EveMap_h
#define EveMap_h

#include "EveMapNodes.h"
#include <vector>
#include <hash_map>

BLUE_DECLARE( EveMap );

// -------------------------------------------------------------
// Description:
//   Represents the Eve Map in its entirety, in terms of the static structure.
//   Does not contain any runtime data.
//
//	 Both nodes (which include regions and constellations) and jumps are stored in
//   a contiguous vector. Efficient lookups are achieved through the use of 
//	 EveMapNodeID objects, which hold the position of a node in the array.
//
//	 Internal data structures use EveMapNodeIDs exclusively.
// SeeAlso:
//   EveMapNode
// -------------------------------------------------------------
BLUE_CLASS( EveMap ):
	public IRoot
{
public:
	EXPOSE_TO_BLUE();
	typedef std::vector<EveMapNode> MapNodeVectorType;

	EveMap();

	// Create a region. Optionally initialize a given outNode with the new EveMapNodeID
	Be::Result<PRESULT> CreateRegion( unsigned regionID );

	void AddLastEveMapNodeIDToLookup( unsigned itemID );

	// Create a constellation. Optionally initialize a given outNode with the new EveMapNodeID
	Be::Result<PRESULT> CreateConstellation( unsigned constellationID, unsigned int regionID );

	// Create a solar system. Optionally initialize a given outNode with the new EveMapNodeID
	Be::Result<PRESULT> CreateSystem( unsigned solarSystemID, unsigned constellationID, float security );

	// Add a jump from one system to another.
	// A more efficient overload using EveMapNodes is available that doesn't have to look up
	// the systems
	//bool AddJump( unsigned fromSystemID, unsigned toSystemID, unsigned jumpGateID );

	// This is the more efficient way to add jumps using EveMapNodeIDs
	Be::Result<PRESULT> AddJump( unsigned fromID, unsigned toID, unsigned jumpGateID );

	// Gets an EveMapNodeID from a constellation itemID
	bool GetConstellationID( unsigned constellationID, EveMapNodeID& constellation ) const;

	// Gets an EveMapNodeID from a solarsystem itemID
	bool GetSolarSystemID( unsigned solarSystemID, EveMapNodeID& solarSystem ) const;

	bool GetNodeID( unsigned itemID, EveMapNodeID& outNode ) const;

	// Get the EveMapNode for a particular solar system
	EveMapNode const * GetSolarSystem( EveMapNodeID solarSystemID ) const;

	// Get the EveMapNode for a particular constellation
	EveMapNode const * GetConstellation( EveMapNodeID solarSystemID ) const;

	// Get the EveMapNode for a particular GetRegion
	EveMapNode const * GetRegion( EveMapNodeID solarSystemID ) const;

	// Calculate packing for the closed list of solar systems
	void FinalizeMap();

	// Get the number of closed list entries required
	// Pre-requires that the map has been finalized
	unsigned int GetRequiredClosedListSize() const;

	// Gives you an iterator range for getting the jumps
	bool GetJumpsForSystem( 
			EveMapNodeID systemID, 
			std::vector<EveSolarSystemJump>::const_iterator& begin, 
			std::vector<EveSolarSystemJump>::const_iterator& end ) const;

private:
	// Storage for the map
	std::vector<EveMapNode> m_nodes;

	// Storage for the jumps for solar systems
	std::vector<EveSolarSystemJump> m_jumps;

	// only for use on public facing functions
	std::hash_map<unsigned,EveMapNodeID> m_itemIDToNodeID;

	// used for deciding the size of the closed list
	unsigned int m_systemCount;
};

TYPEDEF_BLUECLASS( EveMap );

#endif