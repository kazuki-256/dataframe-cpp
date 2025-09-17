#pragma once

#include "column_range.hpp"
#include "column.cpp"




// == make ==

df_column_range_t::df_column_range_t(const column_vector_t& columns)
: _columns(columns) {}


df_column_range_t::df_column_range_t(const column_vector_t& columns, const char* start, const char* end) {
    _columns.reserve(columns.size());

    int index = 0;
    for (; index < columns.size(); index++) {
        if (columns[index].first.compare(start) == 0) {
            break;
        }
    }

    for (; index < columns.size(); index++) {
        _columns.emplace_back(columns[index]);

        if (columns[index].first.compare(end) == 0) {
            break;
        }
    }
}


df_column_range_t::df_column_range_t(const column_vector_t& columns, const std::vector<const char*> selected_names) {
    
}



