#ifndef VF_INCLUDE_GRAPH_H_
#define VF_INCLUDE_GRAPH_H_

#include "node.h"
#include "cluster.h"
#include "vector_base.h"

#include <unordered_set>
#include <array>

namespace vectorforge {
namespace graph {

template <typename DataType, typename DistanceType, std::size_t Dimensions,
    std::size_t MaxConnectionsForNodes>
class Graph {
    using NodeType = node::Node<DataType, DistanceType, Dimensions, MaxConnectionsForNodes>;
    using ClusterType = cluster::Cluster<DataType, DistanceType, Dimensions, MaxConnectionsForNodes>;
    using VectorType = vector_base::VectorBase<DataType, DistanceType, Dimensions>;
    using CoordinateArray = std::array<DistanceType, Dimensions>;

    public:
        /**
         * Default Constructor:
         * Sets head cluster to a null pointer
         */
        Graph() : head_(nullptr) {}

        /**
         * 
         */
        ~Graph() {

        }

        void AddNode(const DataType& data, const CoordinateArray& coords) {
            VectorType* new_vector = new VectorType(coords, data);
            NodeType* new_node = new NodeType(new_vector);

            if (head_ == nullptr) {
                head_ = new ClusterType();
                head_ -> AddNode(new_node);
            } else {
                // other stuff
            }
        }

        ClusterType* GetNearestCluster(const CoordinateArray& query_coordinates) {
            if (head_ == nullptr) {
                return nullptr;
            }

            ClusterType* current_cluster = head_;
            std::unordered_set<ClusterType*> cluster_set;
            
            ClusterType* best_cluster = nullptr;
            double best_distance = 0.0; 
            
            while (true) {
                double current_distance = (current_cluster -> GetHeadNode()) -> DistanceToCoords(query_coordinates);
                
                if (best_cluster == nullptr || current_distance < best_distance) {
                    best_cluster = current_cluster;
                    best_distance = current_distance;
                }

                ClusterType* nearest_in_adjacent = nullptr;

                for (ClusterType* c : current_cluster -> GetAdjacencySet()) {
                    if (cluster_set.count(c) == 0) {
                        double distance_to_query = (c -> GetHeadNode()) -> DistanceToCoords(query_coordinates);

                        if (distance_to_query < current_distance) {
                            current_distance = distance_to_query;
                            nearest_in_adjacent = c;
                        }
                    }
                }

                if (nearest_in_adjacent == nullptr) {
                    return best_cluster;
                } else {
                    cluster_set.insert(current_cluster);
                    current_cluster = nearest_in_adjacent;
                }
            }

            return nullptr;
        }

        /**
         * Find nearest K Clusters:
         * Finds the nearest K clusters closes to a queried coordinate
         */
        std::vector<ClusterType*> FindNearestKClusters(
            const std::array<DistanceType, Dimensions>& query_coordinates, 
            const size_t k) {
                std::vector<ClusterType*> results;

                if (k == 0 || head_ == nullptr) {
                    return results;
                }

                results.reserve(k);

                std::priority_queue<std::pair<double, ClusterType*>> cluster_queue;
                cluster_queue.push({
                    head_ -> GetHeadNode() -> DistanceToCoords(query_coordinates), 
                    head_
                });

                ClusterType* current_cluster = head_;
                std::unordered_set<ClusterType*> cluster_set;
                cluster_set.insert(head_);
                
                while (true) {
                    double current_distance = 
                        current_cluster -> DistanceToCoords(query_coordinates);

                    ClusterType* nearest_in_adjacent = nullptr;

                    for (ClusterType* n : current_cluster -> GetAdjacencySet()) {
                        if (cluster_set.count(n) == 0) {
                            double distance_to_query = 
                                n -> DistanceToCoords(query_coordinates);

                            cluster_queue.push({distance_to_query, n});

                            if (cluster_queue.size() > k) {
                                cluster_queue.pop();
                            }

                            if (distance_to_query < current_distance) {
                                current_distance = distance_to_query;
                                nearest_in_adjacent = n;
                            }

                            cluster_set.insert(n);
                        }
                    }

                    if (nearest_in_adjacent == nullptr) {
                        break;
                    } else {
                        current_cluster = nearest_in_adjacent;
                    }
                }

                while (!cluster_queue.empty()) {
                    results.push_back(cluster_queue.top().second);
                    cluster_queue.pop();
                }

                std::reverse(results.begin(), results.end());

                return results;
        }

    private:
        /**
         * Start Cluster:
         * Serves as the default entry cluster for the graph, makes sure that 
         * all clusters trace back to this cluster
         */
        ClusterType* head_;
};

}
}

#endif