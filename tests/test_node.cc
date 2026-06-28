#include "catch.h"

#include <string>
#include <stdexcept>

#include "../include/vectorforge/node.h"
#include "../include/vectorforge/vector_base.h"

// Create a type alias to keep the tests clean and readable
// Template arguments: <Payload: string, Math: double, 2 Dimensions, Max 3 Connections>
using TestNode = vectorforge::node::Node<std::string, double, 2, 3>;
using TestVector = vectorforge::vector_base::VectorBase<std::string, double, 2>;

TEST_CASE("Node: Initialization and Memory Ownership", "[node]") {
    // 1. Create a heap-allocated vector
    TestVector* vec = new TestVector({1.0, 2.0}, "Payload A");
    
    // 2. Pass ownership to the Node
    TestNode nodeA(vec);

    REQUIRE(nodeA.GetData().GetData() == "Payload A");
    REQUIRE(nodeA.GetAdjacencySet().empty());

    // 3. Test SetData (which should safely delete the old payload)
    TestVector* new_vec = new TestVector({3.0, 4.0}, "Payload B");
    nodeA.SetData(new_vec);
    
    REQUIRE(nodeA.GetData().GetData() == "Payload B");
}

TEST_CASE("Node: Adding and Severing Connections", "[node]") {
    TestNode nodeA(new TestVector({0.0, 0.0}, "Node A"));
    TestNode nodeB(new TestVector({1.0, 1.0}, "Node B"));
    TestNode nodeC(new TestVector({2.0, 2.0}, "Node C"));
    TestNode nodeD(new TestVector({3.0, 3.0}, "Node D"));
    TestNode nodeE(new TestVector({4.0, 4.0}, "Node E"));

    SECTION("Bidirectional Connection Logic") {
        nodeA.AddConnection(&nodeB);

        // Both nodes should now have each other in their adjacency sets
        REQUIRE(nodeA.GetAdjacencySet().size() == 1);
        REQUIRE(nodeB.GetAdjacencySet().size() == 1);

        REQUIRE(nodeA.GetAdjacencySet().count(&nodeB) == 1);
        REQUIRE(nodeB.GetAdjacencySet().count(&nodeA) == 1);

        // Severing the connection should remove the pointers from both sets
        nodeA.SeverConnection(&nodeB);
        REQUIRE(nodeA.GetAdjacencySet().empty());
        REQUIRE(nodeB.GetAdjacencySet().empty());
    }

    SECTION("Enforcing Max Connections Exception") {
        // Our MaxConnections template parameter is set to 3.
        nodeA.AddConnection(&nodeB);
        nodeA.AddConnection(&nodeC);
        nodeA.AddConnection(&nodeD);

        // The 4th connection should throw the logic_error we defined
        REQUIRE_THROWS_AS(nodeA.AddConnection(&nodeE), std::logic_error);
    }
}

TEST_CASE("Node: Move Semantics and Neighbor Pointer Updates", "[node]") {
    TestNode nodeA(new TestVector({1.0, 1.0}, "Node A"));
    TestNode nodeB(new TestVector({2.0, 2.0}, "Node B"));
    
    // Connect A and B
    nodeA.AddConnection(&nodeB);

    SECTION("Move Assignment Operator (Tests Selected Code)") {
        TestNode nodeC(new TestVector({3.0, 3.0}, "Node C (To be overwritten)"));
        
        // This triggers the specific code block selected in the Canvas.
        // nodeC steals nodeA's memory, AND nodeB must be updated to point to nodeC!
        nodeC = std::move(nodeA);

        // 1. Verify Node C stole the payload successfully
        REQUIRE(nodeC.GetData().GetData() == "Node A");
        
        // 2. Verify Node B's pointers were updated successfully
        REQUIRE(nodeB.GetAdjacencySet().count(&nodeA) == 0); // B no longer points to A
        REQUIRE(nodeB.GetAdjacencySet().count(&nodeC) == 1); // B now points to C!
        
        // 3. Verify Node C acquired the connections
        REQUIRE(nodeC.GetAdjacencySet().count(&nodeB) == 1);
    }

    SECTION("Move Constructor") {
        // Triggers the Move Constructor instead
        TestNode nodeD(std::move(nodeA));

        // B should no longer point to A, it should point to D
        REQUIRE(nodeB.GetAdjacencySet().count(&nodeA) == 0);
        REQUIRE(nodeB.GetAdjacencySet().count(&nodeD) == 1);
        REQUIRE(nodeD.GetAdjacencySet().count(&nodeB) == 1);
    }
}