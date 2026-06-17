#ifndef HNSW_INCLUDE_VECTOR_BASE_H_
#define HNSW_INCLUDE_VECTOR_BASE_H_

#include <vector>
#include <type_traits>
#include <stdexcept>

namespace hnsw {
namespace vector_base {

namespace exceptions {

void ThrowMismatchingVectorSizesException() {
    throw std::invalid_argument("Mismatching vector sizes");
}

void ThrowIndexOutOfRangeException() {
    throw std::out_of_range("Index is out of bounds");
}

void ThrowZeroLengthException() {
    throw std::logic_error("One or more vectors' length is 0");
}

}

/**
 * Acts as a representation of a high dimensional vector.
 */
template<typename DataType, typename DistanceType, std::size_t Dimensions>
class VectorBase {
    static_assert(std::is_arithmetic<DistanceType>::value, 
        "Error: VectorBase can only hold numeric types (int, float, double).");

    public:
        //Constructors
        /**
         * No arg constructor:
         * Initializes coordinates with given coordinates and no data
         */
        VectorBase() {
            std::array<DistanceType, Dimensions> initial_coords;
            coords_ = initial_coords;
        }

        /**
         * Initial Data Constructor:
         * Assigns data. Leaves coordinates empty
         */
        explicit VectorBase(
            const DataType& initial_data_) : data_(initial_data_) {
                std::array<DistanceType, Dimensions> initial_data;
                data_ = initial_data;
            }
        
        /**
         * Initial Coordinate constructor:
         * Initializes coordinates with given coordinates and no data
         */
        explicit VectorBase(const std::array<DistanceType, Dimensions>& initial_coords) 
            : coords_(initial_coords) {}

        /**
         * Initial Coordinate and Data Constructor:
         * Takes a numeric array and assigns it to the coordinate field of the 
         * VectorBase object. Assigns data.
         */
        explicit VectorBase(
            const std::array<DistanceType, Dimensions>& initial_coords, 
            const DataType& initial_data_)
            : coords_(initial_coords), data_(initial_data_) {}

        //Getters and Setters
        /**
         * Data Setter:
         * Replaces the data to new data
         */
        void SetData(const DataType& new_data) {
            data_ = new_data;
        }

        /**
         * Data Getter:
         * Returns the data
         */
        const DataType& GetData() const {
            return data_;
        }

        /**
         * Coordinate Setter:
         * Replaces the coordinates to new coordinates based on the double array passed
         * through.
         */
        void SetCoords(const std::array<DataType, Dimensions>& new_coords) {
            coords_ = new_coords;
        }

        /**
         * Coordinate Getter:
         * Returns a reference to the coordinate vector
         */
        const std::array<DistanceType, Dimensions>& GetCoords() const {
            return coords_;
        }

        /**
         * Number of Coordinates Getter:
         * Returns the number of coordinates in the VectorBase object
         */
        std::size_t GetDimensions() const {
            return Dimensions;
        }

        //Arithmetic Overrides
        /**
         * Plus (+) Operator Overload
         * Facilitates easy addition of multiple VectorBase objects. Keeps the
         * first data point and discards the second.
         */
        VectorBase operator+(const VectorBase& other) const {
            if (Dimensions  != other.GetDimensions()) {
                exceptions::ThrowMismatchingVectorSizesException();
            }

            std::array<DistanceType, Dimensions> result_coords;

            for (std::size_t i = 0; i < Dimensions; i++) {
                result_coords[i] = coords_[i] + other[i];
            }

            return VectorBase<DataType, DistanceType, Dimensions>(
                result_coords, data_);
        }

        /**
         * Minus (-) Operator Overload
         * Facilitates easy subtraction of multiple VectorBase objects. Keeps 
         * the first data point and discards the second.
         */
        VectorBase operator-(const VectorBase& other) const {
            if (Dimensions != other.GetDimensions()) {
                exceptions::ThrowMismatchingVectorSizesException();
            }

            std::array<DistanceType, Dimensions> result_coords;

            for (std::size_t i = 0; i < Dimensions ; i++) {
                result_coords[i] = coords_[i] - other[i];
            }

            return VectorBase<DataType, DistanceType, Dimensions>(result_coords, data_);
        }

        //Array Overrides
        /**
         * Array Operator Overload:
         * Returns a reference to the coordinate at specified index 
         */
        double& operator[](std::size_t index) {
            if (index < 0 || index >= Dimensions ) {
                exceptions::ThrowIndexOutOfRangeException();
            }

            return coords_[index];
        }

        /**
         * Array Operator Overload:
         * Returns the coordinate at specified index 
         */
        double operator[](std::size_t index) const {
            if (index < 0 || index >= Dimensions ) {
                exceptions::ThrowIndexOutOfRangeException();
            }

            return coords_[index];
        }

        /**
         * Length:
         * Returns the length of the vector using the pythagorean theorem
         */
        double Length() const {
            double lengthSquared = 0;

            for (std::size_t i = 0; i < Dimensions ; i++) {
                lengthSquared += coords_[i] * coords_[i];
            }

            return std::sqrt(lengthSquared);
        }

        /**
         * Euclidean Distance:
         * Returns the euclidean distance from one point to another point using the 
         * Pythagorean theorem
         */
        double EuclideanDistanceTo(const VectorBase& other) const {
            if (Dimensions  != other.GetDimensions()) {
                exceptions::ThrowIndexOutOfRangeException();
            }

            double squared_distance = 0.0;

            for (std::size_t i = 0; i < Dimensions ; i++) {
                squared_distance += (coords_[i] - other[i]) * (coords_[i] - other[i]);
            }

            return std::sqrt(squared_distance);
        }

        double EuclideanDistanceTo(std::array<DistanceType, Dimensions>&& other) {
            double squared_distance = 0.0;

            for (std::size_t i = 0; i < Dimensions ; i++) {
                squared_distance += (coords_[i] - other[i]) * (coords_[i] - other[i]);
            }

            return std::sqrt(squared_distance);
        }

        /**
         * Cosine Similarity:
         * Returns the cosine similarity of two vectors
         */
        double CosineSimilarityTo(const VectorBase& other) const {
            if (Dimensions  != other.GetDimensions() ) {
                exceptions::ThrowMismatchingVectorSizesException();
            }

            double numerator = 0.0;
            double denominator = 0.0;

            for (std::size_t i = 0; i < Dimensions ; i++) {
                numerator += coords_[i] * other[i];
            }

            denominator = (Length()) * other.Length();

            if (denominator == 0) {
                exceptions::ThrowZeroLengthException();
            }

            return numerator / denominator;
        }

    private:
        /**
         * Holds that actual data of the VectorBase
         */
        
        std::array<DistanceType, Dimensions> coords_;
        DataType data_;
};

}
}

#endif