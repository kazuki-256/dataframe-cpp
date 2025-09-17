#pragma once

#include "column.hpp"
#include "row.cpp"
#include "row_range.cpp"



class df_column_range_t {
protected:
    using column_vector_t = std::vector<std::pair<std::string, df_column_t*>>;

    column_vector_t _columns;


    df_column_range_t(const column_vector_t& columns);

    df_column_range_t(const column_vector_t& columns, const char* start, const char* end);

    df_column_range_t(const column_vector_t& columns, const std::vector<const char*> selected_names);

public:
    // == iterator ==

    class iterator_t;

    iterator_t begin() const;
    iterator_t end() const;


    // == range ==

    df_column_range_t columns(const char* start, const char* end) const;

    df_column_range_t columns(std::vector<const char*> selected_names) const;


    df_row_range_t rows(long start = 0, long end = -1, long interval = 1) const;


    // == get ==

    int get_column_count() const;
    int get_row_count() const;



    df_column_t& column(const char* name) const;

    df_row_t row(long index) const;


    // == operator ==

    df_column_range_t operator+(const df_column_range_t& other) const;
    df_column_range_t& operator+=(const df_column_range_t& other);

    df_column_range_t operator+(const df_column_t& other) const;
    df_column_range_t& operator+=(const df_column_t& other);
    
    // == print ==

    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_column_range_t& range);
};



class df_const_column_range_t : public df_column_range_t {
protected:
    df_const_column_range_t(const column_vector_t& columns);

    df_const_column_range_t(const column_vector_t& columns, const std::vector<const char*> selected_names);

public:
    // == iterator ==

    class iterator_t;

    iterator_t begin() const;
    iterator_t end() const;


    // == range ==

    df_const_column_range_t columns(const char* start, const char* end) const;

    df_const_column_range_t columns(std::vector<const char*> selected_names) const;


    df_const_row_range_t rows(long start = 0, long end = -1, long interval = 1) const;


    // == get ==

    const df_column_t& column(const char* name) const;

    df_const_row_t row(long index) const;


    // == operator ==

    df_const_column_range_t operator+(const df_column_range_t& other) const;
    df_const_column_range_t& operator+=(const df_column_range_t& other);

    df_const_column_range_t operator+(const df_column_t& other) const;
    df_const_column_range_t& operator+=(const df_column_t& other);

    
    // == print ==

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_range_t& range);
};