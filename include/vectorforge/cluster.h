#ifndef VF_INCLUDE_CLUSTER_H_
#define VF_INCLUDE_CLUSTER_H_

#include "node.h"
#include "vector_base.h"

#include <unordered_set>
#include <array>
#include <vector>
#include <stdexcept>
#include <queue>
#include <set>
#include <algorithm>

namespace vectorforge {
namespace cluster {

namespace exceptions {
    inline void ThrowNodeWithCoordExist() {
        throw std::logic_error(
            "Node with coordinate already exists");
    }

    inline void ThrowNodeExistsInGraph() {
        throw std::logic_error(
            "Node already exists in a graph, sever all connections to this node to add to a new graph");
    }

    inline void ThrowCouldNotFindNode() {
        throw std::logic_error(
            "Could not find node in graph");
    }
}

template <typename DataType, typename DistanceType, std::size_t Dimensions,
    std::size_t MaxConnectionsForNodes>
class Cluster {
    using NodeType = node::Node<DataType, DistanceType, Dimensions, MaxConnectionsForNodes>;

    public:
        /**
         * Default Constructor:
         * Initialized head node to null pointer
         */
        Cluster() : head_(nullptr) {};

        /**
         * Destructor:
         * Deletes head pointer only because iterating through all nodes can
         * lead to the entire graph getting deleted.
         */
        ~Cluster() {
            head_ = nullptr;
        }

        Cluster(const Cluster& other) = delete;
        Cluster& operator=(const Cluster& other) = delete;
        Cluster(Cluster&& other) noexcept = delete;
        Cluster& operator=(Cluster&& other) noexcept;

        /**
         * Find Nearest One Node:
         * Greedily finds the nearest node to the queried coordinate
         */
        NodeType* FindNearestNode(
            const std::array<DistanceType, Dimensions>& query_coordinates,
            bool include_dead_nodes = false) {
                if (head_ == nullptr) {
                    return nullptr;
                }

                NodeType* current_node = head_;
                std::unordered_set<NodeType*> node_set;
                
                NodeType* best_node = nullptr;
                double best_distance = 0.0; 
                
                while (true) {
                    double current_distance = current_node->DistanceToCoords(query_coordinates);
                    
                    if (include_dead_nodes || !current_node->IsDead()) {
                        if (best_node == nullptr || current_distance < best_distance) {
                            best_distance = current_distance;
                            best_node = current_node;
                        }
                    }

                    NodeType* nearest_in_adjacent = nullptr;

                    for (NodeType* n : current_node->GetAdjacencySet()) {
                        if (node_set.count(n) == 0) {
                            double distance_to_query = n->DistanceToCoords(query_coordinates);

                            if (distance_to_query < current_distance) {
                                current_distance = distance_to_query;
                                nearest_in_adjacent = n;
                            }
                        }
                    }

                    if (nearest_in_adjacent == nullptr) {
                        return best_node;
                    } else {
                        node_set.insert(current_node);
                        current_node = nearest_in_adjacent;
                    }
                }

                return nullptr;
        }

        /**
         * Find Nearest K Nodes:
         * Greedily finds the nearest K nodes to the given coordinates and 
         * returns a vector of such pointers.
         */
        std::vector<NodeType*> FindNearestKNodes(
            const std::array<DistanceType, Dimensions>& query_coordinates, 
            const size_t k,
            bool include_dead_nodes = false) {
                std::vector<NodeType*> results;

                if (k == 0 || head_ == nullptr) {
                    return results;
                }

                results.reserve(k);

                std::priority_queue<std::pair<double, NodeType*>> node_queue;
                node_queue.push({
                    head_ -> DistanceToCoords(query_coordinates), 
                    head_
                });

                NodeType* current_node = head_;
                std::unordered_set<NodeType*> node_set;
                node_set.insert(head_);
                
                while (true) {
                    double current_distance = 
                        current_node -> DistanceToCoords(query_coordinates);

                    NodeType* nearest_in_adjacent = nullptr;

                    for (NodeType* n : current_node -> GetAdjacencySet()) {
                        if (node_set.count(n) == 0) {
                            double distance_to_query = 
                                n -> DistanceToCoords(query_coordinates);

                            if (include_dead_nodes || !(n -> IsDead())) {
                                node_queue.push({distance_to_query, n});

                                if (node_queue.size() > k) {
                                    node_queue.pop();
                                }

                                if (distance_to_query < current_distance) {
                                    current_distance = distance_to_query;
                                    nearest_in_adjacent = n;
                                }
                            }

                            node_set.insert(n);
                        }
                    }

                    if (nearest_in_adjacent == nullptr) {
                        break;
                    } else {
                        current_node = nearest_in_adjacent;
                    }
                }

                while (!node_queue.empty()) {
                    results.push_back(node_queue.top().second);
                    node_queue.pop();
                }

                std::reverse(results.begin(), results.end());

                return results;
        }

        /**
         * Adjacency List Getter:
         * Returns a reference to the Cluster's adjacency set
         */
        const std::unordered_set<Cluster*>& 
            GetAdjacencySet() const {
                return adjacency_set_;
            }

