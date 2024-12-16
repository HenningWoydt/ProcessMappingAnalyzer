/* Process Mapping Analyzer.
   Copyright (C) 2024  Henning Woydt

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
==============================================================================*/
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "src/definitions.h"
#include "src/graph.h"
#include "src/partition_util.h"

using namespace ProMapAnalyzer;

int main(int argc, char* argv[]) {
    auto sp = std::chrono::system_clock::now();
    std::vector<std::string> args(argv, argv + argc);

    // std::string graph_path     = "../data/graphs/eur.graph";
    // std::string partition_path = "../data/partitions/eur.txt";
    // std::string graph_path     = "../data/graphs/cop20k_A.mtx.graph";
    // std::string partition_path = "../data/partitions/a.txt";
    std::string graph_path     = "../data/graphs/rgg_n26.graph";
    std::string partition_path = "../data/partitions/rgg_n26.txt";
    std::string hierarchy_str  = "4:8:6";
    std::string distance_str   = "1:10:100";
    f64 epsilon                = 0.03;
    std::string out_path       = "out.JSON";

    if (argc > 1) {
        graph_path     = args[1];
        partition_path = args[2];
        hierarchy_str  = args[3];
        distance_str   = args[4];
        epsilon        = std::stod(args[5]);
        out_path       = args[6];
    }

    Graph g(graph_path);
    std::vector<u_int64_t> partition = read_partition(partition_path, g.get_n());
    std::vector<u64> hierarchy       = convert<u64>(split(hierarchy_str, ':'));
    std::vector<u64> distance        = convert<u64>(split(distance_str, ':'));
    u64 k                            = prod<u64>(hierarchy);

    if (g.get_n() != partition.size()) {
        std::cout << "Graph (n=" << g.get_n() << ") and partition (n=" << partition.size() << ") do not have same number of vertices!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (hierarchy.empty()) {
        std::cout << "Entered hierarchy ('" << hierarchy_str << "') is not a valid hierarchy!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (hierarchy.size() != distance.size()) {
        std::cout << "Hierarchy size (" << hierarchy.size() << ") is not equal to Distance size (" << distance.size() << ")!" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (max(partition) >= k) {
        std::cout << "Partition contains id " << max(partition) << " which is greater than k=" << k << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<f64> partition_balance = determine_partition_balance(g, partition, k);
    std::vector<u64> partition_weights = determine_partition_weights(g, partition, k);

    u64 edge_cut, weighted_edge_cut, comm_cost;
    std::vector<u64> edge_cut_layer, weighted_edge_cut_layer, comm_cost_layer;
    determine_all_stats(g, partition, hierarchy, distance, k, edge_cut, weighted_edge_cut, comm_cost, edge_cut_layer, weighted_edge_cut_layer, comm_cost_layer);

    auto ep      = std::chrono::system_clock::now();
    f64 duration = (f64)std::chrono::duration_cast<std::chrono::nanoseconds>(ep - sp).count() / 1e9;

    std::stringstream ss;
    ss << "{\n";

    ss << "\t\"n\": " << g.get_n() << " ,\n";
    ss << "\t\"m\": " << g.get_m() << " ,\n";
    ss << "\t\"graph_weight\": " << g.get_vertex_weights() << " ,\n";
    ss << "\t\"edge_weights\": " << g.get_edge_weights() << " ,\n";
    ss << "\t\"edge_cut\": " << edge_cut << " ,\n";
    ss << "\t\"edge_cut_per_layer\": " << vectorToString(edge_cut_layer) << " ,\n";
    ss << "\t\"weighted_edge_cut\": " << weighted_edge_cut << " ,\n";
    ss << "\t\"weighted_edge_cut_per_layer\": " << vectorToString(weighted_edge_cut_layer) << " ,\n";
    ss << "\t\"comm_cost\": " << comm_cost << " ,\n";
    ss << "\t\"comm_cost_per_layer\": " << vectorToString(comm_cost_layer) << " ,\n";
    ss << "\t\"max_balance\": " << max(partition_balance) << " ,\n";
    ss << "\t\"avg_balance\": " << sum<double>(partition_balance) / static_cast<double>(k) << " ,\n";
    ss << "\t\"min_balance\": " << min(partition_balance) << " ,\n";
    ss << "\t\"L_max\": " << ceil((1 + epsilon) * (static_cast<double>(g.get_vertex_weights()) / static_cast<double>(k))) << ", \n";
    ss << "\t\"partition_balance\": " << vectorToString(partition_balance) << " ,\n";
    ss << "\t\"partition_weights\": " << vectorToString(partition_weights) << ", \n";
    ss << "\t\"is_balanced_on_epsilon\": " << (max(partition_balance) <= 1.03) << ", \n";
    ss << "\t\"is_balanced_on_L_max\": " << (static_cast<double>(max(partition_weights)) <= ceil((1 + epsilon) * (static_cast<double>(g.get_vertex_weights()) / static_cast<double>(k)))) << ", \n";
    ss << "\t\"processed_in\": " << duration << "\n";

    ss << "}";

    std::ofstream out(out_path);
    out << ss.rdbuf();
    out.close();

    return 0;
}
