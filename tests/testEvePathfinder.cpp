////////////////////////////////////////////////////////////
//
//    Creator:   Olafur Thor Gunnarsson
//    Created:   December 2012
//    Copyright: CCP 2012
//

#include "stdafx.h"

#if GOOGLE_TEST

#include "EvePathfinder.h"
#include "EveMapPathfinderCache.h"
#include "include/IEvePathfinderGoal.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "EveMap.h"
#include "EveFloodFillGoal.h"
#include "EveDjikstrasGoal.h"

typedef EveMapNode const * const constEveMapNodePtr;

// -------------------------------------------------------------
// Description:
//   JumpTestPair defines an expected result of a jump count test 
//   between two systems
// SeeAlso:
//   EvePathfinderJumpTest, TestJumpCount, TestJumpCountReversePath
// -------------------------------------------------------------
struct JumpTestPair
{
    char originSystemName;
    char destinationSystemName;
    unsigned int expectedJumpCount;
};


const JumpTestPair JUMP_TEST_PAIRS[] = {
    {'A', 'A', 0},
    {'A', 'B', 1},
    {'A', 'C', 1},
    {'A', 'D', 2},
    {'A', 'E', 2},
    {'B', 'B', 0},
    {'B', 'C', 2},
    {'B', 'D', 1},
    {'B', 'E', 2},
    {'C', 'C', 0},
    {'C', 'D', 2},
    {'C', 'E', 1},
    {'D', 'D', 0},
    {'D', 'E', 1},
    {'E', 'E', 0}
};


// -------------------------------------------------------------
// Description:
//   EvePathfinderTest creates the base test that is used within 
//   the unit tests. It is template-ized so we can create parameterized
//   tests in addition with vanilla tests
// SeeAlso:
//   EvePathfinderBaseTest, EvePathfinderJumpTest
// -------------------------------------------------------------
template <class T> class EvePathfinderTest: public T {
protected:
    virtual void SetUp()
    {
        /*
            This function creates the m_universe consisting of systems A, B, C, D and E
            with jumps according to the following ascii diagram
                               A
                             /   \
                            B     C
                            |     |
                            D ___ E

        */
        // BASIC INIT
        this->m_universe = new EveMap( 7, 7 );
        this->m_goal = new EveDjikstrasGoal();
        this->m_cache = new EveMapPathfinderCache();

        unsigned int i = 0;

        // Init map
        this->m_universe->CreateRegion( i++ );
        this->m_universe->CreateConstellation( i, i-1 );
        i++;

        this->m_universe->CreateSystem( i++, 1, 1.0f, &this->m_nodeA );
        this->m_universe->CreateSystem( i++, 1, 1.0f, &this->m_nodeB );
        this->m_universe->CreateSystem( i++, 1, 1.0f, &this->m_nodeC );
        this->m_universe->CreateSystem( i++, 1, 1.0f, &this->m_nodeD );
        this->m_universe->CreateSystem( i++, 1, 1.0f, &this->m_nodeE );

        this->m_universe->AddJump( this->m_nodeA, this->m_nodeB, i++ );
        this->m_universe->AddJump( this->m_nodeB, this->m_nodeA, i++ );

        this->m_universe->AddJump( this->m_nodeB, this->m_nodeD, i++ );
        this->m_universe->AddJump( this->m_nodeD, this->m_nodeB, i++ );

        this->m_universe->AddJump( this->m_nodeD, this->m_nodeE, i++ );
        this->m_universe->AddJump( this->m_nodeE, this->m_nodeD, i++ );

        this->m_universe->AddJump( this->m_nodeA, this->m_nodeC, i++ );
        this->m_universe->AddJump( this->m_nodeC, this->m_nodeA, i++ );

        this->m_universe->AddJump( this->m_nodeC, this->m_nodeE, i++ );
        this->m_universe->AddJump( this->m_nodeE, this->m_nodeC, i++ );

        this->m_universe->FinalizeMap();

        // Init m_cache for use with m_nodeA map. There may be multiple caches
        this->m_cache->Initialize( *this->m_universe, 15 );

        this->m_cache->ClearCache();	
    };

    virtual void TearDown()
    {
        delete this->m_universe;
        delete this->m_goal;
        delete this->m_cache;
    };

    std::string GetNodeName(EveMapNodeID node){
        if(node == this->m_nodeA)
            return "A";
        else if(node == this->m_nodeB)
            return "B";
        else if(node == this->m_nodeC)
            return "C";
        else if(node == this->m_nodeD)
            return "D";
        else if(node == this->m_nodeE)
            return "E";
        throw "Could not find node";
    }    

    EveMap* m_universe;
    EveDjikstrasGoal* m_goal;
    EveMapPathfinderCache* m_cache;
    EveMapNodeID m_nodeA, m_nodeB, m_nodeC, m_nodeD, m_nodeE;
};

// -------------------------------------------------------------
// Description:
//   A class that makes it easier to test jump count between two 
//   systems
// SeeAlso:
//   TestJumpCount, TestJumpCountReversePath
// -------------------------------------------------------------
class EvePathfinderJumpTest: public EvePathfinderTest<::testing::TestWithParam<JumpTestPair>>{
protected:

    virtual EveMapNodeID GetOrigin()
    {
        return this->GetNode(this->GetParam().originSystemName);
    }

    virtual EveMapNodeID GetDestination()
    {
        return this->GetNode(this->GetParam().destinationSystemName);
    }

