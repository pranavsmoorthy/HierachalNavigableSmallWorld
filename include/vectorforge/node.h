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
    inline void ThrowMaxConnectionsReached() {
        throw std::logic_error("Max connections reached for one or more node");
    }

    inline void ThrowCannotConnectToDeadNode() {
        throw std::logic_error("Cannot create a connection to a dead node");
    }

    inline void ThrowCannotGetFromDeadNode() {
        throw std::logic_error("Cannot get data from a dead node");
    }

    inline void ThrowCannotEditDeadNode() {
        throw std::logic_error("Cannot edit dead node");
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
            : data_(data), dead_(false) {}

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
            dead_(false),
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

            dead_ = false;

            delete data_;
            for (Node* n : adjacency_set_) {
                n -> adjacency_set_.erase(this);
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
         * Returns the data value that the Node holds
         */
        const DataType& GetData() const {
            if (dead_) {
                exceptions::ThrowCannotGetFromDeadNode();
            }

            return data_ -> GetData();
        }

        /**
         * Data Setter:
         * Sets the data to what is given in the function
         */
        void SetData(const DataType& other) {
            if (dead_) {
                exceptions::ThrowCannotEditDeadNode();
            }

            data_ -> SetData(other);
        }

        /**
         * Adjacency List Getter:
         * Returns a reference to the Node's adjacency set
         */
        const std::unordered_set<Node*>& GetAdjacencySet() const {
            return adjacency_set_;
        }

        /**
         * Coordinate Getter:
         * Returns the coordinate of the node's coordinates
         */
        const std::array<DistanceType, Dimensions>& GetCoords() const {
            return data_ -> GetCoords();
        }

        /**
         * Is Dead:
         * Returns if node is dead or not
         */
        bool IsDead() {
            return dead_;
        }

        /**
         * Mark Dead:
         * Effectively marks the node as dead and removes the data, but does
         * not actually delete it (unless it has no adjacent nodes) in order to 
         * maintain connections
         */
        void MarkDead() {
            if (!dead_) {
                dead_ = true;

                if (adjacency_set_.empty()) {
                    delete this;
                } else {
                    for (Node* n : adjacency_set_) {
                        if (!(n -> dead_)) {
                            return;
                        }
                    }

                    delete this;
                }
            }
        }

        /**
         * Resurrect Node:
         * Resurrects a node from dead state and sets the data
         */
        void ResurrectNode(DataType data) {
            if (dead_) {
                dead_ = false;
                data_ -> SetData(data);
            }
        }

        //Adding and Removing Connections
        /**
         * Add Connections: 
         * Adds this node to the other node's adjacency set, and vice versa
         */
        void AddConnection(Node* other) {
            if (dead_ || other -> dead_) {
                exceptions::ThrowCannotConnectToDeadNode();
            }

            if (
                adjacency_set_.size() == MaxConnections || 
                other -> adjacency_set_.size() == MaxConnections
            ) {
                exceptions::ThrowMaxConnectionsReached();
            } 

            adjacency_set_.insert(other);
            other -> adjacency_set_.insert(this);
        }

        /**
         * Sever Connections:
         * Removes this node from other node's adjacency set, and vice versa
         */
        void SeverConnection(Node* other) {
            adjacency_set_.erase(other);
            other -> adjacency_set_.erase(this);

            if ((other -> adjacency_set_).empty() && other -> dead_) {
                delete other;
            }

            if (adjacency_set_.empty() && dead_) {
                delete this;
            }
        }

        /**
         * AtConnectionLimit:
         * Returns if the node has the max connections
         */
        bool AtConnectionLimit() {
            return adjacency_set_.size() >= MaxConnections;
        }

        /**
         * Distance to Other Node:
         * Returns distance to another node
         */
        double DistanceToNode(Node* other) {
            return data_ -> EuclideanDistanceTo(other -> data_ -> GetCoords());
        }

        /**
         * Distance to Coords:
         * Returns distance to a set of coordinates
         */
        double DistanceToCoords(
            const std::array<DistanceType, Dimensions>& other) {
                return data_ -> EuclideanDistanceTo(other);
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

        /**
         * Dead: 
         * Holds whether node is dead or alive
         */
        bool dead_;
};


}
}

#endif