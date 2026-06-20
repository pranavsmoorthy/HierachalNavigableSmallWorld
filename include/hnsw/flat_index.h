#ifndef HNSW_INCLUDE_FLAT_INDEX_H_
#define HNSW_INCLUDE_FLAT_INDEX_H_

#include "vector_base.h"

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <queue>

namespace hnsw {
namespace flat_index {

/**
 * This is meant for testing. The class will be representing a linear search
 * and will be used to compare against the final HNSW graph.
 */
template <typename DataType, typename DistanceType, std::size_t Dimensions>
class FlatIndex {
    using VectorBaseType = vector_base::VectorBase<DataType, DistanceType, Dimensions>;

    public:
        //Default Constructor
        FlatIndex() {}

        //Rule of 5:
        /**
         * Destructor:
         * Goes through the data vector an removes all pointers to free the 
         * memory
         */
        ~FlatIndex() {
            for (VectorBaseType* v : data_) {
                delete v;
            }
        }

        /**
         * Copy Constructor:
         * Goes through the other object, sets the size constraint, and goes through
         * data vector, creates a new copy of every VectorBase object and pushes it to
         * current data vector
         */
        FlatIndex(const FlatIndex& other) {
            data_.reserve(other.data_.size());

            for (VectorBaseType* v : other.data_) {
                data_.push_back(new VectorBaseType(*v));
            }
        }

        /**
         * Copy Assignment:
         * Overrides the = operator. Deletes all pre existing data, replaces 
         * the size constraints, and then repopulates the current data array 
         * with the data in the other object
         */
        FlatIndex& operator=(const FlatIndex& other) {
            if (this == &other) {
                return *this;
            }

            for (VectorBaseType* v : data_) {
                delete v;
            }

            data_.clear();

            for (VectorBaseType* v : other.data_) {
                data_.push_back(new VectorBaseType(*v));
            }

            return *this;
        }

        /**
         * Move Constructor:
         * Sets current size constraint to other size constraint, then adds 
         * shallow copies of the other data to current data vector array, 
         * replaces the data with null pointers
         */
        FlatIndex(FlatIndex&& other) noexcept 
            : data_(std::move(other.data_)) {}

        /**
         * Move Assignment:
         * Clears out current data vector, sets the size constraint, and 
         * repopulates the data vector
         */
        FlatIndex& operator=(FlatIndex&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            for (VectorBaseType* v : data_) {
                delete v;
            }

            data_ = std::move(other.data_);

            return *this;
        }
        
        //Getters and Setters:
        /**
         * Data Getter:
         * Returns data vector
         */
        const std::vector<VectorBaseType*>& GetData() const {
            return data_;
        }

        /**
         * Number of Coordinates Getter:
         * Returns the number of coordinates in the VectorBase object
         */
        std::size_t GetDimensions() const {
            return Dimensions;
        }

        /**
         * Data Adder:
         * Takes a reference to a VectorBase object, creates a pointer out of 
         * that reference, and adds it to the data vector. Throws an error if 
         * the number of coordinates in the vector object differs from the 
         * size constraint
         */
        void AddData(
            const vector_base::VectorBase<DataType, DistanceType, Dimensions>& 
            vector_reference) {
                data_.push_back(new VectorBaseType(vector_reference));
        }

        /**
         * Search:
         * Performs a linear search on the data vector looking for the k nearest 
         * vectors to the query. For testing/benchmarking purposes
         */
        std::vector<DataType> Search(
            const std::array<DistanceType, Dimensions>& query, 
            std::size_t k) const {
                if (k == 0 || data_.empty()) {
                    return {};
                }

                std::priority_queue<std::pair<double, VectorBaseType*>> queue;

                for (VectorBaseType* v : data_) {
                        double distance = v -> EuclideanDistanceTo(query);
                        queue.push({distance, v});

                        if (queue.size() > k) {
                            queue.pop();
                        }
                }

                std::vector<DataType> results;
                results.reserve(k);

                while (!queue.empty()) {
                    results.push_back((*queue.top().second).GetData());
                    queue.pop();
                }

                std::reverse(results.begin(), results.end());

                return results;
        }
    
    private:
        /**
         * The data vector
         */
        std::vector<
            vector_base::VectorBase<DataType, DistanceType, Dimensions>*
            > data_;
};

};
};

#endif