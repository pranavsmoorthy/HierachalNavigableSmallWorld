#include "catch.h"

#include "../include/vectorforge/cluster.h"
#include "../include/vectorforge/node.h"
#include "../include/vectorforge/vector_base.h"

#include <string>
#include <stdexcept>

// Your project uses a mix of the 'vectorforge' and 'vectorforge' namespaces. 
// We bring them into scope here for clean test definitions.
using namespace vectorforge::cluster;
using namespace vectorforge::node;
using namespace vectorforge::vector_base;

// Type Aliases to keep the tests readable
using TestVector = VectorBase<std::string, double, 2>;
using TestNode = Node<std::string, double, 2, 3>;
using TestCluster = Cluster<std::string, double, 2, 3>;

TEST_CASE("Cluster: Initialization and Head Node", "[cluster]") {
    TestCluster cluster;

    // 1. Verify default state
    REQUIRE(cluster.GetHeadNode() == nullptr);

    // 2. Add the first node (should become the head)
    TestNode* head_node = new TestNode(new TestVector({0.0, 0.0}, "Head Payload"));
    
    // AddNode should return an empty vector (no islands created)
    REQUIRE(cluster.AddNode(head_node).empty());

    REQUIRE(cluster.GetHeadNode() != nullptr);
    REQUIRE(cluster.GetHeadNode()->GetData() == "Head Payload");
}

TEST_CASE("Cluster: AddNode Exception Handling", "[cluster]") {
    TestCluster cluster;

    // Create two nodes and connect them outside the cluster
    TestNode* nodeA = new TestNode(new TestVector({1.0, 1.0}, "A"));
    TestNode* nodeB = new TestNode(new TestVector({2.0, 2.0}, "B"));
    nodeA -> AddConnection(nodeB);

    // Attempting to add a node that already has connections should trigger 
    // the exceptions::ThrowNodeExistsInGraph() logic!
    REQUIRE_THROWS_AS(cluster.AddNode(nodeA), std::logic_error);
    
    // Clean up our manual nodes
    delete nodeA; 
    delete nodeB;
}

TEST_CASE("Cluster: Meta-Graph Connections", "[cluster]") {
    TestCluster clusterA;
    TestCluster clusterB;
    TestCluster clusterC;
    TestCluster clusterD;
    TestCluster clusterE;

    SECTION("Bidirectional Cluster Linking") {
        clusterA.AddConnection(&clusterB);

        // Both clusters should now be linked in the Meta-Graph
        REQUIRE(clusterA.GetAdjacencySet().size() == 1);
        REQUIRE(clusterB.GetAdjacencySet().size() == 1);

        // Sever the link
        clusterA.SeverConnection(&clusterB);
        REQUIRE(clusterA.GetAdjacencySet().empty());
        REQUIRE(clusterB.GetAdjacencySet().empty());
    }
}

TEST_CASE("Cluster: Empty / Single Node Search Behaviors", "[cluster]") {
    TestCluster cluster;
    std::array<double, 2> query = {5.0, 5.0};

    SECTION("Searching an empty cluster safely returns empty/null") {
        REQUIRE(cluster.FindNearestNode(query) == nullptr);
        
        auto k_results = cluster.FindNearestKNodes(query, 3);
        REQUIRE(k_results.empty());
    }

    SECTION("Searching a cluster with only a head node") {
        TestNode* head_node = new TestNode(new TestVector({0.0, 0.0}, "Origin"));
        cluster.AddNode(head_node);

        // Single Nearest Node
        TestNode* result = cluster.FindNearestNode(query);
        REQUIRE(result != nullptr);
        REQUIRE(result->GetData() == "Origin");

        // K-Nearest Nodes (asking for 3, but only 1 exists)
        auto k_results = cluster.FindNearestKNodes(query, 3);
        REQUIRE(k_results.size() == 1);
        REQUIRE(k_results[0]->GetData() == "Origin");
    }
}

