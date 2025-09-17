#pragma once

#include "column.hpp"
#include "object.cpp"
#include "object_iterator.cpp"


// ==== df_object_range_t ====

// == make ==

constexpr df_object_range_t::df_object_range_t(df_column_t* column, long start, long end, long interval)
: _column(column), _start(start), _end(end), _interval(interval) {}



// == iterator ==

df_memory_iterator_t df_object_range_t::memory_begin() const {
    return df_memory_iterator_t(_column, _start, _interval);
}

df_memory_iterator_t df_object_range_t::memory_end() const {
    return df_memory_iterator_t(_column, _end, _interval);
}


df_object_iterator_t df_object_range_t::begin() const {
    return df_object_iterator_t(_column, _start, _interval);
}

df_object_iterator_t df_object_range_t::end() const {
    return df_object_iterator_t(_column, _end, _interval);
}






// == print ==

std::ostream& df_object_range_t::write_stream(std::ostream& os, const char* outside_name) const {
    df_value_write_callback_t writer = df_value_get_write_callback(_column->data_type, DF_TYPE_TEXT);

    df_memory_iterator_t iter = memory_begin();
    df_value_t value;
    std::string buf;

    os << "| " << outside_name << " |\n";
    for (; iter != memory_end(); iter++) {
        if (*iter.get_null()) {
            os << "| null |\n";
            continue;
        }

        value = _column->type_loader(iter.get_value());
        writer(value, &buf);

        os << "| " << buf << " |\n";
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const df_object_range_t& range) {
    return range.write_stream(os, "VALUES");
}





// ==== df_const_object_range_t ====

// == make ==

constexpr df_const_object_range_t::df_const_object_range_t(const df_column_t* column, long start_index, long end_index, long interval)
: df_object_range_t((df_column_t*)column, start_index, end_index, interval) {}



df_const_memory_iterator_t df_const_object_range_t::memory_begin() const {
    return df_const_memory_iterator_t(_column, _start, _interval);
}

df_const_memory_iterator_t df_const_object_range_t::memory_end() const {
    return df_const_memory_iterator_t(_column, _end, _interval);
}


df_const_object_iterator_t df_const_object_range_t::begin() const {
    return df_const_object_iterator_t(_column, _start, _interval);
}

df_const_object_iterator_t df_const_object_range_t::end() const {
    return df_const_object_iterator_t(_column, _end, _interval);
}




// == print ==

std::ostream& operator<<(std::ostream& os, const df_const_object_range_t& range) {
    return range.write_stream(os, "VALUES");
}

