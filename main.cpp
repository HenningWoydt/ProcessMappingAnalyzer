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
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "src/graph.h"
#include "src/partition_util.h"

using namespace ProMapAnalyzer;

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);

    std::string graph_path     = "../data/graphs/cop20k_A.mtx.graph";
    std::string partition_path = "../data/partitions/cop20k_A.mtx.graph.partition.txt";
    std::string hierarchy_str  = "4:8:6";
    std::string distance_str   = "1:10:100";
    double epsilon             = 0.03;
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
    std::vector<int> partition = read_partition(partition_path);
    std::vector<int> hierarchy = convert<int>(split(hierarchy_str, ':'));
    std::vector<int> distance  = convert<int>(split(distance_str, ':'));
    auto k                     = prod<int>(hierarchy);

    if (g.get_n() != static_cast<int>(partition.size())) {
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

    std::vector<double> partition_balance   = determine_partition_balance(g, partition, k);
    std::vector<long int> partition_weights = determine_partition_weights(g, partition, k);

    std::stringstream ss;
    ss << "{\n";

    ss << "\t\"n\": " << g.get_n() << " ,\n";
    ss << "\t\"m\": " << g.get_m() << " ,\n";
    ss << "\t\"graph_weight\": " << g.get_g_weight() << " ,\n";
    ss << "\t\"edge_weights\": " << determine_edge_weights(g) << " ,\n";
    ss << "\t\"edge_cut\": " << determine_edge_cut(g, partition) << " ,\n";
    ss << "\t\"edge_cut_per_layer\": " << vectorToString(determine_edge_cut_per_layer(g, partition, hierarchy, k)) << " ,\n";
    ss << "\t\"weighted_edge_cut\": " << determine_weighted_edge_cut(g, partition) << " ,\n";
    ss << "\t\"weighted_edge_cut_per_layer\": " << vectorToString(determine_weighted_edge_cut_per_layer(g, partition, hierarchy, k)) << " ,\n";
    ss << "\t\"comm_cost\": " << determine_comm_cost(g, partition, hierarchy, distance, k) << " ,\n";
    ss << "\t\"comm_cost_per_layer\": " << vectorToString(determine_comm_cost_per_layer(g, partition, hierarchy, distance, k)) << " ,\n";
    ss << "\t\"max_balance\": " << max(partition_balance) << " ,\n";
    ss << "\t\"avg_balance\": " << sum<double>(partition_balance) / static_cast<double>(k) << " ,\n";
    ss << "\t\"min_balance\": " << min(partition_balance) << " ,\n";
    ss << "\t\"L_max\": " << ceil((1 + epsilon) * (static_cast<double>(g.get_g_weight()) / static_cast<double>(k))) << ", \n";
    ss << "\t\"partition_balance\": " << vectorToString(partition_balance) << " ,\n";
    ss << "\t\"partition_weights\": " << vectorToString(partition_weights) << ", \n";
    ss << "\t\"is_balanced_on_epsilon\": " << (max(partition_balance) <= 1.03) << ", \n";
    ss << "\t\"is_balanced_on_L_max\": " << (static_cast<double>(max(partition_weights)) <= ceil((1 + epsilon) * (static_cast<double>(g.get_g_weight()) / static_cast<double>(k)))) << "\n";

    ss << "}";

    std::ofstream out(out_path);
    out << ss.rdbuf();
    out.close();

    return 0;
}
