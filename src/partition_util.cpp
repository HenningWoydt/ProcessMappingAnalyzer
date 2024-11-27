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
    void determine_location(const int p_id,
                            const std::vector<int>& hierarchy,
                            const int k,
                            std::vector<int>& loc) {
        int r_start = 0;
        int r_end   = k;

        const int s = static_cast<int>(hierarchy.size());
        for (int i = 0; i < s; ++i) {
            const int n_parts = hierarchy[s - 1 - i];
            const int add     = (r_end - r_start) / n_parts;

            for (int j = 0; j < n_parts; ++j) {
                if (r_start <= p_id && p_id < r_start + add) {
                    loc[s - 1 - i] = j;
                    r_end          = r_start + add;
                    break;
                }
                r_start += add;
            }
        }
    }

    int determine_distance(const int u_id,
                           const int v_id,
                           const int k,
                           const std::vector<int>& hierarchy,
                           const std::vector<int>& distance,
                           std::vector<int>& u_loc,
                           std::vector<int>& v_loc) {
        // special case
        if (u_id == v_id) {
            return 0;
        }

        determine_location(u_id, hierarchy, k, u_loc);
        determine_location(v_id, hierarchy, k, v_loc);

        // determine the distance
        const int s = static_cast<int>(hierarchy.size());
        for (int i = 0; i < static_cast<int>(hierarchy.size()); ++i) {
            if (u_loc[s - 1 - i] != v_loc[s - 1 - i]) {
                return distance[s - 1 - i];
            }
        }
        // unreachable
        abort();
    }

    long int determine_comm_cost(const Graph& g,
                                 const std::vector<int>& partition,
                                 const std::vector<int>& hierarchy,
                                 const std::vector<int>& distance,
                                 const int k) {
        std::vector<int> u_loc(hierarchy.size());
        std::vector<int> v_loc(hierarchy.size());

        long int cost = 0;

        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                const int u_id = partition[u];
                const int v_id = partition[v];

                if (u_id != v_id) {
                    const int u_v_distance = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);

                    cost += weight * u_v_distance;
                }
            }
        }

        return cost * 2;
    }

    std::vector<long int> determine_comm_cost_per_layer(const Graph& g,
                                                        const std::vector<int>& partition,
                                                        const std::vector<int>& hierarchy,
                                                        const std::vector<int>& distance,
                                                        const int k) {
        std::vector<long int> comm_cost_per_layer(hierarchy.size(), 0);

        std::vector<int> loc_distance(hierarchy.size());
        std::iota(loc_distance.begin(), loc_distance.end(), 0);

        std::vector<int> u_loc(hierarchy.size());
        std::vector<int> v_loc(hierarchy.size());

        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                const int u_id = partition[u];
                const int v_id = partition[v];

                if (u_id != v_id) {
                    const int u_v_distance = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);
                    const int d            = determine_distance(u_id, v_id, k, hierarchy, loc_distance, u_loc, v_loc);

                    comm_cost_per_layer[d] += weight * u_v_distance;
                }
            }
        }

        for (auto& x : comm_cost_per_layer) {
            x *= 2;
        }

        return comm_cost_per_layer;
    }

    std::vector<long int> determine_partition_weights(const Graph& g,
                                                      const std::vector<int>& partition,
                                                      const int k) {
        std::vector<long int> partition_weights(k, 0);
        for (int i = 0; i < static_cast<int>(partition.size()); ++i) {
            partition_weights[partition[i]] += g.get_v_weight(i);
        }

        return partition_weights;
    }

    std::vector<double> determine_partition_balance(const Graph& g,
                                                    const std::vector<int>& partition,
                                                    const int k) {
        const auto g_weight          = g.get_g_weight();
        const double balanced_weight = static_cast<double>(g_weight) / static_cast<double>(k);

        std::vector<int> partition_weights(k, 0);
        for (int i = 0; i < static_cast<int>(partition.size()); ++i) {
            partition_weights[partition[i]] += g.get_v_weight(i);
        }

        std::vector<double> partition_balance(k, 0.0);
        for (int i = 0; i < k; ++i) {
            partition_balance[i] = static_cast<double>(partition_weights[i]) / balanced_weight;
        }

        return partition_balance;
    }

    long int determine_edge_cut(const Graph& g,
                                const std::vector<int>& partition) {
        long int edge_cut = 0;
        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                edge_cut += partition[u] != partition[v];
            }
        }
        return edge_cut;
    }

    std::vector<long int> determine_edge_cut_per_layer(const Graph& g,
                                                       const std::vector<int>& partition,
                                                       const std::vector<int>& hierarchy,
                                                       const int k) {
        std::vector<long int> edge_cut_per_layer(hierarchy.size(), 0);

        std::vector<int> distance(hierarchy.size());
        std::iota(distance.begin(), distance.end(), 0);

        std::vector<int> u_loc(hierarchy.size());
        std::vector<int> v_loc(hierarchy.size());

        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                const int u_id = partition[u];
                const int v_id = partition[v];

                if (u_id != v_id) {
                    const int d = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);
                    edge_cut_per_layer[d] += 1;
                }
            }
        }

        return edge_cut_per_layer;
    }

    long int determine_weighted_edge_cut(const Graph& g,
                                         const std::vector<int>& partition) {
        long int weighted_edge_cut = 0;
        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                weighted_edge_cut += (partition[u] != partition[v]) * weight;
            }
        }
        return weighted_edge_cut;
    }

    std::vector<long int> determine_weighted_edge_cut_per_layer(const Graph& g,
                                                                const std::vector<int>& partition,
                                                                const std::vector<int>& hierarchy,
                                                                const int k) {
        std::vector<long int> weighted_edge_cut_per_layer(hierarchy.size(), 0);

        std::vector<int> distance(hierarchy.size());
        std::iota(distance.begin(), distance.end(), 0);

        std::vector<int> u_loc(hierarchy.size());
        std::vector<int> v_loc(hierarchy.size());

        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                const int u_id = partition[u];
                const int v_id = partition[v];

                if (u_id != v_id) {
                    const int d = determine_distance(u_id, v_id, k, hierarchy, distance, u_loc, v_loc);
                    weighted_edge_cut_per_layer[d] += weight;
                }
            }
        }

        return weighted_edge_cut_per_layer;
    }

    long int determine_edge_weights(const Graph& g) {
        long int edge_weights = 0;
        for (int u = 0; u < g.get_n(); ++u) {
            for (const auto& [v, weight] : g[u]) {
                edge_weights += weight;
            }
        }
        return edge_weights;
    }
}
