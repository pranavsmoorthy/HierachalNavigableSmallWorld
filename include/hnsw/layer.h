#ifndef HNSW_INCLUDE_LAYER_H_
#define HNSW_INCLUDE_LAYER_H_

#include "node.h"
#include "vector_base.h"

#include <set>

namespace hnsw {
namespace layer {

template <typename DataType, typename DistanceType, std::size_t Dimensions,
    std::size_t MaxConnections>
class Layer {
    public:
        /**
         * Default Constructor:
         * Initialized head node to null pointer
         */
        Layer() : head_(nullptr) {};

        /**
         * Destructor:
         * 
         */
        ~Layer() {

        }

        Layer(const Layer& other) = delete;
        Layer& operator=(const Layer& other) = delete;
        Layer(Layer&& other) noexcept = delete;
        Layer& operator=(Layer&& other) noexcept;

        /**
         * Add 
         */

    private:
        /**
         * Start Node:
         * Serves as the default entry node for the layer, makes sure that all 
         * nodes trace back to this node
         */
        node::Node<DataType, DistanceType, Dimensions, MaxConnections>* head_;

        node::Node<DataType, DistanceType, Dimensions, MaxConnections>* TraverseToCoordinate() {
            
        }
};

}
}

#endif