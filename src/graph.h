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
            std::ifstream file(path, std::ios::ate); // Open file in "at end" mode to get size
            size_t file_size = file.tellg(); // Get file size
            file.seekg(0); // Rewind to the beginning

            std::string content(file_size, '\0');
            file.read(&content[0], file_size);

            const char* ptr = content.data();
            const char* end = ptr + content.size();

            bool has_v_weights = false;
            bool has_e_weights = false;

            // Parse header
            while (*ptr == '%') {
                ptr = std::find(ptr, end, '\n') + 1; // Skip comment lines
            }

            // Parse and clean the first non-comment line
            while (*ptr == ' ') ++ptr; // Skip leading spaces
            const char* line_end = std::find(ptr, end, '\n'); // Find the end of the line

            // Parse the header
            std::string header(ptr, line_end);
            std::vector<std::string> temp = split(header, ' ');
            ptr = line_end + 1; // Move pointer past the header line

            n = std::stoi(temp[0]);
            m = std::stoi(temp[1]) * 2;

            std::string fmt = temp.size() >= 2 ? temp[3] : "000";
            has_v_weights   = fmt[1] == '1';
            has_e_weights   = fmt[2] == '1';

            v_weights.resize(n, 1);
            vertex_weights = n;
            neighborhoods.resize(n + 1, 0);
            neighborhoods[0] = 0;
            edges.resize(m);
            curr_m = 0;

            // Parse edges
            u64 u = 0;
            while (ptr < end) {
                while (*ptr == '%') {
                    ptr = std::find(ptr, end, '\n') + 1; // Skip comment lines
                }

                const char* line_start = ptr;
                line_end               = std::find(ptr, end, '\n');
                ptr                    = line_end + 1;

                u64 curr_number = 0;
                u64 v           = 0;
                bool active     = false;
                bool vertex_set = false;

                const char* it = line_start;
                if (has_v_weights) {
                    for (; it < line_end; ++it) {
                        char c = *it;
                        if (c == ' ' && active) {
                            v_weights[u] = curr_number;
                            vertex_weights += curr_number - 1;
                            curr_number = 0;
                            active      = false;
                            ++it;
                            break;
                        }
                        curr_number = curr_number * 10 + (c - '0');
                        active      = true;
                    }
                }

                if (has_e_weights) {
                    for (; it < line_end; ++it) {
                        char c = *it;
                        if (c == ' ') {
                            if (active) {
                                if (!vertex_set) {
                                    v          = curr_number - 1;
                                    vertex_set = true;
                                } else {
                                    edges[curr_m].v        = v;
                                    edges[curr_m++].weight = curr_number;
                                    edge_weights += curr_number;
                                }
                            }
                            curr_number = 0;
                            active      = false;
                        } else {
                            curr_number = curr_number * 10 + (c - '0');
                            active      = true;
                        }
                    }
                } else {
                    for (; it < line_end; ++it) {
                        char c = *it;
                        if (c == ' ') {
                            if (active) {
                                edges[curr_m].v        = curr_number - 1;
                                edges[curr_m++].weight = 1;
                                edge_weights += 1;
                            }
                            curr_number = 0;
                            active      = false;
                        } else {
                            curr_number = curr_number * 10 + (c - '0');
                            active      = true;
                        }
                    }
                }

                // Handle the last number on the line
                if (active) {
                    if (!has_e_weights) {
                        edges[curr_m].v        = curr_number - 1;
                        edges[curr_m++].weight = 1;
                        edge_weights += 1;
                    } else {
                        edges[curr_m].v        = v;
                        edges[curr_m++].weight = curr_number;
                        edge_weights += curr_number;
                    }
                }

                neighborhoods[u + 1] = curr_m;
                u += 1;
            }

            edge_weights /= 2;
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