TEST_CASE("Cluster: Multi-Node Search Routing", "[cluster]") {
    TestCluster cluster;

    // Create a path of nodes
    TestNode* nodeA = new TestNode(new TestVector({0.0, 0.0}, "Node A")); // Head
    TestNode* nodeB = new TestNode(new TestVector({1.0, 1.0}, "Node B"));
    TestNode* nodeC = new TestNode(new TestVector({2.0, 2.0}, "Node C"));
    TestNode* nodeD = new TestNode(new TestVector({3.0, 3.0}, "Node D"));


    // Set Node A as the entry point for the cluster
    cluster.AddNode(nodeA);

    // Manually link the nodes to form a traversable graph: A <-> B <-> C <-> D
    nodeA->AddConnection(nodeB);
    nodeB->AddConnection(nodeC);
    nodeC->AddConnection(nodeD);

    SECTION("Greedy Search for Single Nearest Node") {
        // Query coordinate closer to Node C (2.0, 2.0)
        std::array<double, 2> query = {2.1, 2.1};
        
        auto* result = cluster.FindNearestNode(query);
        
        // The search should traverse A -> B -> C and stop at C as the local minimum
        REQUIRE(result != nullptr);
        REQUIRE(result->GetData() == "Node C");
    }

    SECTION("Greedy Search for K-Nearest Nodes") {
        // Query coordinate closer to Node B (1.0, 1.0)
        std::array<double, 2> query = {1.1, 1.1};
        
        auto k_results = cluster.FindNearestKNodes(query, 3);
        
        REQUIRE(k_results.size() == 3);
        REQUIRE(k_results[0]->GetData() == "Node B"); // Closest
        REQUIRE(k_results[1]->GetData() == "Node C"); // 2nd Closest
        REQUIRE(k_results[2]->GetData() == "Node A"); // 3rd Closest
    }
}

