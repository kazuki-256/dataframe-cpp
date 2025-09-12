#pragma once

#include "../config.hpp"
#include "row_range.hpp"
#include "dataframe.hpp"

#include "column.cpp"
#include "row.cpp"



// == make ==

df_row_range_t::df_row_range_t(df_dataframe_t* df, long start_index, long end_index, long interval) {
    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    const long LENGTH = df->get_row_count();

    this->df = df;
    this->start_index = df_calculate_index(start_index, LENGTH);
    this->end_index = df_calculate_index(end_index, LENGTH) + 1;
    this->interval = interval;

    const long RANGE = this->end_index - this->start_index;
    if (RANGE * this->interval < 0) {
        throw df_exception_endless_range();
    }
    this->end_index = this->end_index - RANGE % this->interval;
}


// ==  ==

df_row_t df_row_range_t::begin() const {
    return df_row_t(&df->columns, start_index, interval);
}


df_row_t df_row_range_t::end() const {
    return df_row_t(end_index);
}



// == write ==

std::ostream& df_row_range_t::write_stream(std::ostream& os) const {
    const int COLUMN_COUNT = df->get_column_count();

    // loader, writer, iterator
    struct write_info_t {
        df_memory_iterator_t iter = {NULL, 0L};
        df_value_load_callback_t loader;
        df_value_write_callback_t writer;
    } write_info_list[COLUMN_COUNT];

    // == print titles ==

    os << "| ";
    int index = 0;
    for (auto& named_column : df->columns) {
        os << named_column.first << " | ";

        write_info_t& info = write_info_list[index++];
        info.iter = named_column.second.memory_begin();
        info.loader = named_column.second.type_loader;
        info.writer = df_value_get_write_callback(named_column.second.data_type, DF_TYPE_TEXT);
    }
    os << "\n";

    // == print data ==

    write_info_t* END = write_info_list + COLUMN_COUNT;
    std::string s(32, '\0');
    
    for (int row = start_index; row < end_index; row += interval) {
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

df_const_row_range_t::df_const_row_range_t(const df_dataframe_t* df, long start_index, long end_index, long interval)
    : df_row_range_t((df_dataframe_t*)df, start_index, end_index, interval) {}


// == iterator ==

df_const_row_t df_const_row_range_t::begin() {
    return df_const_row_t(&df->columns, start_index, interval);
}

df_const_row_t df_const_row_range_t::end() {
    return df_const_row_t(end_index);
}


// == print ==

std::ostream& operator<<(std::ostream& os, const df_const_row_range_t& range) {
    return range.write_stream(os);
}



