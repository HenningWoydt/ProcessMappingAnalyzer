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
#ifndef PROCESSMAPPINGANALYZER_DEFINITIONS_H
#define PROCESSMAPPINGANALYZER_DEFINITIONS_H

#include <algorithm>

namespace ProMapAnalyzer {
    typedef int8_t s8;
    typedef int16_t s16;
    typedef int32_t s32;
    typedef int64_t s64;

    typedef u_int8_t u8;
    typedef u_int16_t u16;
    typedef u_int32_t u32;
    typedef u_int64_t u64;

    typedef float f32;
    typedef double f64;

    typedef u32 vertex_t;
    typedef s32 weight_t;
    typedef u32 partition_t;
}

#endif //PROCESSMAPPINGANALYZER_DEFINITIONS_H