TEST_CASE("Cluster: Dynamic AddNode Connection Logic", "[cluster]") {
    TestCluster cluster;

    TestNode* nodeA = new TestNode(new TestVector({0.0, 0.0}, "Node A (Head)"));
    TestNode* nodeB = new TestNode(new TestVector({2.0, 2.0}, "Node B"));
    TestNode* nodeC = new TestNode(new TestVector({2.1, 2.1}, "Node C"));

    SECTION("Connecting to an empty cluster") {
        // Adding head node should not return any isolated nodes
        REQUIRE(cluster.AddNode(nodeA).empty());
        REQUIRE(cluster.GetHeadNode() == nodeA);
    }

    SECTION("Connecting to a populated cluster (Multiple K-NN Links)") {
        cluster.AddNode(nodeA);
        
        REQUIRE(cluster.AddNode(nodeB).empty());
        REQUIRE(nodeB->GetAdjacencySet().count(nodeA) == 1);

        // Node C should connect to BOTH A and B without causing isolation
        REQUIRE(cluster.AddNode(nodeC).empty());
        
        REQUIRE(nodeC->GetAdjacencySet().count(nodeB) == 1); // Closest
        REQUIRE(nodeC->GetAdjacencySet().count(nodeA) == 1); // Second closest
        
        REQUIRE(nodeB->GetAdjacencySet().count(nodeC) == 1);
        REQUIRE(nodeA->GetAdjacencySet().count(nodeC) == 1);
    }

    SECTION("Connecting a node with identical coordinates throws an exception") {
        cluster.AddNode(nodeA); // Is at (0.0, 0.0)

        // Attempting to add a node with the exact same geometric coordinates
        TestNode* duplicateNode = new TestNode(new TestVector({0.0, 0.0}, "Duplicate Payload"));
        
        // This should trigger the exceptions::ThrowNodeWithCoordExist() error!
        REQUIRE_THROWS_AS(cluster.AddNode(duplicateNode), std::logic_error);

        // Clean up memory since it was safely rejected from the graph
        delete duplicateNode; 
    }

    SECTION("Connecting to a full cluster triggers Routability Test (The Island Effect)") {
        // Setup a fully connected clique of 4 nodes (MaxConnections is 3)
        cluster.AddNode(nodeA); // (0.0, 0.0)
        cluster.AddNode(nodeB); // (2.0, 2.0)
        cluster.AddNode(nodeC); // (2.1, 2.1)

        TestNode* nodeD = new TestNode(new TestVector({10.0, 10.0}, "Node D (Outlier)"));
        REQUIRE(cluster.AddNode(nodeD).empty());

        // At this point, A, B, C, and D all have exactly 3 connections. They are completely full!
        REQUIRE(nodeA->GetAdjacencySet().size() == 3);
        REQUIRE(nodeD->GetAdjacencySet().size() == 3);

        // Insert Node E right into the middle of the A-B-C dense cluster
        TestNode* nodeE = new TestNode(new TestVector({1.0, 1.0}, "Node E"));
        
        // E will connect to its 3 nearest neighbors (A, B, C).
        // Since A, B, C are full, they must bidirectionally sever their FURTHEST connection (Node D).
        // Because D is completely severed, the Routability Test will fail to find it,
        // and AddNode MUST return D in the isolated_nodes vector!
        std::vector<TestNode*> isolated_nodes = cluster.AddNode(nodeE);

        // Verify the Island was caught!
        REQUIRE(isolated_nodes.size() == 1);
        REQUIRE(isolated_nodes[0] == nodeD);
        
        // Verify D is truly isolated
        REQUIRE(nodeD->GetAdjacencySet().empty());
        
        // Verify E successfully integrated
        REQUIRE(nodeE->GetAdjacencySet().size() == 3);
    }

    SECTION("Connecting a worse node to a full cluster is rejected (Distance Heuristic)") {
        // Setup a fully connected clique of 4 nodes
        cluster.AddNode(nodeA); // (0.0, 0.0)
        cluster.AddNode(nodeB); // (2.0, 2.0)
        cluster.AddNode(nodeC); // (2.1, 2.1)
        
        // Add a 4th tight node so everyone hits MaxConnections (3)
        TestNode* tightNode = new TestNode(new TestVector({1.0, 1.0}, "Tight Node"));
        cluster.AddNode(tightNode);

        // Now insert a massive outlier.
        TestNode* outlierNode = new TestNode(new TestVector({100.0, 100.0}, "Far Outlier"));
        
        // The nearest_k_nodes will return A, B, C (or tightNode). 
        // BUT because outlierNode is further away than their existing connections,
        // your new `if (distance < max_distance)` check will fail.
        // They will refuse to sever their good connections!
        cluster.AddNode(outlierNode);

        // Verify the outlier was completely rejected by the full nodes
        REQUIRE(outlierNode->GetAdjacencySet().empty());
        
        // Verify the tight cluster remained perfectly intact and didn't sever anyone
        REQUIRE(nodeA->GetAdjacencySet().size() == 3);
        REQUIRE(tightNode->GetAdjacencySet().size() == 3);
    }

    SECTION("Connecting a node with identical coordinates to a dead node resurrects it") {
        cluster.AddNode(nodeA); // (0.0, 0.0)
        cluster.AddNode(nodeB); // (2.0, 2.0)
        
        // Mark nodeB as dead. Because it is connected to nodeA, its adjacency 
        // set is NOT empty, so it safely remains in the cluster's memory as a bridge.
        nodeB->MarkDead();
        REQUIRE(nodeB->IsDead() == true);
        
        // Attempt to insert a new node at the exact same geometric coordinates as the dead nodeB
        TestNode* resurrectingNode = new TestNode(new TestVector({2.0, 2.0}, "Resurrected Payload"));
        
        // AddNode should catch the dead node at this coordinate, resurrect it, and return no isolated nodes
        REQUIRE(cluster.AddNode(resurrectingNode).empty());
        
        // Verify nodeB was brought back to life and stole the new payload
        REQUIRE(nodeB->IsDead() == false);
        REQUIRE(nodeB->GetData() == "Resurrected Payload");
        
        // Clean up the resurrectingNode pointer! Because your Cluster simply took its payload
        // and resurrected the old node, the new Node object itself was rejected and must be deleted.
        delete resurrectingNode;
    }
}

