#pragma once
#ifndef EvePathfinder_h
#define EvePathfinder_h

class EveMap;
class EveMapPathfinderCache;
struct IEvePathfinderGoal;

// This is the main function for running the pathfinder
// The map defines the universe and its connections
// The goal controls the behavior of the pathfinder and when it terminates
// The cache contains the data for the path finding as it processes
// It should be possible to multi-thread, given separate caches, it would also be simple to add time slicing if desirable.
void RunPathfinder( const EveMap* universeMap, const IEvePathfinderGoal* goal, EveMapPathfinderCache* cache );

#endif