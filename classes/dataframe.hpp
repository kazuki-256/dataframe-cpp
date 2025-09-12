#pragma once

#include "../config.hpp"
#include "column.hpp"
#include "row_range.hpp"






class df_dataframe_t {
    friend class df_row_range_t;
    friend class df_const_row_range_t;


    std::vector<df_named_column_t> columns;
    
    // == statistics ==
    df_date_t last_column_update;

    df_named_column_t* find_column(const char* name) const;


public:
    ~df_dataframe_t();


    df_dataframe_t();

    df_dataframe_t(const std::initializer_list<df_named_column_t>& source_columns);

    // df_dataframe_t(std::initializer_list<df_named_column_t>&& source_columns);


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


    df_row_t row(long index);

    df_const_row_t row(long index) const;



    // == add ==

    df_dataframe_t& add_column(std::string&& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, const df_column_t& column);



    df_dataframe_t& add_row(const df_const_row_t& source);

    df_dataframe_t& add_row(const std::vector<df_object_t>& row);

    df_dataframe_t& add_row(const std::initializer_list<df_object_t>& row);



    // == iterator ==

    std::vector<df_named_column_t>::iterator begin();

    std::vector<df_named_column_t>::iterator end();

    std::vector<df_named_column_t>::const_iterator begin() const;

    std::vector<df_named_column_t>::const_iterator end() const;



    df_row_range_t rows(long start, long end, long interval);

    df_const_row_range_t rows(long start, long end, long interval) const;



    // == print ==

    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df);



    // == sql ==

    df_query_t select(const char* sql) const;

    df_query_t where(const char* sql) const;

    df_query_t group_by(const char* sql) const;

    df_query_t order_by(const char* sql) const;

    df_query_t with(df_dataframe_t& other, const char* as_name) const;


    // == vector ==

    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator+(long num) const;
    df_query_t operator+(double num) const;

    df_query_t operator-(const df_query_t& query) const;
    df_query_t operator-(long num) const;
    df_query_t operator-(double num) const;

    df_query_t operator*(const df_query_t& query) const;
    df_query_t operator*(long num) const;
    df_query_t operator*(double num) const;

    df_query_t operator/(const df_query_t& query) const;
    df_query_t operator/(long num) const;
    df_query_t operator/(double num) const;

    df_query_t operator%(const df_query_t& query) const;
    df_query_t operator%(long num) const;
    df_query_t operator%(double num) const;


    // == read/write ==

    friend df_column_t df_read_csv(const char* file);
    friend df_column_t df_read_xlsx(const char* file, const char* sheet_name);
    friend df_column_t df_read_html(const char* file);
    friend df_column_t df_read_db();
    friend df_column_t df_read_json(const char* file);
    friend df_column_t df_read_markdown(const char* file);

    int write_csv(const char* file) const;
    int write_xlsx(const char* file, const char* sheet_name) const;
    int write_html(const char* file) const;
    int write_db() const;
    int write_json(const char* file) const;
    int write_markdown(const char* file) const;
};


