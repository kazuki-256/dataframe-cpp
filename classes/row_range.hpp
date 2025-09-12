#pragma once

#include "../config.hpp"
#include "row.hpp"


class df_row_range_t {
    friend class df_dataframe_t;

protected:
    df_dataframe_t* df;
    long start_index;
    long end_index;
    long interval;

public:
    
    df_row_range_t(df_dataframe_t* df, long start_index, long end_index, long interval);



    df_row_t begin() const;

    df_row_t end() const;



    std::ostream& write_stream(std::ostream& os) const ;


    friend std::ostream& operator<<(std::ostream& os, const df_row_range_t& range);
};


class df_const_row_range_t : public df_row_range_t {
    friend class df_dataframe_t;

public:
    df_const_row_range_t(const df_dataframe_t* df, long start_index, long end_index, long interval);

    df_const_row_t begin();

    df_const_row_t end();

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_range_t& range);
};


