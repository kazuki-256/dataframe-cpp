#pragma once

#include "object_iterator.hpp"
#include "column.hpp"


// ==== df_memory_iterator_t ====

// == make ==

constexpr df_memory_iterator_t::df_memory_iterator_t(df_column_t* column, long index, long interval)
: source(column), index(index), interval(interval) {}


// == add ==

inline df_memory_iterator_t& df_memory_iterator_t::operator++() {
    index++;
    return *this;
}

inline df_memory_iterator_t& df_memory_iterator_t::operator++(int) {
    index++;
    return *this;
}

inline df_memory_iterator_t& df_memory_iterator_t::operator+=(long offset) {
    index += offset;
    return *this;
}


// == compare ==

inline bool df_memory_iterator_t::operator!=(const df_memory_iterator_t& other) const {
    return index != other.index;
}


// == get ==

inline bool* df_memory_iterator_t::get_null() const {
    return source->nulls + index;
}

inline uint8_t* df_memory_iterator_t::get_value() const {
    return source->values + index * source->size_per_data;
}




// ==== df_const_memory_iterator_t ====

// == make ==

constexpr df_const_memory_iterator_t::df_const_memory_iterator_t(const df_column_t* column, long index, long interval)
: df_memory_iterator_t((df_column_t*)column, index, interval) {}


// == get ==

inline const bool* df_const_memory_iterator_t::get_null() const {
    return source->nulls + index;
}

inline const uint8_t* df_const_memory_iterator_t::get_value() const {
    return source->values + index * source->size_per_data;
}



// ==== df_object_iterator_t ====

// == make ==

constexpr df_object_iterator_t::df_object_iterator_t(long index)
: df_memory_iterator_t(NULL, index, 1) {}


inline df_object_iterator_t::df_object_iterator_t(df_column_t* column, long index, long interval) : df_memory_iterator_t(column, index, interval) {
    proxy.target_type = column->data_type;
    proxy.lock_state = true;
}

// == get ==

inline df_object_t& df_object_iterator_t::operator*() {
    proxy.set_target(get_null(), get_value(), source->type_loader);
    return proxy;
}



// ==== df_const_object_iterator_t ====

// == make ==

constexpr df_const_object_iterator_t::df_const_object_iterator_t(long index)
: df_object_iterator_t(index) {}


inline df_const_object_iterator_t::df_const_object_iterator_t(const df_column_t* column, long index, long interval)
: df_object_iterator_t((df_column_t*)column, index, interval) {}


// == get ==

inline const df_object_t& df_const_object_iterator_t::operator*() {
    proxy.set_target(get_null(), get_value(), source->type_loader);
    return proxy;
}



