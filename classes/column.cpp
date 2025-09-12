#pragma once

#include "../config.hpp"
#include "column.hpp"
#include "object.cpp"
#include "object_iterator.cpp"




// ==== df_column_t ====

df_memory_iterator_t df_column_t::memory_begin() {
    return df_memory_iterator_t(this, 0);
}

df_memory_iterator_t df_column_t::memory_end() {
    return df_memory_iterator_t(NULL, length);
}



df_const_memory_iterator_t df_column_t::memory_begin() const {
    return df_const_memory_iterator_t(this, 0);
}

df_const_memory_iterator_t df_column_t::memory_end() const {
    return df_const_memory_iterator_t(NULL, length);
}



df_object_iterator_t df_column_t::begin() {
    return df_object_iterator_t(this, 0);
}

df_object_iterator_t df_column_t::end() {
    return df_object_iterator_t(length);
}



df_const_object_iterator_t df_column_t::begin() const {
    return df_const_object_iterator_t(this, 0);
}

df_const_object_iterator_t df_column_t::end() const {
    return df_const_object_iterator_t(length);
}





// == destroy ==

inline void df_column_t::destroy() noexcept {
    df_debug3("destroy %s[%ld] column", df_type_get_string(data_type), get_length());
    if (nulls != NULL) {
        free(nulls);
    }
    if (values != NULL) {
        free(values);
    }
}


df_column_t::~df_column_t() {
    destroy();
}


// == make ==

inline void df_column_t::basic_init(df_type_t _data_type, long _length, long _capacity) {
    data_type = _data_type;
    size_per_data = df_type_get_size(_data_type);

    type_initer = df_value_get_init_callback(_data_type);
    type_loader = df_value_get_load_callback(_data_type);

    length = _length;
    capacity = (_capacity * 2 + 4096) & ~4095;

    nulls  = (bool*)malloc(capacity);
    values = (uint8_t*)malloc(capacity * size_per_data);

    if (nulls == NULL || values == NULL) {
        throw df_exception_not_enough_memory();
    }
}


template<typename T>
inline void df_column_t::typed_init_no_init(df_type_t data_type, const std::initializer_list<T>& sources) {
    df_debug3("typed-make %s[0] at %p", df_type_get_string(data_type), this);
    basic_init(data_type, sources.size(), sources.size());

    df_memory_iterator_t iter = memory_begin();

    for (T val : sources) {
        *iter.get_null() = false;
        *((T*)iter.get_value()) = val;
        iter++;
    }
}




df_column_t::df_column_t() {}


df_column_t::df_column_t(df_type_t data_type, long start_capacity) {
    df_debug3("make1 %s[0] at %p", df_type_get_string(data_type), this);
    basic_init(data_type, 0, start_capacity);
}


df_column_t::df_column_t(const std::initializer_list<df_object_t>& objects) {
    df_debug3("make2 %s[%ld] at %p", df_type_get_string(data_type), objects.size(), this);
    basic_init(objects.begin()->target_type, 0, objects.size());
    basic_extend_values(objects);
}




// == copy ==

