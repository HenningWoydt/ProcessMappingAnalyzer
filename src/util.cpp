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
#include "util.h"

namespace ProMapAnalyzer {
    void line_to_ints(const std::string& line,
                      std::vector<int>& ints) {
        ints.clear();
        int curr_number = 0;
        bool active     = false;

        for (const char c : line) {
            if (c == ' ') {
                if (active) {
                    ints.push_back(curr_number);
                }
                curr_number = 0;
                active      = false;
            } else {
                curr_number = curr_number * 10 + (c - '0');
                active      = true;
            }
        }

        if (active) {
            ints.push_back(curr_number);
        }
    }

    bool file_exists(const std::string& path) {
        const std::ifstream f(path.c_str());
        return f.good();
    }

    std::vector<std::string> split(const std::string& str,
                                   const char c) {
        std::vector<std::string> splits;

        std::istringstream iss(str);
        std::string token;

        while (std::getline(iss, token, c)) {
            if (!token.empty()) {
                splits.push_back(token);
            }
        }

        return splits;
    }

    std::vector<int> read_partition(const std::string& path) {
        if (!file_exists(path)) {
            std::cerr << "File " << path << " does not exist!" << std::endl;
            std::abort();
        }

        std::vector<int> partition;

        std::ifstream file(path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (line[0] == 'c') { continue; }
                if (line.back() == '\n' || line.back() == '\r') { line.pop_back(); }

                partition.push_back(std::stoi(line));
            }
        }

        return partition;
    }

    void generate_communication_graph(std::string& hierarchy,
                                      std::string& distance,
                                      std::string& file_path) {
        std::vector<std::string> h_str = split(hierarchy, ':');
        std::vector<long int> h             = convert<long int>(h_str);
        std::reverse(h.begin(), h.end());
        std::vector<std::string> d_str = split(distance, ':');
        std::vector<long int> d             = convert<long int>(d_str);

        std::vector<std::vector<long int>> hierarchy_indices;
        for (long int x : h) {
            hierarchy_indices.emplace_back();
            hierarchy_indices.reserve(x);
            for (long int i = 0; i < x; ++i) {
                hierarchy_indices.back().push_back(i);
            }
        }

        std::vector<std::vector<long int>> vertex_identifier;

        // Vector to store the current indices for each vector
        std::vector<size_t> indices(hierarchy_indices.size(), 0);

        while (true) {
            // Find the rightmost vector that has more elements left after the current element
            vertex_identifier.emplace_back();
            for (size_t i = 0; i < hierarchy_indices.size(); ++i) {
                vertex_identifier.back().push_back(hierarchy_indices[i][indices[i]]);
            }

            int i = static_cast<int>(hierarchy_indices.size()) - 1;
            for (; i >= 0; --i) {
                if (indices[i] + 1 < hierarchy_indices[i].size()) {
                    indices[i]++;
                    break;
                }
                indices[i] = 0; // Reset the index and carry the increment to the next vector
            }

            // If no such vector is found, we're done
            if (i < 0) {
                break;
            }
        }

        auto k = prod<long int>(h);
        std::vector<long int> dist_mtx(k * k);
        std::fill(dist_mtx.begin(), dist_mtx.end(), static_cast<long int>(0));

        for (long int i = 0; i < k; ++i) {
            for (long int j = 0; j < k; ++j) {
                if (i == j) {
                    dist_mtx[i * k + j] = 0;
                } else {
                    std::vector<long int>& ident1 = vertex_identifier[i];
                    std::vector<long int>& ident2 = vertex_identifier[j];

                    for (long int l = 0; l < static_cast<int>(h.size()); ++l) {
                        if (ident1[l] != ident2[l]) {
                            dist_mtx[i * k + j] = d[h.size() - 1 - l];
                            break;
                        }
                    }
                }
            }
        }

        // write graph
        long int n_edges     = 0;
        std::string fmt = "001";
        std::string content;
        for (long int i = 0; i < k; ++i) {
            std::string line;
            for (long int j = i + 1; j < k; ++j) {
                line += "" + std::to_string(j + 1) + " " + std::to_string(dist_mtx[i * k + j]) + " ";
                n_edges++;
            }
            if (!line.empty()) {
                line.pop_back();
            }
            content.append(line + "\n");
        }
        content = "" + std::to_string(k) + " " + std::to_string(n_edges) + " " + fmt + "\n" + content;

        std::ofstream out_file;
        out_file.open(file_path);
        out_file << content;
        out_file.close();
    }
}
