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
class df_row_t {
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

    df_date_t last_update = (time_t)0;       // if column update, remake column_info and match_info
    df_date_t* check_update = NULL;



    df_row_t(std::vector<df_named_column_t>& columns, df_date_t& check_update);

    constexpr df_row_t(long index);

public:
    ~df_row_t();

    df_row_t& operator++();

    df_row_t& operator*();

    df_object_t& operator[](const char* name);

    bool operator!=(const df_row_t& other);

    
    
    class iterator_t;

    iterator_t begin();
    
    iterator_t end();


    
    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df);
};


class df_const_row_t : public df_row_t {
    friend class df_dataframe_t;


    df_const_row_t(const std::vector<df_named_column_t>& columns, const df_date_t& check_update);

    constexpr df_const_row_t(long index);
public:
    const df_object_t& operator[](const char* name);
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

    df_dataframe_t(df_dataframe_t& src);

    df_dataframe_t& operator=(df_dataframe_t& src);


    // == move ==

    df_dataframe_t(df_dataframe_t&& src) noexcept;



    // == get ==

    int get_column_count() const;

    long get_row_count() const;



    df_column_t& operator[](const char* name);

    const df_column_t& operator[](const char* name) const;


    df_row_t& loc(int index);

    df_row_t& loc(int index) const;



    // == add ==

    df_dataframe_t& add_column(std::string&& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, const df_column_t& column);



    df_dataframe_t& add_row(const df_row_t& row);

    df_dataframe_t& add_row(const std::vector<df_object_t>& row);

    df_dataframe_t& add_row(const std::initializer_list<df_object_t>& row);



    // == iterator ==

    df_row_t begin();

    df_row_t end();

    df_const_row_t begin() const;

    df_const_row_t end() const;



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


