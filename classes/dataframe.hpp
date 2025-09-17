#pragma once

#include "../config.hpp"
#include "column.hpp"
#include "row_range.hpp"
#include "column_range.hpp"






class df_dataframe_t {
    friend class df_row_range_t;

    using column_range_t = std::vector<std::pair<std::string, df_column_t*>>;


    column_range_t active_columns;      // uniqure_ptr<df_column_t>
    std::vector<df_column_t*> dropped_columns;      // uniqure_ptr<df_column_t>


    std::pair<std::string, df_column_t*>* _find_column(const char* name) const;

    void _drop_column(df_column_t* column);


public:
    ~df_dataframe_t();


    df_dataframe_t();

    df_dataframe_t(const std::initializer_list<std::pair<std::string, df_column_t>>& source_columns);

    // df_dataframe_t(std::initializer_list<std::pair<std::string, df_column_t>>&& source_columns);


    // == copy ==

    df_dataframe_t(const df_dataframe_t& src);

    df_dataframe_t& operator=(const df_dataframe_t& src);


    // == move ==

    df_dataframe_t(df_dataframe_t&& src) noexcept;

    df_dataframe_t& operator=(df_dataframe_t&& src) noexcept;



    // == get ==

    int get_column_count() const;

    long get_row_count() const;



    bool has_column(const char* name) const;


    // get single column

    df_column_t& operator[](const char* name);

    df_column_t& column(const char* name);


    const df_column_t& operator[](const char* name) const;

    const df_column_t column(const char* name) const;


    // get multi column

    df_column_range_t columns(const char* start, const char* end);

    df_column_range_t columns(std::vector<const char*> names);


    df_const_column_range_t columns(const char* start, const char* end) const;

    df_const_column_range_t columns(std::vector<const char*> name) const;



    // get single row

    df_row_t row(long index);

    df_const_row_t row(long index) const;


    // get multi row

    df_row_range_t rows(long start, long end, long interval);

    const df_row_range_t rows(long start, long end, long interval) const;


    // == set ==

    // plan relace to df["name"] = column or df.column("name") = column
    df_dataframe_t& add_column(std::string&& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, df_column_t&& column);

    df_dataframe_t& add_column(const std::string& name, const df_column_t& column);




    // plan replace to df.row(-1) = source
    df_dataframe_t& add_row(const df_const_row_t& source);

    df_dataframe_t& add_row(const std::vector<df_object_t>& row);


    // == drop ==

    void drop_column(const char* column_name);

    void remove_index(long index);


    void clear_columns();

    void clear_rows();


    // == iterator ==

    std::vector<std::pair<std::string, df_column_t*>>::iterator begin();

    std::vector<std::pair<std::string, df_column_t*>>::iterator end();

    std::vector<std::pair<std::string, df_column_t*>>::const_iterator begin() const;

    std::vector<std::pair<std::string, df_column_t*>>::const_iterator end() const;



    // == print ==

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

    friend df_dataframe_t df_read_csv(const char* file);
    friend df_dataframe_t df_read_xlsx(const char* file, const char* sheet_name);
    friend df_dataframe_t df_read_html(const char* file);
    friend df_dataframe_t df_read_db();
    friend df_dataframe_t df_read_json(const char* file);
    friend df_dataframe_t df_read_markdown(const char* file);

    int write_csv(const char* file) const;
    int write_xlsx(const char* file, const char* sheet_name) const;
    int write_html(const char* file) const;
    int write_db() const;
    int write_json(const char* file) const;
    int write_markdown(const char* file) const;
};


