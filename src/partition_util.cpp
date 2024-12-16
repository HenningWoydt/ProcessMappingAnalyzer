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
#include "partition_util.h"

#include <numeric>

namespace ProMapAnalyzer {
    void determine_location(const u64 p_id,
                            const std::vector<u64>& hierarchy,
                            const u64 k,
                            std::vector<u64>& loc) {
        u64 r_start = 0;
        u64 r_end   = k;

        const size_t s = hierarchy.size();
        for (size_t i = 0; i < s; ++i) {
            const u64 n_parts = hierarchy[s - 1 - i];
            const u64 add     = (r_end - r_start) / n_parts;

            for (u64 j = 0; j < n_parts; ++j) {
                if (r_start <= p_id && p_id < r_start + add) {
                    loc[s - 1 - i] = j;
                    r_end          = r_start + add;
                    break;
                }
                r_start += add;
            }
        }
    }

    u64 determine_distance(const u64 u_id,
                           const u64 v_id,
                           const u64 k,
                           const std::vector<u64>& hierarchy,
                           const std::vector<u64>& distance,
                           std::vector<u64>& u_loc,
                           std::vector<u64>& v_loc) {
        // special case
        if (u_id == v_id) {
            return 0;
        }

        determine_location(u_id, hierarchy, k, u_loc);
        determine_location(v_id, hierarchy, k, v_loc);

        // determine the distance
        const u64 s = hierarchy.size();
        for (u64 i = 0; i < s; ++i) {
            if (u_loc[s - 1 - i] != v_loc[s - 1 - i]) {
                return distance[s - 1 - i];
            }
        }
        // unreachable
        abort();
    }

    void determine_all_stats(const Graph& g,
                             const std::vector<u64>& partition,
                             const std::vector<u64>& hierarchy,
                             const std::vector<u64>& distance,
                             const u64 k,
                             u64& edge_cut,
                             u64& weighted_edge_cut,
                             u64& comm_cost,
                             std::vector<u64>& edge_cut_layer,
                             std::vector<u64>& weighted_edge_cut_layer,
                             std::vector<u64>& comm_cost_layer) {
        edge_cut = weighted_edge_cut = comm_cost = 0;

        edge_cut_layer.resize(hierarchy.size());
        std::fill(edge_cut_layer.begin(), edge_cut_layer.end(), 0);

        weighted_edge_cut_layer.resize(hierarchy.size());
        std::fill(weighted_edge_cut_layer.begin(), weighted_edge_cut_layer.end(), 0);

        comm_cost_layer.resize(hierarchy.size());
        std::fill(comm_cost_layer.begin(), comm_cost_layer.end(), 0);

        std::vector<u64> loc_distance(hierarchy.size());
        std::iota(loc_distance.begin(), loc_distance.end(), 0);

        std::vector<u64> u_loc(hierarchy.size());
        std::vector<u64> v_loc(hierarchy.size());

        for (u64 u = 0; u < g.get_n(); ++u) {
            for (size_t idx = g.get_start(u); idx < g.get_end(u); ++idx) {
                const u64 v      = g.get_edge(idx).v;
                const u64 weight = g.get_edge(idx).weight;

                const u64 u_id = partition[u];
                const u64 v_id = partition[v];

                if (u_id != v_id) {
                    const u64 u_v_distance = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);
                    const u64 d            = determine_distance(u_id, v_id, k, hierarchy, loc_distance, u_loc, v_loc);

                    // edge cut
                    edge_cut += 1;
                    edge_cut_layer[d] += 1;

                    // weighted edge cut
                    weighted_edge_cut += weight;
                    weighted_edge_cut_layer[d] += weight;

                    // comm cost
                    comm_cost += weight * u_v_distance;
                    comm_cost_layer[d] += weight * u_v_distance;
                }
            }
        }

        edge_cut /= 2;
        for (auto& x : edge_cut_layer) {
            x /= 2;
        }

        weighted_edge_cut /= 2;
        for (auto& x : weighted_edge_cut_layer) {
            x /= 2;
        }
    }

    std::vector<u64> determine_partition_weights(const Graph& g,
                                                 const std::vector<u64>& partition,
                                                 const u64 k) {
        std::vector<u64> partition_weights(k, 0);
        for (u64 i = 0; i < partition.size(); ++i) {
            partition_weights[partition[i]] += g.get_v_weight(i);
        }

        return partition_weights;
    }

    std::vector<f64> determine_partition_balance(const Graph& g,
                                                 const std::vector<u64>& partition,
                                                 const u64 k) {
        const auto g_weight       = g.get_vertex_weights();
        const f64 balanced_weight = static_cast<f64>(g_weight) / static_cast<f64>(k);

        std::vector<u64> partition_weights(k, 0);
        for (u64 i = 0; i < partition.size(); ++i) {
            partition_weights[partition[i]] += g.get_v_weight(i);
        }

        std::vector<f64> partition_balance(k, 0.0);
        for (u64 i = 0; i < k; ++i) {
            partition_balance[i] = static_cast<f64>(partition_weights[i]) / balanced_weight;
        }

        return partition_balance;
    }
}
