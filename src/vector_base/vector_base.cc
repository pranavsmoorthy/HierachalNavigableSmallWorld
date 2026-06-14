#include "vector_base.h"

#include <vector>
#include <stdexcept>
#include <cmath>

namespace hnsw {
namespace vector_base {

/**
 * No arg constructor:
 * Initializes coordinates with empty double array of size 2
 */
VectorBase::VectorBase() : coords_(std::vector<double>(0)) {}

/**
 * Initial Coordinate Constructor:
 * Takes a double vector and assigns it to the coordinate field of the 
 * VectorBase object.
 */
VectorBase::VectorBase(const std::vector<double>& initial_coords) 
    : coords_(initial_coords) {}

/**
 * Coordinate Setter:
 * Replaces the coordinates to new coordinates based on the double array passed
 * through.
 */
void VectorBase::SetCoords(const std::vector<double>& coords) {
    this -> coords_ = coords;
}

/**
 * Coordinate Getter:
 * Returns a reference to the coordinate vector
 */
const std::vector<double>& VectorBase::GetCoords() const {
    return coords_;
}

/**
 * Number of Coordinates Getter:
 * Returns the number of coordinates in the VectorBase object
 */
std::size_t VectorBase::GetNumCoords() const {
    return this -> coords_.size();
}

/**
 * Plus (+) Operator Overload
 * Facilitates easy addition of multiple VectorBase objects
 */
VectorBase VectorBase::operator+(const VectorBase& other) const {
    if (coords_.size() != other.coords_.size()) {
        throw std::invalid_argument("Mismatching vector sizes");
    }

    std::vector<double> result_coords(coords_.size());

    for (std::size_t i = 0; i < coords_.size(); i++) {
        result_coords[i] = coords_[i] + other.coords_[i];
    }

    return VectorBase(result_coords);
}

/**
 * Minus (-) Operator Overload
 * Facilitates easy subtraction of multiple VectorBase objects
 */
VectorBase VectorBase::operator-(const VectorBase& other) const {
    if (coords_.size() != other.coords_.size()) {
        throw std::invalid_argument("Mismatching vector sizes");
    }

    std::vector<double> result_coords(coords_.size());

    for (std::size_t i = 0; i < coords_.size(); i++) {
        result_coords[i] = coords_[i] - other.coords_[i];
    }

    return VectorBase(result_coords);
}

/**
 * Array Operator Overload:
 * Returns a reference to the coordinate at specified index 
 */
double& VectorBase::operator[](std::size_t index) {
    if (index < 0 || index >= coords_.size()) {
        throw std::out_of_range("Index is out of bounds");
    }

    return coords_[index];
}

/**
 * Array Operator Overload:
 * Returns the coordinate at specified index 
 */
double VectorBase::operator[](std::size_t index) const {
    if (index < 0 || index >= coords_.size()) {
        throw std::out_of_range("Index is out of bounds");
    }

    return coords_[index];
}

/**
 * Length:
 * Returns the length of the vector using the pythagorean theorem
 */
double VectorBase::Length() const {
    double lengthSquared = 0;

    for (std::size_t i = 0; i < coords_.size(); i++) {
        lengthSquared += coords_[i] * coords_[i];
    }

    return std::sqrt(lengthSquared);
}

/**
 * Euclidean Distance:
 * Returns the euclidean distance from one point to another point using the 
 * Pythagorean theorem
 */
double VectorBase::EuclideanDistanceTo(const VectorBase& other) const {
    if (coords_.size() != other.coords_.size()) {
        throw std::invalid_argument("Mismatching vector sizes");
    }

    double squared_distance = 0.0;

    for (std::size_t i = 0; i < coords_.size(); i++) {
        squared_distance += (coords_[i] - other[i]) * (coords_[i] - other[i]);
    }

    return std::sqrt(squared_distance);
}

/**
 * Cosine Similarity:
 * Returns the cosine similarity of two vectors
 */
double VectorBase::CosineSimilarityTo(const VectorBase& other) const {
    if (coords_.size() != other.coords_.size()) {
        throw std::invalid_argument("Mismatching vector sizes");
    }

    double numerator = 0.0;
    double denominator = 0.0;

    for (std::size_t i = 0; i < coords_.size(); i++) {
        numerator += coords_[i] * other[i];
    }

    denominator = (Length()) * other.Length();

    if (denominator == 0) {
        throw std::logic_error("One or more vectors' length is 0");
    }

    return numerator / denominator;
}

};
};