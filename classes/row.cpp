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

df_row_t::df_row_t(source_t* columns, long index, long interval) {
    const int COLUMN_LENGTH = columns->size();

    source = columns;

    matched_start = (matched_info_t*)calloc(COLUMN_LENGTH, sizeof(matched_info_t));
    matched_end = matched_start + COLUMN_LENGTH;

    object_start = (object_info_t*)malloc(COLUMN_LENGTH * sizeof(object_info_t));
    object_end = object_start;

    current = index;
    this->interval = interval;
    ordered = false;
}


constexpr df_row_t::df_row_t(long index) : current(index) {}


// == get ==

int df_row_t::get_column_count() const {
    return source->size();
}

df_row_t& df_row_t::operator*() {
    return *this;
}



inline df_row_t::object_info_t* df_row_t::_add_object_cashe(std::pair<std::string, df_column_t*>& pair) const {
    object_info_t* ptr = object_end++;
    ptr->name = &pair.first;
    ptr->column = pair.second;

    ptr->object.target_type = pair.second->data_type;
    ptr->object.lock_state = true;
    return ptr;
}


inline df_object_t& df_row_t::_at(const char* name, df_row_t::object_info_t*& info) const {
    // == from object_info list ==

    // == from created object list ==
    for (info = object_start; info < object_end; info++) {
        if (info->name->compare(name) == 0) {
            goto label_get_data;
        }
    }

    // == match from source ==
    
    for (auto& pair : *source) {
        // filter not matching
        if (pair.first.compare(name) != 0) {
            continue;
        }

        // make info
        info = _add_object_cashe(pair);
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
df_object_t& df_row_t::at(const char* name) const {
    object_info_t* info;
    return _at(name, info);
}


df_object_t& df_row_t::operator[](const char* name) const {
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
    df_object_t& result = _at(name, object_info);

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


bool df_row_t::operator!=(const df_row_t& other) const {
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


df_row_t::iterator_t df_row_t::begin() const {
    // == fill object_info_t ==
    if (ordered == false) {
        ordered = true;

        object_end = object_start;
        for (auto& pair : *source) {
            _add_object_cashe(pair);
        }

        memset(matched_start, 0, (long)matched_end - (long)matched_start);
        matched_end = matched_start;
    }

    // == make iterator ==
    return iterator_t(object_start, current);
}

df_row_t::iterator_t df_row_t::end() const {
    return iterator_t(object_end, 0);
}
    


// == write ==

std::ostream& df_row_t::write_stream(std::ostream& os) const {
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
    return row.write_stream(os);
}






// ==== df_const_row_t ====

// == make ==

df_const_row_t::df_const_row_t(
    const_source_t* columns,
    long index, long interval
) : df_row_t((source_t*)columns, index, interval) {}


constexpr df_const_row_t::df_const_row_t(long index) : df_row_t(index) {}


// == other ==

const df_object_t& df_const_row_t::operator[](const char* name) const {
    return df_row_t::operator[](name);
}



std::ostream& operator<<(std::ostream& os, const df_const_row_t& row) {
    return row.write_stream(os);
}



