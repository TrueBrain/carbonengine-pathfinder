#pragma once
#ifndef EveMap_h
#define EveMap_h

#include "EveMapNodes.h"
#include <vector>


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
class EveMap
{
public:
	typedef std::vector<EveMapNode> MapNodeVectorType;

	EveMap( unsigned nodeCount, unsigned jumpCount );

	// Create a region. Optionally initialize a given outNode with the new EveMapNodeID
	bool CreateRegion( unsigned regionID, EveMapNodeID* outNode = NULL );

	// Create a constellation. Optionally initialize a given outNode with the new EveMapNodeID
	bool CreateConstellation( unsigned constellationID, unsigned int regionID, EveMapNodeID* outNode = NULL );

	// Create a solar system. Optionally initialize a given outNode with the new EveMapNodeID
	bool CreateSystem( unsigned solarSystemID, unsigned constellationID, float security, EveMapNodeID* outNode = NULL );

	// Add a jump from one system to another.
	// A more efficient overload using EveMapNodes is available that doesn't have to look up
	// the systems
	bool AddJump( unsigned fromSystemID, unsigned toSystemID, unsigned jumpGateID );

	// This is the more efficient way to add jumps using EveMapNodeIDs
	bool AddJump( EveMapNodeID fromID, EveMapNodeID toID, unsigned jumpGateID );

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

	// Cython, fucking me over.
	EveMapNode* GetSolarSystem2( EveMapNodeID solarSystemID );

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

	// used for deciding the size of the closed list
	unsigned int m_systemCount;
};

#endif