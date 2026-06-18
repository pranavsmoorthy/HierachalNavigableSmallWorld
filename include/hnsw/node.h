#ifndef HNSW_INCLUDE_NODE_H_
#define HNSW_INCLUDE_NODE_H_

#include "vector_base.h"

#include <set>
#include <algorithm>
#include <stdexcept>

namespace hnsw {
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
                (*n).adjacency_set_.erase(this);
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
        Node(Node&& other) noexcept : data_(*other.data_) {
            delete other.data_;

            for (Node* n : adjacency_set_) {
                n -> adjacency_set_.erase(&other);
                n -> adjacency_set_.insert(this);
            }

            for (vector_base::VectorBase<DataType, DistanceType, Dimensions>* 
                v : other.adjacency_set_) {
                    adjacency_set_.insert(v);
                    delete v;
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
            const vector_base::VectorBase<DataType, DistanceType, Dimensions>* 
            other) {
                delete data_;
                data_ = *other;
        }

        /**
         * Adjacency List Getter:
         * Returns a reference to the Node's adjacency set
         */
        const std::set<Node*>& 
            GetAdjacencySet() const {return adjacency_set_;
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
        std::set<Node*> adjacency_set_;
};


}
}

#endif