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
#ifndef PROCESSMAPPINGANALYZER_PARTITION_UTIL_H
#define PROCESSMAPPINGANALYZER_PARTITION_UTIL_H

#include <vector>

#include "graph.h"

namespace ProMapAnalyzer {
    void determine_location(int p_id,
                            const std::vector<int>& hierarchy,
                            int k,
                            std::vector<int>& loc);

    int determine_distance(int u_id,
                           int v_id,
                           int k,
                           const std::vector<int>& hierarchy,
                           const std::vector<int>& distance,
                           std::vector<int>& u_loc,
                           std::vector<int>& v_loc);

    long int determine_comm_cost(const Graph& g,
                                 const std::vector<int>& partition,
                                 const std::vector<int>& hierarchy,
                                 const std::vector<int>& distance,
                                 int k);

    std::vector<long int> determine_comm_cost_per_layer(const Graph& g,
                                                        const std::vector<int>& partition,
                                                        const std::vector<int>& hierarchy,
                                                        const std::vector<int>& distance,
                                                        int k);

    std::vector<long int> determine_partition_weights(const Graph& g,
                                                      const std::vector<int>& partition,
                                                      int k);

    std::vector<double> determine_partition_balance(const Graph& g,
                                                    const std::vector<int>& partition,
                                                    int k);

    long int determine_edge_cut(const Graph& g,
                                const std::vector<int>& partition);

    std::vector<long int> determine_edge_cut_per_layer(const Graph& g,
                                                       const std::vector<int>& partition,
                                                       const std::vector<int>& hierarchy,
                                                       int k);

    long int determine_weighted_edge_cut(const Graph& g,
                                         const std::vector<int>& partition);

    std::vector<long int> determine_weighted_edge_cut_per_layer(const Graph& g,
                                                                const std::vector<int>& partition,
                                                                const std::vector<int>& hierarchy,
                                                                int k);

    long int determine_edge_weights(const Graph& g);
}

#endif //PROCESSMAPPINGANALYZER_PARTITION_UTIL_H
