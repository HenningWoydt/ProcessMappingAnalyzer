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

#include "util.h"

namespace ProMapAnalyzer {
    struct Edge {
        int v;
        int weight;
    };

    class Graph {
        int n = 0;
        int m = 0;

        std::vector<int> v_weights;
        std::vector<std::vector<Edge>> adj;

    public:
        Graph() = default;

        explicit Graph(const int n_vertices) {
            n = n_vertices;
            v_weights.resize(n_vertices, 1);
            adj.resize(n_vertices);
        }

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
                    n                               = std::stoi(header[0]);
                    m                               = 0;

                    adj.resize(n);
                    v_weights.resize(n, 1);

                    std::string fmt = "000";
                    if (header.size() == 3 && header[2].size() == 3) {
                        fmt = header[2];
                    }

                    has_v_weights = fmt[1] == '1';
                    has_e_weights = fmt[2] == '1';

                    break;
                }

                // read in edges
                std::vector<int> ints;
                int u = 0;
                while (std::getline(file, line)) {
                    if (line[0] == '%') { continue; }

                    line_to_ints(line, ints);
                    int i = 0;

                    if (has_v_weights) { set_v_weight(u, ints[i++]); }

                    while (i < static_cast<int>(ints.size())) {
                        int v      = ints[i++] - 1;
                        int weight = has_e_weights ? ints[i++] : 1;
                        if (u < v) {
                            add_edge(u, v, weight);
                        }
                    }
                    u += 1;
                }
            }
        }

        int get_n() const {
            return n;
        }

        int get_m() const {
            return m;
        }

        std::vector<Edge>& operator[](const int u) {
            return adj[u];
        }

        const std::vector<Edge>& operator[](const int u) const {
            return adj[u];
        }

        void set_v_weight(const int v, const int weight = 1) {
            v_weights[v] = weight;
        }

        int get_v_weight(const int v) const {
            return v_weights[v];
        }

        int get_g_weight() const {
            return sum<int>(v_weights);
        }

        void add_edge(int u, int v, const int weight = 1) {
            if (u > v) std::swap(u, v);

            adj[u].push_back({v, weight});
            m += 1;
        }

        bool edge_exists(int u, int v) {
            if (u > v) std::swap(u, v);
            return std::any_of(adj[u].begin(), adj[u].end(), [&](const Edge& e) { return e.v == v; });
        }

        int get_edge_weight(const int u, const int v) {
            int min = std::min(u, v);
            int max = std::max(u, v);

            for (auto& [vv, weight] : adj[min]) {
                if (vv == max) {
                    return weight;
                }
            }
            abort();
        }
    };
}

#endif //PROCESSMAPPINGANALYZER_GRAPH_H