TEST_CASE("Cluster: Optional Dead Node Inclusion in Search", "[cluster]") {
    TestCluster cluster;

    TestNode* nodeA = new TestNode(new TestVector({0.0, 0.0}, "Origin"));
    TestNode* nodeB = new TestNode(new TestVector({2.0, 2.0}, "Bridge"));
    TestNode* nodeC = new TestNode(new TestVector({4.0, 4.0}, "Destination"));

    // Add nodes linearly
    cluster.AddNode(nodeA);
    cluster.AddNode(nodeB);
    cluster.AddNode(nodeC);

    // Mark the middle node as dead
    nodeB->MarkDead();

    SECTION("Default search excludes dead nodes (Soft Deletion)") {
        // Query exactly at nodeB's location
        std::array<double, 2> query = {2.0, 2.0};
        
        // Because include_dead_nodes defaults to false, it should completely ignore B
        // and return one of the alive nodes (A or C).
        TestNode* result = cluster.FindNearestNode(query); 
        
        REQUIRE(result != nullptr);
        REQUIRE(result->IsDead() == false);
        REQUIRE(result != nodeB);
    }

    SECTION("Search with include_dead_nodes = true successfully returns dead nodes") {
        std::array<double, 2> query = {2.0, 2.0};
        
        // Explicitly pass true for the include_dead_nodes flag
        TestNode* result = cluster.FindNearestNode(query, true);
        
        // The search should now "see" B as a valid endpoint and return it!
        REQUIRE(result != nullptr);
        REQUIRE(result->IsDead() == true);
        REQUIRE(result == nodeB);
    }
}

TEST_CASE("Cluster: Coordinate-Based DeleteNode & Head Reassignment", "[cluster]") {
    TestCluster cluster;

    TestNode* nodeA = new TestNode(new TestVector({0.0, 0.0}, "Origin (Head)"));
    TestNode* nodeB = new TestNode(new TestVector({5.0, 5.0}, "Alive Neighbor"));
    TestNode* nodeC = new TestNode(new TestVector({-5.0, -5.0}, "Dead Neighbor"));

    cluster.AddNode(nodeA);
    cluster.AddNode(nodeB);
    cluster.AddNode(nodeC);

    // Pre-kill nodeC so we can test that the head_ reassignment ignores it
    std::array<double, 2> coordsC = {-5.0, -5.0};
    cluster.DeleteNode(coordsC);
    REQUIRE(nodeC->IsDead() == true);

    SECTION("Successfully deletes a non-head node by coordinate") {
        REQUIRE(nodeB->IsDead() == false);

        std::array<double, 2> query_coords = {5.0, 5.0};
        cluster.DeleteNode(query_coords);

        // Verify that the actual node in the graph was correctly identified and marked dead
        REQUIRE(nodeB->IsDead() == true);
    }

    SECTION("Deleting the head node safely reassigns head to an ALIVE neighbor") {
        REQUIRE(cluster.GetHeadNode() == nodeA);
        REQUIRE(nodeA->IsDead() == false);

        std::array<double, 2> head_coords = {0.0, 0.0};
        cluster.DeleteNode(head_coords);

        REQUIRE(nodeA->IsDead() == true);
        
        // The head must have been automatically reassigned to nodeB!
        // Your logic successfully skipped nodeC because it was already dead.
        REQUIRE(cluster.GetHeadNode() == nodeB);
        REQUIRE(cluster.GetHeadNode()->IsDead() == false);
    }

    SECTION("Throws an exception when deleting a node that doesn't exist") {
        std::array<double, 2> far_coords = {2e9, 2e9};

        // Use Catch2's REQUIRE_THROWS to catch your exceptions::ThrowCouldNotFindNode()
        REQUIRE_THROWS(cluster.DeleteNode(far_coords));
    }
}

TEST_CASE("Cluster: Self-Destruction on Empty Head Deletion", "[cluster]") {
    // Because your DeleteNode calls `delete this;` when the head has no neighbors,
    // we MUST allocate the cluster on the heap here to avoid a stack segmentation fault!
    TestCluster* heap_cluster = new TestCluster();
    
    TestNode* lonelyNode = new TestNode(new TestVector({0.0, 0.0}, "Lonely Head"));
    heap_cluster->AddNode(lonelyNode);

    REQUIRE(heap_cluster->GetHeadNode() == lonelyNode);

    std::array<double, 2> head_coords = {0.0, 0.0};
    
    // This will find the lonely node, see it has no neighbors, and trigger `delete this;`.
    // If the test suite finishes without crashing, the self-deletion was successful!
    heap_cluster->DeleteNode(head_coords); 
    
    // Safety Note: We cannot safely interact with 'heap_cluster' after this line.
}

