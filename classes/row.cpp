#pragma once

#include "row.hpp"
#include "column.cpp"


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

df_row_t::df_row_t(std::vector<df_named_column_t>* columns, long index, long interval) {
    const int COLUMN_LENGTH = columns->size();

    source = columns;

    matched_start = (matched_info_t*)calloc(COLUMN_LENGTH, sizeof(matched_info_t));
    matched_end = matched_start + COLUMN_LENGTH;

    object_start = (object_info_t*)malloc(COLUMN_LENGTH * sizeof(object_info_t));
    object_end = object_start;

    current = index;
    this->interval = interval;
}


constexpr df_row_t::df_row_t(long index) : current(index) {}


// == get ==

int df_row_t::get_length() const {
    return source->size();
}

df_row_t& df_row_t::operator*() {
    return *this;
}



inline df_object_t& df_row_t::basic_at(const char* name, df_row_t::object_info_t*& info) {
    // == from object_info list ==

    // == from created object list ==
    for (info = object_start; info < object_end; info++) {
        if (info->name->compare(name) == 0) {
            goto label_get_data;
        }
    }

    // == match from source ==
    
    for (df_named_column_t& column : *source) {
        // filter not matching
        if (column.first.compare(name) != 0) {
            continue;
        }

        // make info
        info->name = &column.first;
        info->column = (df_column_t*)&column.second;

        info->object.target_type = column.second.data_type;
        info->object.lock_state = true;
        object_end++;

        goto label_get_data;
    }

    throw df_exception_out_of_index();

label_get_data:
    info->object.set_target(
        info->column->nulls + current, info->column->values + current * info->column->size_per_data,
        info->column->type_loader
    );
    return info->object;
}


// operator[]() without pointer compare
df_object_t& df_row_t::at(const char* name) {
    object_info_t* info;
    return basic_at(name, info);
}


df_object_t& df_row_t::operator[](const char* name) {
    // == from matched cashe ==

    matched_info_t* match_info;
    object_info_t* object_info;

    for (match_info = matched_start; match_info->address != NULL; match_info++) {
        // execute only matched
        if (match_info->address != name) {
            continue;
        }

        match_info->object_info->object.set_target(
            match_info->object_info->column->nulls + current, match_info->object_info->column->values + current * match_info->object_info->column->size_per_data,
            match_info->object_info->column->type_loader
        );
        return match_info->object_info->object;
    }

    // == from object_info or source ==
    df_object_t& result = basic_at(name, object_info);

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
    return result;
}


// == other ==

df_row_t& df_row_t::operator++() {
    current += interval;
    return *this;
}


bool df_row_t::operator!=(const df_row_t& other) {
    return current != other.current;
}


// == iterator ==

class df_row_t::iterator_t {
    friend class df_row_t;
protected:
    object_info_t* info;
    long current;

    inline iterator_t(object_info_t* object_info, long index) {
        info = object_info;
        current = index;
    }
public:
    inline const df_object_t& operator*() {
        info->object.set_target(
            info->column->nulls + current, info->column->values + current * info->column->size_per_data,
            info->column->type_loader
        );
        return info->object;
    }

    inline iterator_t& operator++() {
        info++;
        return *this;
    }

    inline iterator_t& operator++(int) {
        info++;
        return *this;
    }

    inline iterator_t& operator+=(int offset) {
        info += offset;
        return *this;
    }

    inline bool operator!=(const iterator_t& other) const {
        return info != other.info;
    }
};


df_row_t::iterator_t df_row_t::begin() {
    // == fill object_info_t ==
    if (object_start + source->size() != object_end) {
        object_info_t* ptr = object_start;

        for (df_named_column_t& column : *source) {
            ptr->name = &column.first;
            ptr->column = (df_column_t*)&column.second;

            ptr->object.target_type = column.second.data_type;
            ptr->object.lock_state = true;
            ptr++;
        }
        object_end = ptr;
        df_debug7("test %p %p", object_start, object_end);

        matched_end = matched_start;
        matched_start->address = NULL;
    }

    // == make iterator ==
    return iterator_t(object_start, current);
}

df_row_t::iterator_t df_row_t::end() {
    return iterator_t(object_end, 0);
}
    


// == write ==

std::ostream& df_row_t::write_stream(std::ostream& os) {
    df_debug7("");
    iterator_t iter = begin();

    os << "| ";
    for (object_info_t* ptr = object_start; ptr < object_end; ptr++) {
        os << *ptr->name << " | ";
    }

    os << "\n| ";
    
    for (; iter != end(); iter++) {
        os << (*iter).to_string() << " | ";
    }
    os << "\n";

    return os;
}


std::ostream& operator<<(std::ostream& os, const df_row_t& row) {
    df_debug7("");
    return ((df_row_t&)row).write_stream(os);
}






// ==== df_const_row_t ====

// == make ==

df_const_row_t::df_const_row_t(
    const std::vector<df_named_column_t>* columns,
    long index, long interval
) : df_row_t((std::vector<df_named_column_t>*)columns, index, interval) {}


constexpr df_const_row_t::df_const_row_t(long index) : df_row_t(index) {}


// == other ==

const df_object_t& df_const_row_t::operator[](const char* name) {
    return df_row_t::operator[](name);
}



std::ostream& operator<<(std::ostream& os, const df_const_row_t& row) {
    return ((df_row_t&)row).write_stream(os);
}



