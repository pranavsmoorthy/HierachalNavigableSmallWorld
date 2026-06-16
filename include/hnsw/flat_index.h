#ifndef HNSW_EXCEPTIONS_FLAT_INDEX_H_
#define HNSW_EXCEPTIONS_FLAT_INDEX_H_

#include "vector_base.h"

#include <vector>

namespace hnsw {
namespace flat_index {

/**
 * This is meant for testing. The class will be representing a linear search
 * and will be used to compare against the final HNSW graph.
 */
class FlatIndex {
    public:
        /**
         * Constructor:
         * Single constructor that takes the size of the VectorBase objects,
         * which will be enforced.
         */

        explicit FlatIndex(std::size_t vector_size);

        /**
         * Rule of 5:
         * 1. Destructor - Frees the resource
         * 2. Copy Constructor - Creates deep copy of object
         * 3; Copy Assignment - Overwrites existing via deep copy
         * 4. Move Constructor - Transfer ownership of resources from temporary
         *    object to an new one without copying
         * 5. Move Assignment - Transfer ownership of resources from a 
         *    temporary object to an existing one
         */

        ~FlatIndex();
        FlatIndex(const FlatIndex& other);
        FlatIndex& operator=(const FlatIndex& other);
        FlatIndex(FlatIndex&& other) noexcept;
        FlatIndex& operator=(FlatIndex&& other) noexcept;

        /**
         * Getters and Setters:
         * One getter, which returns the data of the FlatIndex. AddData adds a 
         * pointer to the underlying data vector.
         */

        const std::vector<vector_base::VectorBase*>& GetData() const;
        void AddData(const vector_base::VectorBase& vector_reference);

        /**
         * Search:
         * Performs a linear search on the data. This is mainly for testing and
         * acts as a benchmark for the HNSW graph to compare against.
         */

        std::vector<vector_base::VectorBase> Search(
            const vector_base::VectorBase& query, std::size_t k) const;
    
    private:
        /**
         * The size the vectors must be in order to be added to the data vector
         */
        std::size_t size_constraint_;

        /**
         * The data vector
         */
        std::vector<vector_base::VectorBase*> data_;
};

};
};

#endif