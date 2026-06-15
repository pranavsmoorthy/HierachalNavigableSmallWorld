#ifndef HNSW_SRC_VECTOR_BASE_H_
#define HNSW_SRC_VECTOR_BASE_H_

#include <vector>

namespace hnsw {
namespace vector_base {

/**
 * Acts as a representation of a high dimensional vector.
 */
class VectorBase {
    public:
        /**
         * Constructors:
         * One default no arg constructor and one explicit constructor that 
         * takes a double vector to initialize 
         */

        VectorBase();
        explicit VectorBase(const std::vector<double>& initial_coords);

        /**
         * Getters and Setters:
         * Get and replace coords in current VectorBase object with new 
         * coordinates. Get number of coordinates in a VectorBase object,
         * read only
         */

        void SetCoords(const std::vector<double>& coords);
        const std::vector<double>& GetCoords() const;
        std::size_t GetNumCoords() const;

        /**
         * Arithmetic Operator Overload:
         * Overload the operators for + and - for easier addition and
         * subtraction of vectors. Other VectorBase is left constant
         */

        VectorBase operator+(const VectorBase& other) const;
        VectorBase operator-(const VectorBase& other) const;

        /**
         * Array Operators Overload:
         * Overload the square bracket [] operators with read only and 
         * write options that take a size_t index;
         */

        double& operator[](std::size_t index);
        double operator[](std::size_t index) const;

        /**
         * Other operations:
         * Includes finding the length of the vector and euclidean 
         * distance to another vector, both of which use pythagorean 
         * theorem, as well as the cosine similarity. All are read only.
         */

        double Length() const;
        double EuclideanDistanceTo(const VectorBase& other) const;
        double CosineSimilarityTo(const VectorBase& other) const;

    private:
        /**
         * Holds that actual data of the VectorBase
         */
        
        std::vector<double> coords_;
};

}
}

#endif