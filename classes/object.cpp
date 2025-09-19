#pragma once

#include "object.hpp"


// == destroy ==

void df_object_t::destroy() {
    if (!is_proxy()) {
        df_value_release(target_value, data_type);
    }
}

df_object_t::~df_object_t() {
    df_debug2("delete %s object at %p", df_type_get_string(data_type), this);
    destroy();
}



// == make ==

void df_object_t::init_as_local() {
    target_null = (bool*)local_buffer;
    target_value = local_buffer + sizeof(bool);
}


constexpr df_object_t::df_object_t() {}


template<typename T>
df_object_t::df_object_t(const T& const_value) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    df_debug2("create %s object at %p", df_type_get_string(DATA_TYPE), this);

    init_as_local();
    data_type = DATA_TYPE;
    *target_null = false;

    if constexpr (DATA_TYPE == DF_TYPE_TEXT) {
        new (target_value) std::string(const_value);
        target_preload = df_value_load_struct(target_value);
    }
    else {
        target_preload = df_value_get_write_callback(DATA_TYPE, DATA_TYPE)(const_value, target_value);
    }
}


df_object_t::df_object_t(const char* const_string) {
    df_debug2("create %s object at %p", df_type_get_string(DF_TYPE_TEXT), this);

    init_as_local();
    data_type = DF_TYPE_TEXT;
    *target_null = false;

    new (target_value) std::string(const_string);
    target_preload = df_value_load_struct(target_value);
}


df_object_t::df_object_t(std::string&& const_string) {
    df_debug2("create %s object at %p", df_type_get_string(DF_TYPE_TEXT), this);

    init_as_local();
    data_type = DF_TYPE_TEXT;
    *target_null = false;

    new (target_value) std::string(std::move(const_string));
    target_preload = df_value_load_struct(target_value);
}


df_object_t::df_object_t(df_null_t) {
    df_debug2("create null object");

    init_as_local();
    data_type = DF_TYPE_UINT8;
    *target_null = true;
}



// == copy ==

void df_object_t::copy(const df_object_t& other) noexcept {
    df_debug2("copy %s object from %p to %p", df_type_get_string(other.data_type), &other, this);

    data_type = other.data_type;
    *target_null = *other.target_null;

    target_preload = df_value_write(other.target_preload, other.data_type, this->target_value, other.data_type);

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
        df_value_release(target_value, data_type);
    }

    copy(other);
    return *this;
}



// == move ==

void df_object_t::move(df_object_t& other) noexcept {
    df_debug2("move %s object from %p to %p", df_type_get_string(other.data_type), &other, this);

    if (other.is_variant()) {
        init_as_local();
        memcpy(local_buffer, other.local_buffer, sizeof(bool) + df_type_get_size(other.data_type));

        other.target_null = NULL;   // take local local_buffer control
    }
    else {
        target_value = (uint8_t*)other.target_value;
        target_null = other.target_null;
    }

    target_preload  = other.target_preload;
    data_type     = other.data_type;

    category_titles = other.category_titles;
}


df_object_t::df_object_t(df_object_t&& other) noexcept {
    move(other);
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

void df_object_t::_set_target(bool* null_ptr, uint8_t* value_ptr, df_value_load_callback_t loader) {
    target_null = null_ptr;
    target_value = value_ptr;
    target_preload = loader(value_ptr);
}


inline void df_object_t::lock() {
    object_lock = true;
}



inline void df_object_t::_change_type(df_type_t dest_type) {
    df_debug2("set %s object to %s", df_type_get_string(data_type), df_type_get_string(dest_type));

    if (is_locked()) {
        throw df_exception_t("couldn't change locked object");
    }

    if (is_proxy()) {
        init_as_local();
    }
    else {
        df_value_release(target_value, data_type);
    }

    data_type = dest_type;
}


template<typename T>
df_object_t& df_object_t::operator=(const T src) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    _change_type(DATA_TYPE);
    *target_null = false;

    new (target_value) T(src);
    target_preload = df_value_get_load_callback(DATA_TYPE)(target_value);
    return *this;
}


df_object_t& df_object_t::operator=(const char* src) {
    _change_type(DF_TYPE_TEXT);
    *target_null = false;

    new (target_value) std::string(src);
    target_preload = df_value_get_load_callback(DF_TYPE_TEXT)(target_value);
    return *this;
}


df_object_t& df_object_t::operator=(df_null_t) {
    _change_type(DF_TYPE_UINT8);
    *target_null = true;
    return *this;
}




template<typename T>
df_object_t& df_object_t::operator<<(const T src) {
    constexpr df_type_t DATA_TYPE = df_type_get_type<T>;
    df_debug2("cast %s to %s object", df_type_get_string(DATA_TYPE), df_type_get_string(data_type));

    df_value_t value = df_value_get_load_callback(DATA_TYPE)(&src);
    target_preload = df_value_write(value, DATA_TYPE, target_value, data_type);
    *target_null = false;

    return *this;
}


df_object_t& df_object_t::operator<<(const char* src) {
    df_debug2("cast TEXT to %s object", df_type_get_string(data_type));

    std::string s = src;
    df_value_t value = &s;

    target_preload = df_value_write(value, DF_TYPE_TEXT, target_value, data_type);
    *target_null = false;
    return *this;
}


df_object_t& df_object_t::operator<<(const df_object_t& src) {
    df_debug2("cast NULL to %s object", df_type_get_string(data_type));

    target_preload = df_value_write(src.target_preload, src.data_type, target_value, data_type);
    *target_null = src.is_null();
    return *this;
}


df_object_t& df_object_t::operator<<(df_null_t) {
    df_debug2("cast null to %s object", df_type_get_string(data_type));

    *target_null = true;
    return *this;
}





// == convert ==

template<typename T>
df_object_t::operator T() const {
    df_debug2("convert %s object to %s", df_type_get_string(data_type), df_type_get_string(df_type_get_type<T>));

    T output;
    df_value_write(target_preload, data_type, &output, df_type_get_type<T>);
    return output;
}



// == check state ==

inline bool df_object_t::is_locked() const {
    return object_lock;
}

inline bool df_object_t::is_proxy() const {
    return (uint8_t*)target_null != local_buffer;
}

inline bool df_object_t::is_variant() const {
    return (uint8_t*)target_null == local_buffer;
}


inline bool df_object_t::is_null() const {
    return *target_null;
}


inline df_type_t df_object_t::get_type() const {
    return data_type;
}



// == print ==

inline std::string df_object_t::to_string() const {
    return is_null() ? "null" : (std::string)*this;
}


inline std::ostream& operator<<(std::ostream& stream, const df_object_t& object) {
    return stream << object.to_string();
}





