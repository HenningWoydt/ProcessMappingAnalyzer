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
#ifndef PROCESSMAPPINGANALYZER_UTIL_H
#define PROCESSMAPPINGANALYZER_UTIL_H

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace ProMapAnalyzer {
    void line_to_ints(const std::string& line,
                      std::vector<int>& ints);

    bool file_exists(const std::string& path);

    template <typename T1, typename T2>
    T1 sum(const std::vector<T2>& vec) {
        T1 s = static_cast<T1>(0);

        for (auto& x : vec) {
            s += static_cast<T1>(x);
        }

        return s;
    }

    template <typename T>
    T max(const std::vector<T>& vec) {
        T m = vec[0];

        for (auto& x : vec) {
            m = std::max(m, x);
        }

        return m;
    }

    template <typename T>
    T min(const std::vector<T>& vec) {
        T m = vec[0];

        for (auto& x : vec) {
            m = std::min(m, x);
        }

        return m;
    }

    template <typename T1, typename T2>
    T1 prod(const std::vector<T2>& vec) {
        T1 p = static_cast<T1>(1);

        for (auto& x : vec) {
            p *= static_cast<T1>(x);
        }

        return p;
    }

    template <typename T>
    T convert_to(const std::string& str) {
        T result;
        std::istringstream iss(str);
        iss >> result;
        return result;
    }

    template <typename T>
    std::vector<T> convert(const std::vector<std::string>& vec) {
        std::vector<T> v;

        for (auto& s : vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    template <typename T>
    std::vector<T> convert(const std::vector<std::string>&& vec) {
        std::vector<T> v;

        for (auto& s : vec) {
            v.push_back(convert_to<T>(s));
        }

        return v;
    }

    std::vector<std::string> split(const std::string& str,
                                   char c);

    std::vector<int> read_partition(const std::string& path);

    template <typename T>
    void printVectorOfVectors(const std::vector<std::vector<T>>& vecs) {
        // Determine the maximum length of vectors for spacing
        size_t maxLen = 0;
        for (const auto& vec : vecs) {
            if (vec.size() > maxLen) {
                maxLen = vec.size();
            }
        }

        // Print each vector
        for (const auto& vec : vecs) {
            for (const auto& elem : vec) {
                std::cout << std::setw(3) << elem << " "; // Adjust the spacing as needed
            }
            std::cout << std::endl;
        }
    }

    void generate_communication_graph(std::string& hierarchy,
                                      std::string& distance,
                                      std::string& file_path);

    template <typename T>
    std::string vectorToString(const std::vector<T>& vec) {
        std::ostringstream oss;
        oss << "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i != vec.size() - 1) {
                oss << ", ";
            }
        }
        oss << "]";
        return oss.str();
    }
}

#endif //PROCESSMAPPINGANALYZER_UTIL_H
