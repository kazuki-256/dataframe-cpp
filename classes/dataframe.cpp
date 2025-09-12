#pragma once

#include "dataframe.hpp"
#include "column.cpp"
#include "row.cpp"
#include "range_rows.cpp"







// ==== df_dataframe_t ====

// == iterator ==

std::vector<df_named_column_t>::iterator df_dataframe_t::begin() {
    return columns.begin();
}

std::vector<df_named_column_t>::iterator df_dataframe_t::end() {
    return columns.end();
}

std::vector<df_named_column_t>::const_iterator df_dataframe_t::begin() const {
    return columns.begin();
}

std::vector<df_named_column_t>::const_iterator df_dataframe_t::end() const {
    return columns.end();
}



inline void df_dataframe_t::basic_range_rows(long& start, long& end, long& interval) const {
    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    const long LENGTH = get_row_count();

    start = df_calculate_index(start, LENGTH);
    end = df_calculate_index(end, LENGTH);

    long range = end - start;
    if (range * interval < 0) {
        throw df_exception_endless_range();
    }
    end = end - range % interval;
}


df_const_range_rows_t df_dataframe_t::range_rows(long start = 0, long end = -1, long interval = 1) const {
    basic_range_rows(start, end, interval);
    return df_const_range_rows_t(this, start, end, interval);
}

df_range_rows_t df_dataframe_t::range_rows(long start = 0, long end = -1, long interval = 1) {
    basic_range_rows(start, end, interval);
    return df_range_rows_t(this, start, end, interval);
}



// == destroy ==

df_dataframe_t::~df_dataframe_t() {
    columns.clear();
}



// == create ==

df_dataframe_t::df_dataframe_t() {}


df_dataframe_t::df_dataframe_t(const std::initializer_list<df_named_column_t>& sources) {
    df_debug4("create df_dataframe_t by copy");
    columns.reserve(sources.size() * 2);

    long length = sources.begin()->second.get_length();

    for (auto& pair : sources) {
        if (length != pair.second.get_length()) {
            throw df_exception_t("not same size columns!");
        }

        columns.emplace_back(pair);
    }
}

df_dataframe_t::df_dataframe_t(std::initializer_list<df_named_column_t>&& sources) {
    df_debug4("create df_dataframe_t by move");
    columns.reserve(sources.size() * 2);

    for (auto& pair : sources) {
        columns.emplace_back(std::move((df_named_column_t&)pair));
    }
}



// == copy ==

df_dataframe_t::df_dataframe_t(const df_dataframe_t& src) {
    columns = src.columns;
}

df_dataframe_t& df_dataframe_t::operator=(const df_dataframe_t& src) {
    if (!columns.empty()) {
        columns.clear();
    }
    columns = src.columns;
    return *this;
}


// == move ==

df_dataframe_t::df_dataframe_t(df_dataframe_t&& src) noexcept {
    columns = std::move(src.columns);
}


df_dataframe_t& df_dataframe_t::operator=(df_dataframe_t&& src) noexcept {
    columns = std::move(src.columns);
    return *this;
}



// == get ==

int df_dataframe_t::get_column_count() const {
    return columns.size();
}

long df_dataframe_t::get_row_count() const {
    return columns[0].second.get_length();
}





df_named_column_t* df_dataframe_t::find_column(const char* name) const {
    for (const df_named_column_t& named_column : columns) {
        if (named_column.first.compare(name) == 0) {
            return (df_named_column_t*)&named_column;
        }
    }
    return NULL;
}



df_column_t& df_dataframe_t::operator[](const char* name) {
    df_named_column_t* result = find_column(name);
    if (result == NULL) {
        throw df_exception_out_of_index();
    }
    return result->second;
}

const df_column_t& df_dataframe_t::operator[](const char* name) const {
    df_named_column_t* result = find_column(name);
    if (result == NULL) {
        throw df_exception_out_of_index();
    }
    return result->second;
}




df_row_t df_dataframe_t::loc(long index) {
    return df_row_t(&columns, df_calculate_index(index, get_row_count()), 1);
}


df_const_row_t df_dataframe_t::loc(long index) const {
    return df_const_row_t(&columns, df_calculate_index(index, get_row_count()), 1);
}




// == add_column ==

df_dataframe_t& df_dataframe_t::add_column(std::string&& name, df_column_t&& column) {
    last_column_update = time(NULL);

    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(std::move(name), std::move(column));
        return *this;
    }
    result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, df_column_t&& column) {
    last_column_update = time(NULL);
    
    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(name, std::move(column));
        return *this;
    }
    result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, const df_column_t& column) {
    last_column_update = time(NULL);
    
    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(name, column);
        return *this;
    }
    result->second = column;
    return *this;
}




df_dataframe_t& df_dataframe_t::add_row(const df_const_row_t& source) {
    return *this;
}




// == print ==

std::ostream& df_dataframe_t::write_stream(std::ostream& os) const {
    return range_rows().write_stream(os);
}



std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df) {
    df.write_stream(os);
    return os;
}