    virtual unsigned int GetExpectedJumpCount()
    {
        return this->GetParam().expectedJumpCount;
    }
private:
    EveMapNodeID GetNode(const char systemName){
        if(systemName == 'A')
            return this->m_nodeA;
        else if(systemName == 'B')
            return this->m_nodeB;
        else if(systemName == 'C')
            return this->m_nodeC;
        else if(systemName == 'D')
            return this->m_nodeD;
        else if(systemName == 'E')
            return this->m_nodeE;
        throw "Could not find node";
    }
};


TEST_P(EvePathfinderJumpTest, TestJumpCount)
{
    this->m_goal->AddOrigin( this->GetOrigin() );
    this->m_goal->SetGoal( *this->m_universe, this->GetDestination() );

    RunPathfinder( *this->m_universe, *this->m_goal, *this->m_cache );

    EveMapClosedListNodeID closedListID = this->m_universe->GetSolarSystem( this->GetDestination() )->m_closedListID;
    unsigned int jumpCount = this->m_cache->GetCurrentPath(closedListID).m_jumpCountFromOrigin;
    ASSERT_EQ(this->GetExpectedJumpCount(), jumpCount) << "The jump count is invalid";
}


TEST_P(EvePathfinderJumpTest, TestJumpCountReversePath)
{
    this->m_goal->AddOrigin( this->GetDestination() );
    this->m_goal->SetGoal( *this->m_universe, this->GetOrigin() );

    RunPathfinder( *this->m_universe, *this->m_goal, *this->m_cache );

    EveMapClosedListNodeID closedListID = this->m_universe->GetSolarSystem( this->GetOrigin() )->m_closedListID;
    unsigned int jumpCount = this->m_cache->GetCurrentPath(closedListID).m_jumpCountFromOrigin;
    ASSERT_EQ(this->GetExpectedJumpCount(), jumpCount) << "The jump count is invalid";
}

INSTANTIATE_TEST_CASE_P(TestAllJumps, EvePathfinderJumpTest, ::testing::ValuesIn(JUMP_TEST_PAIRS));


// -------------------------------------------------------------
// Description:
//   A class used to test system lookup within a jump count.
// SeeAlso:
//   TestGetSystemsInSingleJump, TestGetSystemsWithinTwoJumps
// -------------------------------------------------------------
class EvePathfinderSystemsWithinRange: public EvePathfinderTest<::testing::Test>
{
public:
    // -------------------------------------------------------------
    // Description:
    //   Creates a string used for outputting the value of the input
    //   vector in a comprehensional manner
    // Arguments:
    //   jumpCountToSystemVector - a vector of JumpCountToSystem
    // Return Value:
    //   A string describing the content of the vector
    // SeeAlso:
    //   TestGetSystemsInSingleJump, TestGetSystemsWithinTwoJumps
    // -------------------------------------------------------------
    std::string GetJumpCountAndSystemStringFromVector( std::vector<JumpCountToSystem> jumpCountToSystemVector)
    {
        std::ostringstream oss;

        for( unsigned int i = 0; i < jumpCountToSystemVector.size(); i++ )
        {
            if( i != 0 )
            {
                oss << ", ";
            }
            oss << GetNodeName(jumpCountToSystemVector[i].m_node) << "[" << jumpCountToSystemVector[i].m_jumpCount << "]";
        }
        return "{ " + oss.str() + " }";
    }
};

TEST_F(EvePathfinderSystemsWithinRange, TestGetSystemsInSingleJump)
{  
    this->m_goal->AddOrigin( this->m_nodeA );

    RunPathfinder( *this->m_universe, *this->m_goal, *this->m_cache );

    std::vector<JumpCountToSystem> systems = std::vector<JumpCountToSystem>();
    m_cache->GetSystemsWithinJumpCount( 1, 2, systems );

    std::vector<JumpCountToSystem> expectedSystems = std::vector<JumpCountToSystem>();
    JumpCountToSystem jumpCountToB = {1, this->m_nodeB };
    JumpCountToSystem jumpCountToC = {1, this->m_nodeC };
    expectedSystems.push_back( jumpCountToB );
    expectedSystems.push_back( jumpCountToC );
    
    ASSERT_THAT( systems, ::testing::ContainerEq( expectedSystems ))  << GetJumpCountAndSystemStringFromVector(systems) 
        << " did not equal the expected " << GetJumpCountAndSystemStringFromVector(expectedSystems);
}


TEST_F(EvePathfinderSystemsWithinRange, TestGetSystemsWithinTwoJumps)
{
    this->m_goal->AddOrigin( this->m_nodeA );

    RunPathfinder( *this->m_universe, *this->m_goal, *this->m_cache );

    std::vector<JumpCountToSystem> systems = std::vector<JumpCountToSystem>();
    m_cache->GetSystemsWithinJumpCount( 1, 3, systems );

    std::vector<JumpCountToSystem> expectedSystems = std::vector<JumpCountToSystem>();
    JumpCountToSystem jumpCountToB = {1, this->m_nodeB };
    JumpCountToSystem jumpCountToC = {1, this->m_nodeC };
    JumpCountToSystem jumpCountToD = {2, this->m_nodeD };
    JumpCountToSystem jumpCountToE = {2, this->m_nodeE };
    expectedSystems.push_back( jumpCountToB );
    expectedSystems.push_back( jumpCountToC );
    expectedSystems.push_back( jumpCountToD );
    expectedSystems.push_back( jumpCountToE );

    ASSERT_THAT( systems, ::testing::ContainerEq( expectedSystems )) << GetJumpCountAndSystemStringFromVector(systems) 
        << " did not equal the expected " << GetJumpCountAndSystemStringFromVector(expectedSystems);
};


int main(int argc, char **argv) 
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#endif
