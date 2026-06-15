#include "../include/hnsw/flat_index.h"

#include <vector>
#include <stdexcept>
#include <algorithm>
#include <queue>

namespace hnsw {
namespace flat_index {

/**
 * Constructor:
 * Takes an unsigned integer for the vector size and creates a new vector based
 * on that.
 */
FlatIndex::FlatIndex(std::size_t vector_size) 
    : size_constraint_(vector_size), data_() {}

/**
 * Destructor:
 * Goes through the data vector an removes all pointers to free the memory
 */
FlatIndex::~FlatIndex() {
    for (vector_base::VectorBase* v : data_) {
        delete v;
    }
}

/**
 * Copy Constructor:
 * Goes through the other object, sets the size constraint, and goes through
 * data vector, creates a new copy of every VectorBase object and pushes it to
 * current data vector
 */
FlatIndex::FlatIndex(const FlatIndex& other) 
    : size_constraint_(other.size_constraint_) {
        data_.reserve(other.data_.size());

        for (vector_base::VectorBase* v : other.data_) {
            data_.push_back(new vector_base::VectorBase(*v));
        }
}

/**
 * Copy Assignment:
 * Overrides the = operator. Deletes all pre existing data, replaces the size
 * constraints, and then repopulates the current data array with the data in
 * the other object
 */
FlatIndex& FlatIndex::operator=(const FlatIndex& other) {
    if (this == &other) {
        return *this;
    }

    for (vector_base::VectorBase* v : data_) {
        delete v;
    }

    data_.clear();

    size_constraint_ = other.size_constraint_;

    for (vector_base::VectorBase* v : other.data_) {
        data_.push_back(new vector_base::VectorBase(*v));
    }

    return *this;
}

/**
 * Move Constructor:
 * Sets current size constraint to other size constraint, then adds shallow 
 * copies of the other data to current data vector array, replaces the data
 * with null pointers
 */
FlatIndex::FlatIndex(FlatIndex&& other) noexcept : 
    size_constraint_(other.size_constraint_), data_(std::move(other.data_)) {
        other.size_constraint_ = 0;
}

/**
 * Move Assignment:
 * Clears out current data vector, sets the size constraint, and repopulates 
 * the data vector
 */
FlatIndex& FlatIndex::operator=(FlatIndex&& other) noexcept {
    if (this == &other) {
        return *this;
    }

    for (vector_base::VectorBase* v : data_) {
        delete v;
    }

    size_constraint_ = other.size_constraint_;
    data_ = std::move(other.data_);

    other.size_constraint_ = 0;

    return *this;
}

/**
 * Data Getter:
 * Returns data vector
 */
const std::vector<vector_base::VectorBase*>& FlatIndex::GetData() const {
    return data_;
}

/**
 * Data Adder:
 * Takes a reference to a VectorBase object, creates a pointer out of that
 * reference, and adds it to the data vector. Throws an error if the 
 * number of coordinates in the vector object differs from the size 
 * constraint
 */
void FlatIndex::AddData(const vector_base::VectorBase& vector_reference) {
    if (vector_reference.GetNumCoords() != size_constraint_) {
        throw std::invalid_argument("Mismatching vector sizes"); 
    }

    data_.push_back(new vector_base::VectorBase(vector_reference));
}

/**
 * Search:
 * Performs a linear search on the data vector looking for the k nearest 
 * vectors to the query. For testing/benchmarking purposes
 */
std::vector<vector_base::VectorBase> FlatIndex::Search(
    const vector_base::VectorBase& query, std::size_t k) const {
        if (k == 0 || data_.empty()) {
            return {};
        }

        std::priority_queue<std::pair<double, vector_base::VectorBase*>> queue;

        for (vector_base::VectorBase* v : data_) {
            double distance = v -> EuclideanDistanceTo(query);
            queue.push({distance, v});

            if (queue.size() > k) {
                queue.pop();
            }
        }

        std::vector<vector_base::VectorBase> results;
        results.reserve(k);

        while (!queue.empty()) {
            results.push_back(*(queue.top().second));
            queue.pop();
        }

        std::reverse(results.begin(), results.end());

        return results;
}

};
};