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
#include <numeric>

#include "graph.h"


namespace ProMapAnalyzer {
    inline u64 bitsNeeded64(u64 x) {
        if (x == 0) return 1;
        return 64 - (u64) __builtin_clzll(x);
    }

    class DistanceOracle {
        std::vector<u64> m_hierarchy;
        std::vector<u64> m_distance;
        size_t m_l = 0;
        u64 m_k = 0;

        std::vector<u64> identifier; // O(k)
        std::vector<u64> dist_lookup; // O(64)
        std::vector<u64> layer_lookup; // O(64)

    public:
        void initialize(const std::vector<u64> &hierarchy,
                        const std::vector<u64> &distance) {
            m_hierarchy = hierarchy;
            m_distance = distance;
            m_l = m_hierarchy.size();
            m_k = prod<u64>(m_hierarchy);

            std::vector<u64> bit_sizes;
            bit_sizes.reserve(m_l);

            u64 total_bits = 0;
            for (u64 size : m_hierarchy) {
                const u64 w = bitsNeeded64(size - 1);
                bit_sizes.push_back(w);
                total_bits += w;
            }

            if (total_bits > 64) {
                std::cerr << "Hierarchy too large for 64-bit encoding! k = " << m_k << std::endl;
                std::exit(EXIT_FAILURE);
            }

            identifier.resize(m_k);
            std::vector<u64> loc(m_l);

            for (u64 id = 0; id < m_k; ++id) {
                determine_loc(id, loc);
                u64 ident = loc[m_l - 1];

                for (size_t i = 1; i < m_l; ++i) {
                    ident = ident << bit_sizes[m_l - 1 - i];
                    ident |= loc[m_l - 1 - i];
                }

                identifier[id] = ident;
            }

            dist_lookup.assign(64, m_distance.back());
            layer_lookup.assign(64, m_l - 1);
            size_t idx = 0;
            for (size_t i = 0; i < m_l; ++i) {
                for (size_t j = 0; j < bit_sizes[i]; ++j) {
                    dist_lookup[63 - idx] = m_distance[i];
                    layer_lookup[63 - idx] = i;
                    idx += 1;
                }
            }
        }

        u64 get_distance(u64 u_id, u64 v_id) const {
            if (u_id == v_id) return 0;
            const u64 x = identifier[u_id] ^ identifier[v_id];
            return dist_lookup[__builtin_clzll(x)];
        }

        u64 get_layer(u64 u_id, u64 v_id) const {
            if (u_id == v_id) return 0;
            const u64 x = identifier[u_id] ^ identifier[v_id];
            return layer_lookup[__builtin_clzll(x)];
        }

    private:
        void determine_loc(u64 p_id, std::vector<u64> &loc) const {
            u64 r_start = 0;
            u64 r_end = m_k;

            for (size_t i = 0; i < m_l; ++i) {
                const u64 n_parts = m_hierarchy[m_l - 1 - i];
                const u64 add = (r_end - r_start) / n_parts;

                for (u64 j = 0; j < n_parts; ++j) {
                    if (r_start <= p_id && p_id < r_start + add) {
                        loc[m_l - 1 - i] = j;
                        r_end = r_start + add;
                        break;
                    }
                    r_start += add;
                }
            }
        }
    };

    inline void determine_all_stats(const Graph &g,
                                    const std::vector<u64> &partition,
                                    const std::vector<u64> &hierarchy,
                                    const std::vector<u64> &distance,
                                    const u64 /*k*/,
                                    u64 &edge_cut,
                                    u64 &weighted_edge_cut,
                                    u64 &comm_cost,
                                    std::vector<u64> &edge_cut_layer,
                                    std::vector<u64> &weighted_edge_cut_layer,
                                    std::vector<u64> &comm_cost_layer) {
        const size_t s = hierarchy.size();

        DistanceOracle oracle;
        oracle.initialize(hierarchy, distance);

        edge_cut = weighted_edge_cut = comm_cost = 0;

        edge_cut_layer.assign(s, 0);
        weighted_edge_cut_layer.assign(s, 0);
        comm_cost_layer.assign(s, 0);

        #pragma omp parallel
        {
            u64 local_edge_cut = 0, local_weighted_edge_cut = 0, local_comm_cost = 0;
            std::vector<u64> local_ec_layer(s, 0);
            std::vector<u64> local_wec_layer(s, 0);
            std::vector<u64> local_cc_layer(s, 0);

            #pragma omp for schedule(dynamic, 4096)
            for (u64 u = 0; u < g.n; ++u) {
                const u64 u_id = partition[u];

                for (size_t idx = g.neighborhoods[u]; idx < g.neighborhoods[u + 1]; ++idx) {
                    const u64 v = g.edges_v[idx];
                    const u64 v_id = partition[v];

                    if (u_id != v_id) {
                        const u64 dist = oracle.get_distance(u_id, v_id);
                        const u64 layer = oracle.get_layer(u_id, v_id);

                        local_edge_cut += 1;
                        local_ec_layer[layer] += 1;

                        const u64 weight = g.edges_w[idx];
                        local_weighted_edge_cut += weight;
                        local_wec_layer[layer] += weight;

                        local_comm_cost += weight * dist;
                        local_cc_layer[layer] += weight * dist;
                    }
                }
            }

            #pragma omp critical
            {
                edge_cut += local_edge_cut;
                weighted_edge_cut += local_weighted_edge_cut;
                comm_cost += local_comm_cost;
                for (size_t i = 0; i < s; ++i) {
                    edge_cut_layer[i] += local_ec_layer[i];
                    weighted_edge_cut_layer[i] += local_wec_layer[i];
                    comm_cost_layer[i] += local_cc_layer[i];
                }
            }
        }

        edge_cut /= 2;
        for (auto &x: edge_cut_layer) {
            x /= 2;
        }

        weighted_edge_cut /= 2;
        for (auto &x: weighted_edge_cut_layer) {
            x /= 2;
        }
    }

    inline std::vector<u64> determine_partition_weights(const Graph &g,
                                                        const std::vector<u64> &partition,
                                                        const u64 k) {
        std::vector<u64> partition_weights(k, 0);
        for (u64 i = 0; i < partition.size(); ++i) {
            partition_weights[partition[i]] += g.v_weights[i];
        }

        return partition_weights;
    }

    inline std::vector<f64> determine_partition_balance(const Graph &g,
                                                        const std::vector<u64> &partition,
                                                        const u64 k) {
        const auto g_weight = g.vertex_weights;
        const f64 balanced_weight = static_cast<f64>(g_weight) / static_cast<f64>(k);

        std::vector<u64> partition_weights(k, 0);
        for (u64 i = 0; i < partition.size(); ++i) {
            partition_weights[partition[i]] += g.v_weights[i];
        }

        std::vector<f64> partition_balance(k, 0.0);
        for (u64 i = 0; i < k; ++i) {
            partition_balance[i] = static_cast<f64>(partition_weights[i]) / balanced_weight;
        }

        return partition_balance;
    }
}

#endif //PROCESSMAPPINGANALYZER_PARTITION_UTIL_H
