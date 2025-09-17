#pragma once

#include "../config.hpp"
#include "row_range.hpp"
#include "dataframe.hpp"

#include "column.cpp"
#include "row.cpp"



// == make ==

df_row_range_t::df_row_range_t(column_range_t* columns, long start, long end, long interval) {
    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    const long LENGTH = columns->front().second->get_length();

    _columns = columns;
    _start = df_calculate_index(start, LENGTH);
    _end = df_calculate_index(end, LENGTH) + 1;
    _interval = interval;

    const long RANGE = _end - _start;
    if (RANGE * _interval < 0) {
        throw df_exception_endless_range();
    }
    _end = _end - RANGE % _interval;
}


// == iterator ==

df_row_t df_row_range_t::begin() const {
    return df_row_t(_columns, _start, _interval);
}

df_row_t df_row_range_t::end() const {
    return df_row_t(_end);
}



// == get ==

int df_row_range_t::get_column_count() const {
    return _columns->size();
}

int df_row_range_t::get_row_count() const {
    return _columns->front().second->get_length();
}





// == write ==

std::ostream& df_row_range_t::write_stream(std::ostream& os) const {
    const int COLUMN_COUNT = get_column_count();

    // loader, writer, iterator
    struct write_info_t {
        df_memory_iterator_t iter = {NULL, 0L};
        df_value_load_callback_t loader;
        df_value_write_callback_t writer;
    } write_info_list[COLUMN_COUNT];

    // == print titles ==

    os << "| ";
    int index = 0;
    for (auto& pair : *_columns) {
        os << pair.first << " | ";

        write_info_t& info = write_info_list[index++];
        info.iter = pair.second->memory_begin();
        info.loader = pair.second->type_loader;
        info.writer = df_value_get_write_callback(pair.second->data_type, DF_TYPE_TEXT);
    }
    os << "\n";

    // == print data ==

    write_info_t* END = write_info_list + COLUMN_COUNT;
    std::string s(32, '\0');
    
    for (long index = _start; index < _end; index += _interval) {
        os << "| ";
        for (write_info_t* info = write_info_list; info < END; info->iter++, info++) {
            if (*info->iter.get_null()) {
                os << "null | ";
                continue;
            }

            // const uint8_t* ptr = info->iter.get_value();
            df_value_t value = info->loader(info->iter.get_value());
            info->writer(value, &s);

            os << s << " | ";
        }
        os << "\n";
    }
    return os;
}



std::ostream& operator<<(std::ostream& os, const df_row_range_t& range) {
    return range.write_stream(os);
}




// ==== df_const_row_range_t ====

// == make ==

df_const_row_range_t::df_const_row_range_t(const column_range_t* columns, long start, long end, long interval)
    : df_row_range_t((column_range_t*)columns, start, end, interval) {}


// == iterator ==

df_const_row_t df_const_row_range_t::begin() const {
    return df_const_row_t(_columns, _start, _interval);
}


df_const_row_t df_const_row_range_t::end() const {
    return df_const_row_t(_end);
}


// == print ==

std::ostream& operator<<(std::ostream& os, const df_const_row_range_t& range) {
    return range.write_stream(os);
}



