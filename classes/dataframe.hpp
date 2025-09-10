#pragma once

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_COLUMN_HPP_
#include "column.hpp"
#endif

#include <tuple>



/*
three step to find target:
1. from matched cashe, a pointer number to target map
2. if step 1 couldn't find target, find target from object list, a struct to store running variables, also make matched cashe
3. if step 1 and 2 couldn't finded, create object iter and matched cashe from real data (df_column_t)

*/
class df_const_row_t {
    friend class df_dataframe_t;
protected:
    struct matched_info_t;
    struct object_info_t;

    std::vector<df_named_column_t>* unextended_columns = NULL;

    matched_info_t* matched_start = NULL;   // base size: (COLUMN_LENGTH * 2 + 1), realloc every p >= match_end
    matched_info_t* matched_end = NULL;     // at match_start + match_length

    object_info_t* object_start = NULL;     // size: COLUMN_LENGTH
    object_info_t* object_end = NULL;       // at info_start + length (dynamic), don't worry out of size since it is no possible to 

    long current = 0;
    long interval = 0;


    df_const_row_t(const std::vector<df_named_column_t>* columns, long index, long interval);

    constexpr df_const_row_t(long index);

public:
    // == destroy ==

    ~df_const_row_t();

    // == get ==

    int get_length() const;



    df_const_row_t& operator*();

    const df_object_t& operator[](const char* name);

    // == other ==

    df_const_row_t& operator++();

    bool operator!=(const df_const_row_t& other);

    
    // == iterator ==
    
    class const_iterator_t;
    
    class iterator_t;


    iterator_t begin();
    
    iterator_t end();

    const_iterator_t begin() const;
    
    const_iterator_t end() const;


    // == write_stream ==
    
    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_t& row);
};


class df_row_t : public df_const_row_t {
    friend class df_dataframe_t;


    df_row_t(std::vector<df_named_column_t>* columns, long index, long interval);

    constexpr df_row_t(long index);
public:
    df_object_t& operator[](const char* name);
};








class df_dataframe_t {
    std::vector<df_named_column_t> columns;
    
    // == statistics ==
    df_date_t last_column_update;

    df_named_column_t* find_column(const char* name) const;


public:
    ~df_dataframe_t();


    df_dataframe_t();

    df_dataframe_t(const std::initializer_list<df_named_column_t>& source_columns);

    df_dataframe_t(std::initializer_list<df_named_column_t>&& source_columns);


    // == copy ==

    df_dataframe_t(const df_dataframe_t& src);

    df_dataframe_t& operator=(const df_dataframe_t& src);


    // == move ==

    df_dataframe_t(df_dataframe_t&& src) noexcept;

    df_dataframe_t& operator=(df_dataframe_t&& src) noexcept;



    // == get ==

    int get_column_count() const;

    long get_row_count() const;



    df_column_t& operator[](const char* name);

    const df_column_t& operator[](const char* name) const;


    df_row_t loc(long index);

    const df_const_row_t loc(long index) const;



    // == add ==

    df_dataframe_t& add_column(std::string&& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, const df_column_t& column);



    df_dataframe_t& add_row(const df_const_row_t& source& row);

    df_dataframe_t& add_row(const std::vector<df_object_t>& row);

    df_dataframe_t& add_row(const std::initializer_list<df_object_t>& row);



    // == iterator ==

    std::vector<df_named_column_t>::iterator begin();

    std::vector<df_named_column_t>::iterator end();

    std::vector<df_named_column_t>::const_iterator begin() const;

    std::vector<df_named_column_t>::const_iterator end() const;


    
    class range_rows_t;

    range_rows_t range_rows(long start, long end, long interval);



    // == print ==

    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df);



    // == sql ==

    df_query_t select(const char* sql) const;

    df_query_t where(const char* sql) const;

    df_query_t group_by(const char* sql) const;

    df_query_t order_by(const char* sql) const;

    df_query_t with(df_dataframe_t& other, const char* as_name) const;
};


