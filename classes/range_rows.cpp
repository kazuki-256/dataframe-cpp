#pragma once

#include "../config.hpp"
#include "range_rows.hpp"
#include "dataframe.hpp"

#include "row.cpp"



// == make ==

df_range_rows_t::df_range_rows_t(df_dataframe_t* df, long start_index, long end_index, long interval) {
    this->df = df;
    this->start_index = start_index;
    this->end_index = end_index;
    this->interval = interval;
}


// ==  ==

df_row_t df_range_rows_t::begin() const {
    return df_row_t(&df->columns, start_index, interval);
}


df_row_t df_range_rows_t::end() const {
    return df_row_t(end_index);
}



// == write ==

std::ostream& df_range_rows_t::write_stream(std::ostream& os) const {
    const int COLUMN_COUNT = df->get_column_count();

    // loader, writer, iterator
    struct write_info_t {
        df_column_t::memory_iterator_t iter = 0L;
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



std::ostream& operator<<(std::ostream& os, const df_range_rows_t& range) {
    return range.write_stream(os);
}




// ==== df_const_range_rows_t ====

// == make ==

df_const_range_rows_t::df_const_range_rows_t(const df_dataframe_t* df, long start_index, long end_index, long interval)
    : df_range_rows_t((df_dataframe_t*)df, start_index, end_index, interval) {}


df_const_row_t df_const_range_rows_t::begin() {
    return df_const_row_t(&df->columns, start_index, interval);
}

df_const_row_t df_const_range_rows_t::end() {
    return df_const_row_t(end_index);
}

std::ostream& operator<<(std::ostream& os, const df_const_range_rows_t& range) {
    return range.write_stream(os);
}



