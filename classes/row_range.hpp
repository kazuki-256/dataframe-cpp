#pragma once

#include "../config.hpp"
#include "row.hpp"


class df_row_range_t {
    friend class df_dataframe_t;

protected:
    using column_range_t = std::vector<std::pair<std::string, df_column_t*>>;

    column_range_t* _columns;
    long _start;
    long _end;
    long _interval;

    // == make ==

    df_row_range_t(column_range_t* columns, long start, long end, long interval);
public:
    // == iterator ==

    df_row_t begin() const;

    df_row_t end() const;


    // == get ==

    int get_column_count() const;

    int get_row_count() const;


    // == print ==

    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_row_range_t& range);
};


class df_const_row_range_t : public df_row_range_t {
    friend class df_dataframe_t;

    // == make ==

    df_const_row_range_t(const column_range_t* columns, long start, long end, long interval);

public:
    // == iterator ==

    df_const_row_t begin() const;

    df_const_row_t end() const;


    // == print ==

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_range_t& range);
};


