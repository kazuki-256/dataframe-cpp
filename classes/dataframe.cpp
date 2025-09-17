#pragma once

#include "dataframe.hpp"
#include "column.cpp"
#include "row.cpp"
#include "row_range.cpp"

#include <chrono>
#include <thread>








// ==== df_dataframe_t ====

// == destroy ==

df_dataframe_t::~df_dataframe_t() {
    for (auto& pair : active_columns) {
        dropped_columns.emplace_back(pair.second);
    }

    int undeletable_count = 0;
    for (df_column_t* ptr : dropped_columns) {
        while (!ptr->is_deletable()) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        delete ptr;
    }
}



// == create ==

df_dataframe_t::df_dataframe_t() {}


df_dataframe_t::df_dataframe_t(const std::initializer_list<std::pair<std::string, df_column_t>>& sources) {
    df_debug4("create df_dataframe_t by copy");
    active_columns.reserve(sources.size() * 2);

    long length = sources.begin()->second.get_length();

    for (auto& pair : sources) {
        if (length != pair.second.get_length()) {
            throw df_exception_t("not same size active_columns!");
        }

        active_columns.emplace_back(pair.first, new df_column_t(pair.second));
    }
}
/*
df_dataframe_t::df_dataframe_t(std::initializer_list<std::pair<std::string, df_column_t>>&& sources) {
    df_debug4("create df_dataframe_t by move");
    active_columns.reserve(sources.size() * 2);

    for (auto& pair : sources) {
        active_columns.emplace_back(std::move((std::pair<std::string, df_column_t*>&)pair));
    }
}*/



// == copy ==

df_dataframe_t::df_dataframe_t(const df_dataframe_t& src) {
    active_columns.reserve(src.get_column_count() * 2);

    for (auto& pair : src.active_columns) {
        active_columns.emplace_back(pair.first, new df_column_t(*pair.second));
    }
}

df_dataframe_t& df_dataframe_t::operator=(const df_dataframe_t& src) {
    
    active_columns = src.active_columns;
    return *this;
}


// == move ==

df_dataframe_t::df_dataframe_t(df_dataframe_t&& src) noexcept {
    active_columns = std::move(src.active_columns);
}


df_dataframe_t& df_dataframe_t::operator=(df_dataframe_t&& src) noexcept {
    active_columns = std::move(src.active_columns);
    return *this;
}





// == iterator ==

std::vector<std::pair<std::string, df_column_t*>>::iterator df_dataframe_t::begin() {
    return active_columns.begin();
}

std::vector<std::pair<std::string, df_column_t*>>::iterator df_dataframe_t::end() {
    return active_columns.end();
}

std::vector<std::pair<std::string, df_column_t*>>::const_iterator df_dataframe_t::begin() const {
    return active_columns.begin();
}

std::vector<std::pair<std::string, df_column_t*>>::const_iterator df_dataframe_t::end() const {
    return active_columns.end();
}



// == range ==

df_row_range_t df_dataframe_t::rows(long start = 0, long end = -1, long interval = 1) {
    return df_row_range_t(&active_columns, start, end, interval);
}

const df_row_range_t df_dataframe_t::rows(long start = 0, long end = -1, long interval = 1) const {
    return df_row_range_t((column_range_t*)&active_columns, start, end, interval);
}




// == get ==

int df_dataframe_t::get_column_count() const {
    return active_columns.size();
}

long df_dataframe_t::get_row_count() const {
    return active_columns[0].second->get_length();
}





std::pair<std::string, df_column_t*>* df_dataframe_t::_find_column(const char* name) const {
    for (const std::pair<std::string, df_column_t*>& named_column : active_columns) {
        if (named_column.first.compare(name) == 0) {
            return (std::pair<std::string, df_column_t*>*)&named_column;
        }
    }
    return NULL;
}



df_column_t& df_dataframe_t::operator[](const char* name) {
    std::pair<std::string, df_column_t*>* result = _find_column(name);
    if (result == NULL) {
        result = &active_columns.emplace_back(name, new df_column_t());
    }
    return *result->second;
}

const df_column_t& df_dataframe_t::operator[](const char* name) const {
    std::pair<std::string, df_column_t*>* result = _find_column(name);
    if (result == NULL) {
        throw df_exception_out_of_index();
    }
    return *result->second;
}




df_row_t df_dataframe_t::row(long index) {
    return df_row_t(&active_columns, df_calculate_index(index, get_row_count()), 1);
}


df_const_row_t df_dataframe_t::row(long index) const {
    return df_const_row_t(&active_columns, df_calculate_index(index, get_row_count()), 1);
}




// == add_column ==

df_dataframe_t& df_dataframe_t::add_column(std::string&& name, df_column_t&& column) {
    std::pair<std::string, df_column_t*>* result = _find_column(name.c_str());
    if (result == NULL) {
        active_columns.emplace_back(std::move(name), new df_column_t(std::move(column)));
        return *this;
    }
    *result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, df_column_t&& column) {
    std::pair<std::string, df_column_t*>* result = _find_column(name.c_str());
    if (result == NULL) {
        active_columns.emplace_back(name, new df_column_t(std::move(column)));
        return *this;
    }
    *result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, const df_column_t& column) {
    std::pair<std::string, df_column_t*>* result = _find_column(name.c_str());
    if (result == NULL) {
        active_columns.emplace_back(name, new df_column_t(column));
        return *this;
    }
    result->second = new df_column_t(column);
    return *this;
}




df_dataframe_t& df_dataframe_t::add_row(const df_const_row_t& source) {
    return *this;
}




// == print ==

std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df) {
    df.rows().write_stream(os);
    return os;
}