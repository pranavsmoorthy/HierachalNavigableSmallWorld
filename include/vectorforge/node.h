#ifndef VF_INCLUDE_NODE_H_
#define VF_INCLUDE_NODE_H_

#include "vector_base.h"

#include <unordered_set>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <queue>

namespace vectorforge {
namespace node {

namespace exceptions {
    void ThrowMaxConnectionsReached() {
        throw std::logic_error("Max connections reached for one or more node");
    }
}

/**
 * Node:
 * Represents a single point in the graph
 */
template <typename DataType, typename DistanceType, std::size_t Dimensions,
    std::size_t MaxConnections>
class Node {    
    public:
        //Constructor
        /**
         * Constructor:
         * Takes a VectorBase as the data point it holds
         */
        Node(vector_base::VectorBase<DataType, DistanceType, Dimensions>* data) 
            : data_(data) {}

        //Rule of 5:
        /**
         * Destructor:
         * Deletes the data point. Clears adjacency set.
         */
        ~Node() {
            delete data_;
            
            for (Node* n : adjacency_set_) {
                (n -> adjacency_set_).erase(this);
            }

            adjacency_set_.clear();
        }

        Node(const Node& other) = delete;
        Node& operator=(const Node& other) = delete;

        /**
         * Move Constructor:
         * Adds the data and adjacency list of this Node with that of the other
         * Node and deletes the other Node's properties
         */
        Node(Node&& other) noexcept : 
            data_(other.data_), 
            adjacency_set_(std::move(other.adjacency_set_)) {
                other.data_ = nullptr;

                for (Node* n : adjacency_set_) {
                    n -> adjacency_set_.erase(&other);
                    n -> adjacency_set_.insert(this);
                }
        }

        /**
         * Move Assignment:
         * Clears out current data and adjacency set, and repopulates the 
         * adjacency set with the other Node's adjacency set and replaces the
         * data
         */
        Node& operator=(Node&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            delete data_;
            for (Node* n : adjacency_set_) {
                n->adjacency_set_.erase(this);
            }

            data_ = other.data_;
            adjacency_set_ = std::move(other.adjacency_set_);
            other.data_ = nullptr;

            for (Node* n : adjacency_set_) {
                n -> adjacency_set_.erase(&other);
                n -> adjacency_set_.insert(this);
            }

            return *this;
        }

        //Getters and Setters
        /**
         * Data Getter
         * Returns the vector base that the Node holds
         */
        const vector_base::VectorBase<DataType, DistanceType, Dimensions>& 
            GetData() const {
                return *data_;
        }

        /**
         * Data Setter:
         * Sets the data to what is given in the function
         */
        void SetData(
            vector_base::VectorBase<DataType, DistanceType, Dimensions>* 
            other) {
                delete data_;
                data_ = other;
        }

        /**
         * Adjacency List Getter:
         * Returns a reference to the Node's adjacency set
         */
        const std::unordered_set<Node*>& 
            GetAdjacencySet() const {
                return adjacency_set_;
        }

        //Adding and Removing Connections
        /**
         * Add Connections: 
         * Adds this node to the other node's adjacency set, and vice versa
         */
        void AddConnection(Node& other) {
            if (
                adjacency_set_.size() == MaxConnections || 
                other.adjacency_set_.size() == MaxConnections
            ) {
                exceptions::ThrowMaxConnectionsReached();
            } 

            adjacency_set_.insert(&other);
            other.adjacency_set_.insert(this);
        }

        /**
         * Sever Connections:
         * Removes this node from other node's adjacency set, and vice versa
         */
        void SeverConnection(Node& other) {
            adjacency_set_.erase(&other);
            other.adjacency_set_.erase(this);
        }

        /**
         * AtConnectionLimit:
         * Returns if the node has the max connections
         */
        boolean AtConnectionLimit() {
            return adjacency_set_.size() >= MaxConnections;
        }

        /**
         * 
         */
        std::vector<Node*> LookForRemovableNodes(Node* new_node) {
            std::vector<Node*> removable_nodes;

            if (adjacency_set_.size() < MaxConnections) {
                return removable_nodes;
            }

            std::vector<Node*> kept_nodes;
            std::priority_queue<std::pair<double, Node*>> distance_queue;
            distance_queue.push({data_ -> EuclideanDistanceTo((new_node -> GetData()).GetCoords()), new_node});

            for (Node* n : adjacency_set_) {
                distance_queue.push({n -> EuclideanDistanceTo((n -> GetData()).GetCoords()), n});
            }

            std::vector<Node*> sorted_node_vector;

            while (!distance_queue.empty()) {
                sorted_node_vector.push_back(distance_queue.top().second);
                distance_queue.pop();
            }

            kept_nodes.push_back(sorted_node_vector.back());
            sorted_node_vector.pop_back();

            while (!sorted_node_vector.empty()) {
                Node* candidate = sorted_node_vector.back();
                sorted_node_vector.pop_back();

                bool can_remove = false;

                if (kept_nodes.size() >= MaxConnections) {
                    can_remove = true;
                } else {
                    // Diversity Check
                    for (Node* kept_node : kept_nodes) {
                        double dist_to_kept = candidate->GetData().EuclideanDistanceTo(kept_node->GetData());
                        double dist_to_hub = data_->EuclideanDistanceTo(candidate->GetData());
                        
                        if (dist_to_kept < dist_to_hub) {
                            can_remove = true;
                            break;
                        }
                    }
                }

                if (can_remove) {
                    removable_nodes.push_back(candidate);
                } else {
                    kept_nodes.push_back(candidate);
                }
            }

            return removable_nodes;
        }

    private:
        /**
         * Data:
         * A pointer to the actual VectorBase the node object represents
         */
        vector_base::VectorBase<DataType, DistanceType, Dimensions>* data_;

        /**
         * Adjacency List:
         * The Node's connection in the graph
         */
        std::unordered_set<Node*> adjacency_set_;
};


}
}

#endif