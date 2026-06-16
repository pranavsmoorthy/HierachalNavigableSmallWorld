#include "catch.h"
#include "../include/hnsw/vector_base.h"

#include <vector>
#include <array>
#include <stdexcept>

using namespace hnsw;

TEST_CASE("VectorBase: Initialization and Accessors", "[vector_base]") {
    // Test 1: Default Constructor
    vector_base::VectorBase<double, double, 3> empty_vec;
    REQUIRE(empty_vec[0] == 0.0);
    REQUIRE(empty_vec[1] == 0.0);
    REQUIRE(empty_vec[2] == 0.0);

    // Test 2: Value Constructor & Array Operator
    std::array<double, 3> initial_data = {1.5, -2.0, 3.14};
    vector_base::VectorBase<double, double, 3> populated_vec(initial_data);
    
    REQUIRE(populated_vec[0] == 1.5);
    REQUIRE(populated_vec[1] == -2.0);
    REQUIRE(std::abs(3.14 - populated_vec[2]) <= 0.01);

    // Test 3: Modification via reference operator[]
    populated_vec[1] = 9.9;
    REQUIRE(std::abs(9.9 - populated_vec[1]) <= 0.01);
}

TEST_CASE("VectorBase: Mathematical Operators (+ and -)", "[vector_base]") {
    vector_base::VectorBase<double, double, 2> vec1({1.0, 5.0});
    vector_base::VectorBase<double, double, 2> vec2({3.0, 2.0});

    SECTION("Addition") {
        vector_base::VectorBase<double, double, 2> result = vec1 + vec2;
        REQUIRE(std::abs(4.0 - result[0]) <= 0.01);
        REQUIRE(std::abs(7.0 - result[1]) <= 0.01);
        
        // Ensure original vectors were NOT modified (const-correctness check)
        REQUIRE(vec1[0] == 1.0);
    }

    SECTION("Subtraction") {
        vector_base::VectorBase<double, double, 2> result = vec1 - vec2;
        REQUIRE(std::abs(result[0] + 2.0) <= 0.01);
        REQUIRE(std::abs(result[1] - 3.0) <= 0.01);
    }
}

TEST_CASE("VectorBase: Geometric Formulas", "[vector_base]") {
    SECTION("Length (Magnitude)") {
        // A classic 3-4-5 right triangle
        vector_base::VectorBase<double, double, 2> vec({3.0, 4.0});
        REQUIRE(std::abs(vec.Length() - 5.0) <= 0.01);

        // Origin vector length should be 0
        vector_base::VectorBase<double, double, 2> zero_vec({0.0, 0.0});
        REQUIRE(std::abs(zero_vec.Length() - 0.0) <= 0.01);
    }

    SECTION("Euclidean Distance") {
        vector_base::VectorBase<double, double, 2> vec1({1.0, 2.0});
        vector_base::VectorBase<double, double, 2> vec2({4.0, 6.0});
        
        // Distance between (1,2) and (4,6) is 5.0
        REQUIRE(std::abs(vec1.EuclideanDistanceTo(vec2) - 5.0) <= 0.01);
        
        // Distance to itself should be 0
        REQUIRE(std::abs(vec1.EuclideanDistanceTo(vec1)) - 0.0 <= 0.01);
    }

    SECTION("Cosine Similarity") {
        vector_base::VectorBase<double, double, 2> vec_x({1.0, 0.0});
        vector_base::VectorBase<double, double, 2> vec_y({0.0, 1.0});
        vector_base::VectorBase<double, double, 2> vec_x2({5.0, 0.0});
        vector_base::VectorBase<double, double, 2> vec_neg_x({-1.0, 0.0});

        // 1. Identical direction (Different magnitude) -> Similarity = 1.0
        REQUIRE(std::abs(vec_x.CosineSimilarityTo(vec_x2) - 1.0) <= 0.01);

        // 2. Orthogonal (90 degrees apart) -> Similarity = 0.0
        REQUIRE(std::abs(vec_x.CosineSimilarityTo(vec_y) - 0.0) <= 0.01);

        // 3. Opposite directions (180 degrees apart) -> Similarity = -1.0
        REQUIRE(std::abs(vec_x.CosineSimilarityTo(vec_neg_x) + 1) <= 0.01);

        // 4. EDGE CASE: Division by zero protection
        vector_base::VectorBase<double, double, 2> zero_vec({0.0, 0.0});
        REQUIRE_THROWS_AS(vec_x.CosineSimilarityTo(zero_vec), std::logic_error);
    }
}

TEST_CASE("VectorBase: Differing DataType and DistanceType", "[vector_base]") {
    SECTION("Integer Data with Double Distance") {
        // Vector data is integers, but distance calculations return doubles
        vector_base::VectorBase<int, double, 2> int_vec1({0, 0});
        vector_base::VectorBase<int, double, 2> int_vec2({3, 4});

        // The Euclidean distance should be a precise double (5.0)
        double distance = int_vec1.EuclideanDistanceTo(int_vec2);
        REQUIRE(std::abs(distance - 5.0) <= 0.01);

        // Subtraction should still yield integer vectors
        vector_base::VectorBase<int, double, 2> result = int_vec2 - int_vec1;
        REQUIRE(result[0] == 3);
        REQUIRE(result[1] == 4);
    }
}