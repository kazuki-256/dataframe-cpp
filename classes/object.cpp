#pragma once

#include "object.hpp"


// == destroy ==

void df_object_t::destroy() {
    if (!is_proxy()) {
        df_value_release(target_preload, target_type);
    }
}

df_object_t::~df_object_t() {
    df_debug2("delete %s object at %p", df_type_get_string(target_type), this);
    destroy();
}



// == init ==

void df_object_t::init_as_local() {
    target_null = buffer;
    target_data = target_null + sizeof(bool);
}


constexpr df_object_t::df_object_t() {}


template<typename T>
df_object_t::df_object_t(const T& const_value) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    df_debug2("create %s object at %p", df_type_get_string(DATA_TYPE), this);

    init_as_local();
    target_type = DATA_TYPE;
    *target_null = false;

    if constexpr (DATA_TYPE == DF_TYPE_TEXT) {
        new (target_data) std::string(const_value);
        target_preload = df_value_load_struct(target_data);
    }
    else {
        new (target_data) T(const_value);
        target_preload = df_value_get_load_callback(DATA_TYPE)(target_data);
    }
}

//
df_object_t::df_object_t(const char* const_string) {
    df_debug2("create %s object at %p", df_type_get_string(DF_TYPE_TEXT), this);

    init_as_local();
    target_type = DF_TYPE_TEXT;
    *target_null = false;

    new (target_data) std::string(const_string);
    target_preload = df_value_load_struct(target_data);
}

df_object_t::df_object_t(std::string&& const_string) {
    df_debug2("create %s object at %p", df_type_get_string(DF_TYPE_TEXT), this);

    init_as_local();
    target_type = DF_TYPE_TEXT;
    *target_null = false;

    new (target_data) std::string(std::move(const_string));
    target_preload = df_value_load_struct(target_data);
}



df_object_t::df_object_t(df_null_t) {
    df_debug2("create null object");

    init_as_local();
    target_type = DF_TYPE_UINT8;
    *target_null = true;
}



// == copy ==

void df_object_t::copy(const df_object_t& other) noexcept {
    df_debug2("copy %s object from %p to %p", df_type_get_string(other.target_type), &other, this);

    target_type = other.target_type;
    *target_null = *other.target_null;

    target_preload = df_value_write(other.target_preload, other.target_type, this->target_data, other.target_type);

    category_titles = other.category_titles;
}



df_object_t::df_object_t(const df_object_t& other) noexcept {
    copy(other);
}


df_object_t& df_object_t::operator=(const df_object_t& other) {
    if (this == &other) {
        return *this;
    }
    if (is_locked()) {
        throw df_exception_t("couldn't change locked object");
    }

    if (is_proxy()) {
        init_as_local();
    }
    else {
        df_value_release_mem(target_data, target_type);
    }

    copy(other);
    return *this;
}



// == move ==

void df_object_t::move(df_object_t& other) noexcept {
    df_debug2("move %s object from %p to %p", df_type_get_string(other.target_type), &other, this);

    if (other.is_variant()) {
        init_as_local();
        memcpy(buffer, other.buffer, sizeof(bool) + df_type_get_size(other.target_type));

        other.target_null = NULL;   // take local buffer control
    }
    else {
        target_data = other.target_null;
        target_null = other.target_null;
    }

    target_preload  = other.target_preload;
    target_type     = other.target_type;

    category_titles = other.category_titles;
}


df_object_t::df_object_t(df_object_t&& other) noexcept {
    move(other);
    lock_state  = other.lock_state;
}

df_object_t& df_object_t::operator=(df_object_t&& other) {
    if (this == &other) {
        return *this;
    }
    if (is_locked()) {
        throw df_exception_t("couldn't change locked object");
    }

    destroy();
    move(other);
    return *this;
}



// == set ==

void df_object_t::set_target(uint8_t* target_null, uint8_t* target_data, df_value_load_callback_t loader) {
    this->target_null = target_null;
    this->target_data = target_data;
    this->target_preload = loader(target_data);
}


void df_object_t::lock() {
    lock_state = true;
}



inline void df_object_t::basic_set(df_type_t dest_type) {
    df_debug2("set %s object to %s", df_type_get_string(target_type), df_type_get_string(dest_type));

    if (is_locked()) {
        throw df_exception_t("couldn't change locked object");
    }

    if (is_proxy()) {
        init_as_local();
    }
    else {
        df_value_release_mem(target_data, target_type);
    }

    target_type = dest_type;
}


template<typename T>
df_object_t& df_object_t::operator=(const T src) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    basic_set(DATA_TYPE);
    *target_null = false;

    new (target_data) T(src);
    target_preload = df_value_get_load_callback(DATA_TYPE)(target_data);
    return *this;
}


df_object_t& df_object_t::operator=(const char* src) {
    basic_set(DF_TYPE_TEXT);
    *target_null = false;

    new (target_data) std::string(src);
    target_preload = df_value_get_load_callback(DF_TYPE_TEXT)(target_data);
    return *this;
}


df_object_t& df_object_t::operator=(df_null_t) {
    basic_set(DF_TYPE_UINT8);
    *target_null = true;
    return *this;
}




template<typename T>
df_object_t& df_object_t::operator<<(const T src) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    df_debug2("cast %s to %s object", df_type_get_string(DATA_TYPE), df_type_get_string(target_type));

    df_value_t value = df_value_get_load_callback(DATA_TYPE)(&src);
    target_preload = df_value_write(value, DATA_TYPE, target_data, target_type);
    *target_null = false;

    return *this;
}


df_object_t& df_object_t::operator<<(const char* src) {
    df_debug2("cast TEXT to %s object", df_type_get_string(target_type));

    std::string s = src;
    df_value_t value = &s;

    target_preload = df_value_write(value, DF_TYPE_TEXT, target_data, target_type);
    *target_null = false;
    return *this;
}


df_object_t& df_object_t::operator<<(const df_object_t& src) {
    df_debug2("cast NULL to %s object", df_type_get_string(target_type));

    target_preload = df_value_write(src.target_preload, src.target_type, target_data, target_type);
    *target_null = src.is_null();
    return *this;
}


df_object_t& df_object_t::operator<<(df_null_t) {
    df_debug2("cast null to %s object", df_type_get_string(target_type));

    *target_null = true;
    return *this;
}





// == convert ==

template<typename T>
df_object_t::operator T() const {
    df_debug2("convert %s object to %s", df_type_get_string(target_type), df_type_get_string(df_type_get_type<T>));

    T output;
    df_value_write(target_preload, target_type, &output, df_type_get_type<T>);
    return output;
}



// == check state ==

inline bool df_object_t::is_locked() const {
    return lock_state;
}

inline bool df_object_t::is_proxy() const {
    return target_null != buffer;
}

inline bool df_object_t::is_variant() const {
    return target_null == buffer;
}


inline bool df_object_t::is_null() const {
    return *target_null;
}


inline df_type_t df_object_t::get_type() const {
    return target_type;
}



// == print ==

inline std::string df_object_t::to_string() const {
    return is_null() ? "null" : (std::string)*this;
}


inline std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << object.to_string();
}





