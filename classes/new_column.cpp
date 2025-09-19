#pragma once

#include "new_column.hpp"
#include "object.cpp"


// == destroy ==

void df_column_t::_destroy() noexcept {
    if (_column == NULL) {
        return;
    }
    if (--_column->use_count > 0) {     // if not the last holder, no destroy memory
        return;
    }

    free(_column->nulls);
    free(_column->values);
    _column->label.~basic_string();
    _column->mutexs.~list();
}

df_column_t::~df_column_t() {
    _destroy();
}



// == init ==

void df_column_t::_init(df_type_t data_type, long start_capacity) {
    _column = (_column_t*)malloc(sizeof(_column_t));
    if (_column == NULL) {
        throw df_exception_not_enough_memory();
    }

    _column->data_type = data_type;
    _column->size_per_data = df_type_get_size(data_type);
    _column->type_initer = df_value_get_init_callback(data_type);
    _column->type_loader = df_value_get_load_callback(data_type);


    _column->length = 0;
    _column->capacity = (start_capacity * 2 + 4096) & ~4095;

    _column->nulls  = (bool*)malloc(_column->capacity);
    _column->values = (uint8_t*)malloc(_column->capacity * _column->size_per_data);

    if (_column->nulls == NULL || _column->values == NULL) {
        throw df_exception_not_enough_memory();
    }
    
    new (&_column->label) std::string();

    new (&_column->mutexs) std::list<mutex_t>();
    _column->use_count = 1;

    _column->can_read = true;
    _column->can_write = true;
}





df_column_t::df_column_t(df_type_t data_type, long start_capacity) {
    _init(data_type, start_capacity);
}


df_column_t::df_column_t(const std::initializer_list<df_object_t>& objects) {
    _init(objects.begin()->target_type, objects.size());
    extend(objects);
}



// == copy ==

void df_column_t::_copy(const df_column_t& other) noexcept {
    _init(other.get_data_type(), other.get_length());
    extend(other);
}


df_column_t::df_column_t(const df_column_t& other) noexcept {
    _copy(other);
}

df_column_t& df_column_t::operator=(const df_column_t& other) noexcept {
    _destroy();
    _copy(other);
    return *this;
}




// == move ==

void df_column_t::_move(const df_column_t& other) noexcept {
    _column = other._column;
    _column->use_count++;
}


df_column_t::df_column_t(const df_column_t& other) noexcept {
    _move(other);
}

df_column_t& df_column_t::operator=(const df_column_t& other) noexcept {
    _destroy();
    _move(other);
    return *this;
}



// == iterator ==

/*
df_memory_iterator_t df_column_t::memory_begin() {
    
}

df_memory_iterator_t df_column_t::memory_end() {
    
}

df_object_iterator_t df_column_t::begin() {
    
}

df_object_iterator_t df_column_t::end() {
    
}



df_const_memory_iterator_t df_column_t::memory_begin() const {
    
}

df_const_memory_iterator_t df_column_t::memory_end() const {
    
}

df_const_object_iterator_t df_column_t::begin() const {
    
}

df_const_object_iterator_t df_column_t::end() const {
    
}

*/



// == range ==

/*
df_object_range_t df_column_t::range(long start, long end, long interval) {
    
}



df_object_range_t df_column_t::range(long start, long end, long interval) const {
    
}
*/



// == get ==

inline df_type_t df_column_t::get_data_type() const {
    return _column->data_type;
}

inline long df_column_t::get_length() const {
    return _column->length;
}

inline const std::string& df_column_t::get_label() const {
    return _column->label;
}




