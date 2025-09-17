#pragma once

#include "../config.hpp"
#include "column.hpp"



class df_object_range_t {
    friend class df_column_t;
protected:
    df_column_t* _column = NULL;
    long _start = 0;
    long _end = -1;
    long _interval = 1;

    // == make ==

    constexpr df_object_range_t(df_column_t* column, long start, long end, long interval);

public:
    // == iterator ==

    df_memory_iterator_t memory_begin() const;
    df_memory_iterator_t memory_end() const;

    df_object_iterator_t begin() const;
    df_object_iterator_t end() const;


    // == print ==

    std::ostream& write_stream(std::ostream& os, const char* outside_name) const;

    friend std::ostream& operator<<(std::ostream& os, const df_object_range_t& range);
};



class df_const_object_range_t : df_object_range_t {
    friend class df_column_t;

    constexpr df_const_object_range_t(const df_column_t* column, long start_index, long end_index, long interval);
public:
    // == iterator ==

    df_const_memory_iterator_t memory_begin() const;
    df_const_memory_iterator_t memory_end() const;

    df_const_object_iterator_t begin() const;
    df_const_object_iterator_t end() const;


    friend std::ostream& operator<<(std::ostream& os, const df_const_object_range_t& range);
};




