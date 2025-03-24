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
#ifndef PROCESSMAPPINGANALYZER_GRAPH_H
#define PROCESSMAPPINGANALYZER_GRAPH_H

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include "definitions.h"
#include "util.h"

namespace ProMapAnalyzer {
    struct Edge {
        u64 v;
        u64 weight;

        Edge() = default;
        Edge(const u64 v, const u64 w) : v(v), weight(w) {}
    };

    class Graph {
        u64 n = 0;
        u64 m = 0;

        u64 vertex_weights = 0;
        u64 edge_weights   = 0;

        std::vector<u64> v_weights;
        std::vector<size_t> neighborhoods;
        std::vector<Edge> edges;
        size_t curr_m = 0;

    public:
        explicit Graph(const std::string& path) {
            if (!file_exists(path)) {
                std::cerr << "File " << path << " does not exist!" << std::endl;
                std::abort();
            }

            std::ifstream file(path);
            if (file.is_open()) {
                std::string line;
                bool has_v_weights = false;
                bool has_e_weights = false;

                while (std::getline(file, line)) {
                    if (line[0] == '%') { continue; }

                    // remove leading and trailing whitespaces, replace double whitespaces
                    line.erase(0, line.find_first_not_of(' ')).erase(line.find_last_not_of(' ') + 1);
                    line = std::regex_replace(line, std::regex("\\s{2,}"), " ");

                    // read in header
                    std::vector<std::string> header = split(line, ' ');
                    n                               = std::stoll(header[0]);
                    m                               = std::stoll(header[1]) * 2;

                    neighborhoods.resize(n + 1);
                    v_weights.resize(n, 1);
                    vertex_weights = n;
                    edges.reserve(m);
                    edge_weights = 0;

                    std::string fmt = "000";
                    if (header.size() == 3 && header[2].size() == 3) {
                        fmt = header[2];
                    }

                    has_v_weights = fmt[1] == '1';
                    has_e_weights = fmt[2] == '1';

                    break;
                }

                // read in edges
                std::vector<u64> ints;
                u64 u = 0;
                neighborhoods[0] = 0;
                while (std::getline(file, line)) {
                    if (line[0] == '%') { continue; }

                    line_to_ints(line, ints);
                    size_t i = 0;

                    if (has_v_weights) {
                        v_weights[u] = ints[i++];
                        vertex_weights -= 1;
                        vertex_weights += v_weights[u];
                    }

                    while (i < ints.size()) {
                        u64 v      = ints[i++] - 1;
                        u64 weight = has_e_weights ? ints[i++] : 1;
                        edges.emplace_back(v, weight);
                        edge_weights += weight;
                    }
                    neighborhoods[u + 1] = edges.size();
                    u += 1;
                }
                edge_weights /= 2;
            }
        }

        u64 get_n() const { return n; }
        u64 get_m() const { return m / 2; }
        u64 get_v_weight(const u64 v) const { return v_weights[v]; }
        u64 get_vertex_weights() const { return vertex_weights; }
        u64 get_edge_weights() const { return edge_weights; }

        u64 get_start(const u64 u) const { return neighborhoods[u]; }
        u64 get_end(const u64 u) const { return neighborhoods[u + 1]; }
        Edge get_edge(const u64 idx) const { return edges[idx]; }
        Edge& get_edge(const u64 idx) { return edges[idx]; }
    };
}

#endif //PROCESSMAPPINGANALYZER_GRAPH_H
