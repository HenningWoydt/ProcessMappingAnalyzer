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
    void determine_location(u64 p_id,
                            const std::vector<u64>& hierarchy,
                            u64 k,
                            std::vector<u64>& loc);

    u64 determine_distance(u64 u_id,
                           u64 v_id,
                           u64 k,
                           const std::vector<u64>& hierarchy,
                           const std::vector<u64>& distance,
                           std::vector<u64>& u_loc,
                           std::vector<u64>& v_loc);

    void determine_all_stats(const Graph& g,
                             const std::vector<u64>& partition,
                             const std::vector<u64>& hierarchy,
                             const std::vector<u64>& distance,
                             u64 k,
                             u64& edge_cut,
                             u64& weighted_edge_cut,
                             u64& comm_cost,
                             std::vector<u64>& edge_cut_layer,
                             std::vector<u64>& weighted_edge_cut_layer,
                             std::vector<u64>& comm_cost_layer);

    std::vector<u64> determine_partition_weights(const Graph& g,
                                                 const std::vector<u64>& partition,
                                                 u64 k);

    std::vector<double> determine_partition_balance(const Graph& g,
                                                    const std::vector<u64>& partition,
                                                    u64 k);
}

#endif //PROCESSMAPPINGANALYZER_PARTITION_UTIL_H
