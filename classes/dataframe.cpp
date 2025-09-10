#pragma once

#include "dataframe.hpp"
#include "column.cpp"


// ==== df_row_t ====

// == structures ==

struct df_row_t::object_info_t {
    const std::string* name;
    long last_index;
    df_column_t::object_iterator_t iter;
};


struct df_row_t::matched_info_t {
    const void* address;
    object_info_t* object_info;
};


// == destroy ==

df_row_t::~df_row_t() {
    if (matched_start) {
        free(matched_start);
    }
    if (object_start) {
        free(object_start);
    }
}


// == make ==

df_row_t::df_row_t(std::vector<df_named_column_t>* columns, df_date_t* check_update, long index, long interval) {
    const int COLUMN_LENGTH = columns->size();

    unextended_columns = columns;

    matched_start = (matched_info_t*)calloc(COLUMN_LENGTH * 2 + 1, sizeof(matched_info_t));
    matched_end = matched_start + COLUMN_LENGTH * 2;

    object_start = (object_info_t*)malloc(COLUMN_LENGTH * sizeof(object_info_t));
    object_end = object_start;

    current = index;
    this->interval = interval;

    this->last_update = *check_update;
    this->check_update = check_update;
}


constexpr df_row_t::df_row_t(long index) : current(index) {}


// == other ==

df_row_t& df_row_t::operator++() {
    current += interval;
    return *this;
}

df_row_t& df_row_t::operator*() {
    return *this;
}



df_object_t& df_row_t::operator[](const char* name) {
    // == if column update, remake column_info and match_info ==

    if (*check_update != last_update) {
        // reset matched_info_t
        memset(matched_start, 0, matched_end - matched_start);

        // reset object_info_t
        object_start = (object_info_t*)realloc(object_start, unextended_columns->size() * sizeof(object_info_t));
        object_end = object_start + unextended_columns->size();
    }
    
    // == from matched cashe ==

    matched_info_t* match_info;
    object_info_t* object_info;

    for (match_info = matched_start; match_info->address != NULL; match_info++) {
        // execute only matched
        if (match_info->address == name) {
            object_info = match_info->object_info;
            goto label_get_data;
        }
    }

    // == from created object list ==
    for (object_info = object_start; object_info < object_end; object_info++) {
        if (object_info->name->compare(name) == 0) {
            goto label_make_matched;
        }
    }

    // == match from unextended_columns ==
    
    for (df_named_column_t& column : *unextended_columns) {
        // filter not matching
        if (column.first.compare(name) != 0) {
            continue;
        }

        // make object_info
        *object_info = {&column.first, 0, column.second.begin()};
        object_end++;

        goto label_make_matched;
    }

    throw df_exception_out_of_index();

label_make_matched:
    if (match_info >= matched_end) {
        int capacity = matched_end - matched_start;
        matched_info_t* buffer = (matched_info_t*)realloc(matched_start, (capacity * 2 + 1) * sizeof(matched_info_t));

        if (buffer != NULL) {
            matched_start = buffer;
            matched_end = buffer + capacity * 2;
            match_info = buffer + capacity;
        }
    }

    if (match_info < matched_end) {
        match_info->address = name;
        match_info->object_info = object_info;
    }

label_get_data:
    if (object_info->last_index != current) {
        object_info->iter += (current - object_info->last_index);
        object_info->last_index = current;
    }

    return *object_info->iter;
}


bool df_row_t::operator!=(const df_row_t& other) {
    return current != other.current;
}








// ==== df_const_row_t ====

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




class df_dataframe_t::range_rows_t {
    friend class df_dataframe_t;

    df_dataframe_t* df;
    long start_index;
    long end_index;
    long interval;
    
    range_rows_t(df_dataframe_t* df, long start_index, long end_index, long interval) {
        this->df = df;
        this->start_index = start_index;
        this->end_index = end_index;
        this->interval = interval;
    }

public:

    df_row_t begin() {
        return df_row_t(&df->columns, &df->last_column_update, start_index, interval);
    }

    df_row_t end() {
        return df_row_t(end_index);
    }


    df_const_row_t begin() const {
        return df_const_row_t(&df->columns, &df->last_column_update, start_index, interval);
    }


    df_const_row_t end() const {
        return df_const_row_t(end_index);
    }
};


df_dataframe_t::range_rows_t df_dataframe_t::range_rows(long start = 0, long end = -1, long interval = 1) {
    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    const long LENGTH = get_row_count();

    start = df_calculate_index(start, LENGTH);
    end = df_calculate_index(end + 1, LENGTH);

    long range = end - start;
    if (range * interval < 0) {
        throw df_exception_endless_range();
    }
    end = end - range % interval;

    return range_rows_t(this, start, end, interval);
}






// == make ==

df_const_row_t::df_const_row_t(
    const std::vector<df_named_column_t>* columns, const df_date_t* check_update,
    long index, long interval
) : df_row_t((std::vector<df_named_column_t>*)columns, (df_date_t*)check_update, index, interval) {}


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




// == print ==

std::ostream& df_dataframe_t::write_stream(std::ostream& os) const {
    const int COLUMN_COUNT = get_column_count();
    const long ROW_COUNT = get_row_count();

    // loader, writer, iterator
    struct write_info_t {
        df_column_t::const_memory_iterator_t iter = NULL;
        df_value_load_callback_t loader;
        df_value_write_callback_t writer;
    } write_info_list[COLUMN_COUNT];

    // == print titles ==

    os << "| ";
    int index = 0;
    for (auto& named_column : columns) {
        os << named_column.first << " | ";

        write_info_list[index++] = {
            named_column.second.memory_begin(),
            named_column.second.type_loader,
            df_value_get_write_callback(named_column.second.get_data_type(), DF_TYPE_TEXT)
        };
    }
    os << "\n";

    // == print data ==

    write_info_t* END = write_info_list + COLUMN_COUNT;
    std::string s(32, '\0');
    
    for (int row = 0; row < ROW_COUNT; row++) {
        os << "| ";
        for (write_info_t* info = write_info_list; info < END; info->iter++, info++) {
            if (*info->iter.get_null()) {
                os << "null | ";
                continue;
            }

            // const uint8_t* ptr = info->iter.get_value();
            df_value_t value = info->loader(info->iter.get_value());
            info->writer(value, &s);

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