df_column_t::df_column_t(const df_column_t& src) noexcept {
    df_debug3("copy1 %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.get_length(), &src, this);
    basic_init(src.data_type, 0, src.length);
    basic_extend_column(src);
}


df_column_t& df_column_t::df_column_t::operator=(const df_column_t& src) noexcept {
    df_debug3("copy2 %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.get_length(), &src, this);
    if (this == &src) {
        return *this;
    }
    
    destroy();
    basic_init(src.data_type, 0, src.length);
    basic_extend_column(src);
    return *this;
}



// == move ==

inline void df_column_t::move(df_column_t& src) noexcept {
    data_type = src.data_type;
    size_per_data = src.size_per_data;

    type_initer = src.type_initer;
    type_loader = src.type_loader;

    values = src.values;
    src.values = NULL;
    nulls = src.nulls;
    src.nulls = NULL;

    length = src.length;
    capacity = src.capacity;
}



df_column_t::df_column_t(df_column_t&& src) noexcept {
    df_debug3("move1 %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.get_length(), &src, this);
    move(src);
}


df_column_t& df_column_t::operator=(df_column_t&& src) noexcept {
    df_debug3("move2 %s[%ld] column from %p to %p", df_type_get_string(src.data_type), src.get_length(), &src, this);
    if (this == &src) {
        return *this;
    }

    destroy();
    move(src);
    return *this;
}




// == get ==

inline df_type_t df_column_t::get_data_type() const {
    return data_type;
}


inline long df_column_t::get_length() const {
    return values ? length : 0;
}




df_object_t df_column_t::operator[](long index) {
    index = df_calculate_index(index, length);
    
    df_object_t object;
    object.target_type = data_type;
    object.set_target(nulls + index, values + index * size_per_data, type_loader);
    return object;
}


const df_object_t df_column_t::operator[](long index) const {
    index = df_calculate_index(index, length);
    
    df_object_t object;
    object.target_type = data_type;
    object.set_target(nulls + index, values + index * size_per_data, type_loader);
    return object;
}




// == set ==

int df_column_t::reserve(const long n) {
    long new_length = length + n;

    if (new_length < capacity) {
        return 0;
    }

    long new_capacity = (new_length * 2 + 4096) & ~4095;

    void* mem = realloc(nulls, new_capacity);
    if (mem == NULL) {
        return -1;
    }
    nulls = (bool*)mem;

    mem = realloc(values, new_capacity * size_per_data);
    if (mem == NULL) {
        return -1;
    }
    values = (uint8_t*)mem;

    return 1;
}




// == append / extend / merged ==


// basic extend for initial_list<> or vector<>
template<typename T> inline df_column_t& df_column_t::basic_extend_values(T& objects) {
    df_value_init_callback_t initer = df_value_get_init_callback(data_type);
    df_value_write_callback_t writer = df_value_get_write_callback(data_type, data_type);

    df_memory_iterator_t dest = memory_begin();

    length += objects.size();

    for (const df_object_t& object : objects) {
        // == init ==
        if (initer) {
            initer(dest.get_value());
        }

        // == skip copy if null ==
        if (object.is_null()) {
            *dest.get_null() = true;
            dest++;
            continue;
        }

        // == same type faster copy ==
        if (object.target_type == data_type) {
            writer(object.target_preload, dest.get_value());
            dest++;
            continue;
        }

        // == different type copy ==
        df_value_write(
            object.target_preload, object.target_type,
            dest.get_value(), data_type
        );
        df_debug7("value: %lf", *(double*)dest.get_value());
        dest++;
    }

    return *this;
}



inline void df_column_t::basic_extend_column(const df_column_t& other) {
    df_value_write_callback_t writer = df_value_get_write_callback(other.data_type, data_type);

    df_const_memory_iterator_t src = other.memory_begin();
    df_memory_iterator_t dest = memory_begin() += length;
    length += other.length;

    for (; src != other.memory_end(); src++, dest++) {
        // == init ==
        if (type_initer) {
            type_initer(dest.get_value());
        }

        // == get null  ==
        bool is_null = *dest.get_null() = *src.get_null();

        // == copy if has value ==
        if (!is_null) {
            df_value_t value = other.type_loader(src.get_value());
            writer(value, dest.get_value());
        }
    }
}






df_column_t& df_column_t::append(const df_object_t& object) {
    if (reserve(1) < 0) {
        throw df_exception_not_enough_memory();
    }

    uint8_t* value_ptr = values + length * size_per_data;
    bool*    null_ptr  = nulls + length;
    length++;

    df_value_init(value_ptr, data_type);

    bool is_null = *null_ptr = object.is_null();

    if (!is_null) {
        df_value_write(object.target_preload, object.target_type, value_ptr, data_type);
    }
    
    return *this;
}




df_column_t& df_column_t::extend(const std::initializer_list<df_object_t>& objects) {
    if (reserve(objects.size()) < 0) {
        throw df_exception_not_enough_memory();
    }
    return basic_extend_values(objects);
}


df_column_t& df_column_t::extend(const std::vector<df_object_t>& objects) {
    if (reserve(objects.size()) < 0) {
        throw df_exception_not_enough_memory();
    }
    return basic_extend_values(objects);
}


df_column_t& df_column_t::extend(const df_column_t& other) {
    if (reserve(other.length) < 0) {
        throw df_exception_not_enough_memory();
    }
    basic_extend_column(other);
    return *this;
}




df_column_t df_column_t::merged(const std::initializer_list<df_object_t>& objects) const {
    df_column_t output = *this;
    return output.extend(objects);
}


df_column_t df_column_t::merged(const std::vector<df_object_t>& objects) const {
    df_column_t output = *this;
    return output.extend(objects);
}


df_column_t df_column_t::merged(const df_column_t& other) const {
    df_column_t output = *this;
    return output.extend(other);
}



// == print ==

std::ostream& df_column_t::write_stream(std::ostream& os, const char* column_name) const {
    df_value_write_callback_t writer = df_value_get_write_callback(data_type, DF_TYPE_TEXT);

    df_const_memory_iterator_t iter = memory_begin();
    df_value_t value;
    std::string buf;

    os << "| " << column_name << " |\n";
    for (; iter != memory_end(); iter++) {
        if (*iter.get_null()) {
            os << "| null |\n";
            continue;
        }

        value = type_loader(iter.get_value());
        writer(value, &buf);

        os << "| " << buf << " |\n";
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const df_column_t& column) {
    return column.write_stream(os, "VALUES");
}




// ==== named_column_t ====

std::ostream& operator<<(std::ostream& os, const df_named_column_t& named_column) {
    return named_column.second.write_stream(os, named_column.first.c_str());
}



// ==== typed_column_t ====

df_column_uint8_t::df_column_uint8_t(const std::initializer_list<uint8_t>& sources) {
    typed_init_no_init(DF_TYPE_UINT8, sources);
}

df_column_int16_t::df_column_int16_t(const std::initializer_list<short>& sources) {
    typed_init_no_init(DF_TYPE_INT16, sources);
}

df_column_int32_t::df_column_int32_t(const std::initializer_list<int>& sources) {
    typed_init_no_init(DF_TYPE_INT32, sources);
}

df_column_int64_t::df_column_int64_t(const std::initializer_list<long>& sources) {
    typed_init_no_init(DF_TYPE_INT64, sources);
}



df_column_float32_t::df_column_float32_t(const std::initializer_list<float>& sources) {
    typed_init_no_init(DF_TYPE_FLOAT32, sources);
}

df_column_float64_t::df_column_float64_t(const std::initializer_list<double>& sources) {
    typed_init_no_init(DF_TYPE_FLOAT64, sources);
}



df_column_text_t::df_column_text_t(const std::initializer_list<const char*>& sources) {
    df_debug3("typed-make %s[0] at %p", df_type_get_string(DF_TYPE_TEXT), this);
    basic_init(DF_TYPE_TEXT, sources.size(), sources.size());

    df_memory_iterator_t iter = memory_begin();

    for (const char* val : sources) {
        *iter.get_null() = false;
        new (iter.get_value()) std::string(val);
        iter++;
    }
}

df_column_text_t::df_column_text_t(const std::initializer_list<std::string>& sources) {
    df_debug3("typed-make %s[0] at %p", df_type_get_string(DF_TYPE_TEXT), this);
    basic_init(DF_TYPE_TEXT, sources.size(), sources.size());

    df_memory_iterator_t iter = memory_begin();

    for (const std::string& val : sources) {
        *iter.get_null() = false;
        new (iter.get_value()) std::string(val);
        iter++;
    }
}



df_column_date_t::df_column_date_t(const std::initializer_list<df_date_t>& sources) {
    typed_init_no_init(DF_TYPE_DATE, sources);
}

df_column_time_t::df_column_time_t(const std::initializer_list<const char*>& sources) {
    df_debug3("typed-make %s[0] at %p", df_type_get_string(DF_TYPE_TIME), this);
    basic_init(DF_TYPE_TIME, sources.size(), sources.size());

    df_memory_iterator_t iter = memory_begin();

    for (const char* val : sources) {
        *iter.get_null() = false;
        new (iter.get_value()) df_date_t(val, DF_TIME_FORMAT);
        iter++;
    }
}

df_column_datetime_t::df_column_datetime_t(const std::initializer_list<df_date_t>& sources) {
    typed_init_no_init(DF_TYPE_INT64, sources);
}


