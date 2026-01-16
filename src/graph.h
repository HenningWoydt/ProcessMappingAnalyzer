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
    class Graph {
    public:
        vertex_t n = 0;
        vertex_t m = 0;

        weight_t vertex_weights = 0;
        std::vector<weight_t> v_weights;

        std::vector<u64> neighborhoods;
        std::vector<vertex_t> edges_v;
        std::vector<weight_t> edges_w;

        explicit Graph(const std::string &file_path) {
            if (!file_exists(file_path)) {
                std::cerr << "File " << file_path << " does not exist!" << std::endl;
                exit(EXIT_FAILURE);
            }

            // mmap the whole file
            MMap mm = mmap_file_ro(file_path);
            char *p = mm.data;
            const char *end = mm.data + mm.size;

            // skip comment lines
            while (*p == '%') {
                while (*p != '\n') { ++p; }
                ++p;
            }

            // skip whitespace
            while (*p == ' ') { ++p; }

            // read number of vertices - optimized parsing
            n = 0;
            while (*p != ' ' && *p != '\n') {
                n = n * 10 + (vertex_t) (*p - '0');
                ++p;
            }

            // skip whitespace
            while (*p == ' ') { ++p; }

            // read number of edges - optimized parsing
            m = 0;
            while (*p != ' ' && *p != '\n') {
                m = m * 10 + (vertex_t) (*p - '0');
                ++p;
            }
            m *= 2;

            // search end of line or fmt
            std::string fmt = "000";
            bool has_v_weights = false;
            bool has_e_weights = false;
            while (*p == ' ') { ++p; }
            if (*p != '\n') {
                // found fmt
                fmt[0] = *p;
                ++p;
                if (*p != '\n') {
                    // found fmt
                    fmt[1] = *p;
                    ++p;
                    if (*p != '\n') {
                        // found fmt
                        fmt[2] = *p;
                        ++p;
                    }
                }
                // skip whitespaces
                while (*p == ' ') { ++p; }
            }
            vertex_weights = 0;
            v_weights.resize(n);
            neighborhoods.resize(n + 1);
            neighborhoods[0] = 0;
            edges_v.resize(m);
            edges_w.resize(m);
            has_v_weights = fmt[1] == '1';
            has_e_weights = fmt[2] == '1';

            ++p;
            vertex_t u = 0;
            size_t curr_m = 0;

            while (p < end) {
                // skip comment lines
                while (*p == '%') {
                    while (*p != '\n') { ++p; }
                    ++p;
                }

                // skip whitespaces
                while (*p == ' ') { ++p; }

                // read in vertex weight - optimized
                weight_t vw = 1;
                if (has_v_weights) {
                    vw = 0;
                    while (*p != ' ' && *p != '\n') {
                        vw = vw * 10 + (weight_t) (*p - '0');
                        ++p;
                    }
                    // skip whitespaces
                    while (*p == ' ') { ++p; }
                }
                v_weights[u] = vw;
                vertex_weights += vw;

                // read in edges - optimized inner loop
                while (*p != '\n' && p < end) {
                    vertex_t v = 0;
                    while (*p != ' ' && *p != '\n') {
                        v = v * 10 + (vertex_t) (*p - '0');
                        ++p;
                    }

                    // skip whitespaces
                    while (*p == ' ') { ++p; }

                    weight_t w = 1;
                    if (has_e_weights) {
                        w = 0;
                        while (*p != ' ' && *p != '\n') {
                            w = w * 10 + (weight_t) (*p - '0');
                            ++p;
                        }
                        // skip whitespaces
                        while (*p == ' ') { ++p; }
                    }

                    edges_v[curr_m] = v - 1;
                    edges_w[curr_m] = w;
                    ++curr_m;
                }
                neighborhoods[u + 1] = (vertex_t) curr_m;
                ++u;
                ++p;
            }

            if (curr_m != m) {
                std::cerr << "Number of expected edges " << m << " not equal to number edges " << curr_m << " found!\n";
                munmap_file(mm);
                exit(EXIT_FAILURE);
            }

            // done with the file
            munmap_file(mm);
        }
    };
}

#endif //PROCESSMAPPINGANALYZER_GRAPH_H
