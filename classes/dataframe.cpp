#pragma once

#include "dataframe.hpp"
#include "column.cpp"


// ==== df_row_t ====

// == df_row_t::row_info_t ==

struct df_row_t::row_info_t {
    const std::string* name;
    long last_index;
    df_object_iterator_t iter;
};


// == destroy ==

df_row_t::~df_row_t() {
    if (info_start) {
        free(info_start);
    }
}

// == make ==

df_row_t::df_row_t(std::vector<df_named_column_t>& columns) {
    const int COLUMN_LENGTH = columns.size();

    info_start = (row_info_t*)malloc(COLUMN_LENGTH * sizeof(row_info_t));
    info_end = info_start + COLUMN_LENGTH;

    now = 0;
    length = 0;
    
    row_info_t* p = info_start;
    for (df_named_column_t& col : columns) {
        *(p++) = {&col.first, 0, col.second.begin()};
    }
}


constexpr df_row_t::df_row_t(long index) : info_start(NULL), info_end(NULL), now(index), length(0) {}


// == other ==

df_row_t& df_row_t::operator++() {
    now++;
    return *this;
}

df_row_t& df_row_t::operator*() {
    return *this;
}


df_object_t& df_row_t::operator[](const char* name) {
    for (row_info_t* p = info_start; p < info_end; p++) {
        if (p->name->compare(name) == 0) {
            df_debug7("index %d\n", now - p->last_index);
            if (p->last_index != now) {
                p->iter += (now - p->last_index);
                p->last_index = now;
            }

            return *p->iter;
        }
    }
    throw df_exception_out_of_index();
}


bool df_row_t::operator!=(const df_row_t& other) {
    return now != other.now;
}

bool df_row_t::is_end() const {
    return now >= length;
}




// ==== df_const_row_t ====

// == make ==

df_const_row_t::df_const_row_t(const std::vector<df_named_column_t>& columns) : df_row_t((std::vector<df_named_column_t>&)columns) {}


constexpr df_const_row_t::df_const_row_t(long index) : df_row_t(index) {}


// == other ==

const df_object_t& df_const_row_t::operator[](const char* name) {
    return df_row_t::operator[](name);
}






// ==== df_dataframe_t ====

// == destroy ==

df_dataframe_t::~df_dataframe_t() {
    columns.clear();
}



// == create ==

df_dataframe_t::df_dataframe_t() {}


df_dataframe_t::df_dataframe_t(const std::initializer_list<df_named_column_t>& sources) {
    df_debug4("create df_dataframe_t by copy");
    columns.reserve(sources.size() * 2);

    long length;
    for (auto& pair : sources) {
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

df_dataframe_t::df_dataframe_t(df_dataframe_t& src) {
    columns = src.columns;
}

df_dataframe_t& df_dataframe_t::operator=(df_dataframe_t& src) {
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



// == push_back_column ==

df_dataframe_t& df_dataframe_t::add_column(std::string&& name, df_column_t&& column) {
    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(std::move(name), std::move(column));
        return *this;
    }
    result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, df_column_t&& column) {
    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(name, std::move(column));
        return *this;
    }
    result->second = std::move(column);
    return *this;
}

df_dataframe_t& df_dataframe_t::add_column(const std::string& name, const df_column_t& column) {
    df_named_column_t* result = find_column(name.c_str());
    if (result == NULL) {
        columns.emplace_back(name, column);
        return *this;
    }
    result->second = column;
    return *this;
}



// == iterate ==

df_row_t df_dataframe_t::begin() {
    return df_row_t(columns);
}

df_row_t df_dataframe_t::end() {
    return df_row_t(get_row_count());
}


df_const_row_t df_dataframe_t::begin() const {
    return df_const_row_t(columns);
}


df_const_row_t df_dataframe_t::end() const {
    return df_const_row_t(get_row_count());
}





// == print ==

std::ostream& df_dataframe_t::write_stream(std::ostream& os) const {
    const int COLUMN_COUNT = get_column_count();
    const long ROW_COUNT = get_row_count();

    // loader, writer, iterator
    struct write_info_t {
        df_const_mem_iterator_t iter;
        df_value_load_callback_t loader;
        df_value_write_callback_t writer;
    } write_info_list[COLUMN_COUNT];

    // == print titles ==

    os << "| ";
    int index = 0;
    for (auto& named_column : columns) {
        os << named_column.first << " | ";

        write_info_list[index++] = {
            named_column.second.mem_begin(),
            df_value_get_load_callback(named_column.second.get_data_type()),
            df_value_get_write_callback(named_column.second.get_data_type(), DF_TYPE_TEXT)
        };
    }
    os << "\n";

    // == print data ==

    for (int row = 0; row < ROW_COUNT; row++) {
        os << "| ";
        for (int column_index = 0; column_index < COLUMN_COUNT; column_index++) {
            auto& info = write_info_list[column_index];

            if (*info.iter.get_null()) {
                os << "null | ";
                info.iter++;
                continue;
            }

            df_value_t value = info.loader(info.iter.get_data());
            info.iter++;

            std::string s;
            info.writer(value, &s);

            os << s << " | ";
        }
        os << "\n";
    }
    return os;
}



std::ostream& operator<<(std::ostream& os, const df_dataframe_t& df) {
    df.write_stream(os);
    return os;
}