        /**
         * Get Head Node:
         * Returns the head Node
         */
        NodeType* GetHeadNode() {
            return head_;
        }

        //Adding and Removing Connections
        /**
         * Add Connections: 
         * Adds this cluster to the other cluster's adjacency set, and vice 
         * versa
         */
        void AddConnection(Cluster* other) {
            adjacency_set_.insert(other);
            other -> adjacency_set_.insert(this);
        }

        /**
         * Sever Connections:
         * Removes this cluster from other cluster's adjacency set, and vice 
         * versa
         */
        void SeverConnection(Cluster* other) {
            adjacency_set_.erase(other);
            other -> adjacency_set_.erase(this);
        }

        /**
         * Add Node:
         * Adds a node to the graph. Returns a vector containing pointers to 
         * isolated nodes / groups of nodes
         */
        std::vector<NodeType*> AddNode(NodeType* node) {
            if ((node -> GetAdjacencySet()).size() != 0 || node -> IsDead()) {
                exceptions::ThrowNodeExistsInGraph();
            }

            std::vector<NodeType*> isolated_nodes;

            if (head_ == nullptr) {
                head_ = node;
                return isolated_nodes;
            }

            NodeType* closest_existing = FindNearestNode(node -> GetCoords(), true);

            if (closest_existing != nullptr) {
                double distance = closest_existing -> DistanceToNode(node);

                if (distance < 1e-9) { 
                    if (closest_existing -> IsDead()) {
                        closest_existing -> ResurrectNode(node -> GetData());
                        return isolated_nodes;
                    }

                    exceptions::ThrowNodeWithCoordExist();
                }
            }

            std::vector<NodeType*> nearest_k_nodes = FindNearestKNodes(
                node -> GetCoords(), MaxConnectionsForNodes);

            bool connection_severed = false;

            for (NodeType* n : nearest_k_nodes) {
                if (n -> AtConnectionLimit()) {
                    std::unordered_set<NodeType*> node_adjacency_set = 
                        n -> GetAdjacencySet();

                    NodeType* furthest_node = nullptr;
                    double max_distance = 0.0;

                    for (NodeType* a : node_adjacency_set) {
                        double current_distance = n -> DistanceToNode(a);

                        if (current_distance > max_distance) {
                            max_distance = current_distance;
                            furthest_node = a;
                        }
                    }

                    if (n -> DistanceToNode(node) < max_distance) {
                        n -> SeverConnection(furthest_node);

                        if (!connection_severed) {
                            connection_severed = true;
                        }

                        if (std::find(isolated_nodes.begin(), isolated_nodes.end(), furthest_node) == isolated_nodes.end() 
                            && furthest_node != FindNearestNode(furthest_node -> GetCoords())) {
                                isolated_nodes.push_back(furthest_node);
                        }

                        node -> AddConnection(n);
                    }
                } else {
                    node -> AddConnection(n);
                }                
            }

            if ((node -> GetAdjacencySet()).empty() 
                || (connection_severed && node != FindNearestNode(node -> GetCoords()))) {
                    isolated_nodes.push_back(node);
            }

            return isolated_nodes;
        }

        /**
         * Remove Node:
         * Marks the node as dead, but does not delete it
         */
        void DeleteNode(NodeType* node) {
            NodeType* existing_node = FindNearestNode(node -> GetCoords());

            if (existing_node -> DistanceToNode(node) <= 1e-9) {
                if (existing_node == head_) {
                        std::unordered_set<NodeType*> node_set = existing_node -> GetAdjacencySet();

                        if (node_set.empty()) {
                            delete this;
                            return;
                        }

                        for (NodeType* n : node_set) {
                            if (!(n -> IsDead())) {
                                head_ = n;
                            }
                        }
                    }

                existing_node -> MarkDead();
            } else {
                exceptions::ThrowCouldNotFindNode();
            }
        }

        void DeleteNode(
            const std::array<DistanceType, Dimensions>& query_coordinates) {
                NodeType* existing_node = FindNearestNode(query_coordinates);

                if (existing_node -> DistanceToCoords(
                    query_coordinates) <= 1e-9) {
                        if (existing_node == head_) {
                            std::unordered_set<NodeType*> node_set = existing_node -> GetAdjacencySet();

                            if (node_set.empty()) {
                                delete this;
                                return;
                            }

                            for (NodeType* n : node_set) {
                                if (!(n -> IsDead())) {
                                    head_ = n;
                                }
                            }
                        }

                        existing_node -> MarkDead();
                } else {
                    exceptions::ThrowCouldNotFindNode();
                }
        }

    private:
        /**
         * Start Node:
         * Serves as the default entry node for the cluster, makes sure that all 
         * nodes trace back to this node
         */
        node::Node<DataType, DistanceType, Dimensions, MaxConnectionsForNodes>* head_;

        /**
         * Adjacency Set:
         * A set of the clusters this cluster hasa connection to
         */
        std::unordered_set<Cluster*> adjacency_set_;
};

}
}

#endif