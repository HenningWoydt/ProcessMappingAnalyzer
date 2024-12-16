# /* Process Mapping Analyzer.
# Copyright (C) 2024  Henning Woydt
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or any
# later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
# ==============================================================================*/
cmake_minimum_required(VERSION 3.16)

project(ProcessMappingAnalyzer CXX)
set(PROJECT_VENDOR "Henning Woydt")
set(PROJECT_CONTACT "henning.woydt@informatik.uni-heidelberg.de")
set(PROJECT_URL "https://github.com/HenningWoydt/ProcessMappingAnalyzer")
set(PROJECT_DESCRIPTION "ProcessMappingAnalyzer: Given a graph, a partition, a hierarchy and distances for Process Mapping it determines statistics")
set(PROJECT_VERSION "1.0.0")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler Flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0 -Wall -Wextra -pedantic")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO  "-O3 -g3 -DNDEBUG -march=native -Wall -Wextra -pedantic")

# Find all source and header files
file(GLOB_RECURSE PMA_SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE PMA_HEADERS CONFIGURE_DEPENDS "src/*.h")

# Main executable
add_executable(processmappinganalyzer
        main.cpp
        ${PMA_HEADERS}
        ${PMA_SOURCES})
