#pragma once

#include "../config.hpp"
#include "column_new.hpp"
#include "object.cpp"




// ==== df_column_t ====

// == destroy ==

inline void df_column_t::destroy() noexcept {
    df_debug3("destroy %s[%ld] column", df_type_get_string(data_type), length);

    df_value_release_callback_t release = df_value_get_release_callback(data_type);
    if (release) {
        memory_iterator_t iter = begin();

        for (; iter != end(); iter++) {
            release(iter.get_value());
        }
    }


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

inline void df_column_t::basic_init(df_type_t _data_type, long _capacity) {
    data_type = _data_type;
    size_per_data = df_type_get_size(_data_type);

    length = 0;
    capacity = (_capacity * 2 + 4096) & ~4095;

    nulls  = (bool*)malloc(capacity);
    values = (uint8_t*)malloc(capacity * size_per_data);

    if (nulls == NULL || values == NULL) {
        throw df_exception_not_enough_memory();
    }
}




df_column_t::df_column_t(df_type_t data_type, long start_capacity) {
    basic_init(data_type, start_capacity);
}


df_column_t::df_column_t(const std::initializer_list<df_object_t>& objects) {
    basic_init(objects.begin()->target_type, objects.size());
    
    basic_extend(objects,
        df_value_get_init_callback(data_type), df_value_get_write_callback(data_type, data_type)
    );
}




// == copy ==

df_column_t::df_column_t(const df_column_t& src) {
    basic_init(data_type, src.length);

    basic_extend_values(
        src.values, src.nulls, src.nulls + src.length,
        src.data_type, src.size_per_data,
        df_value_get_init_callback(data_type), df_value_get_load_callback(src.data_type),
        df_value_get_write_callback(src.data_type, data_type)
    );
}



// == move ==

inline void df_column_t::move(df_column_t& src) noexcept {
    data_type = src.data_type;
    size_per_data = src.size_per_data;

    values = src.values;
    src.values = NULL;
    nulls = src.nulls;
    src.nulls = NULL;

    length = src.length;
    capacity = src.capacity;
}



df_column_t::df_column_t(df_column_t&& src) noexcept {
    move(src);
}


df_column_t& df_column_t::operator=(df_column_t&& src) noexcept {
    destroy();
    move(src);
    return *this;
}




// == get ==

inline df_type_t df_column_t::get_data_type() const {
    return data_type;
}


inline long df_column_t::get_length() const {
    return length;
}




df_object_t df_column_t::operator[](long index) {

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

    return 0;
}




// == append / extend / merged ==


// basic extend for initial_list<> or vector<>
template<typename T> inline df_column_t& df_column_t::basic_extend(
        T& objects, df_value_init_callback_t initer, df_value_write_callback_t writer
){
    uint8_t* value_ptr = values + length * size_per_data;
    bool*    null_ptr  = nulls + length;

    length += objects.size();

    for (const df_object_t& object : objects) {
        // == init ==
        if (initer) {
            initer(value_ptr);
        }

        // == skip copy if null ==
        if (object.is_null()) {
            *null_ptr++ = true;
            value_ptr += size_per_data;
            continue;
        }
        *null_ptr++ = false;

        // == same type faster copy ==
        if (object.target_type == data_type) {
            writer(object.target_preload, value_ptr);
            value_ptr += size_per_data;
            continue;
        }

        // == different type copy ==
        df_value_write(
            object.target_preload, object.target_type,
            value_ptr, data_type
        );
        value_ptr += size_per_data;
    }

    return *this;
}



void df_column_t::basic_extend_values(
    uint8_t* src_value_ptr, bool* src_null_ptr, bool* src_null_end,
    df_type_t src_type, int src_size_per_data,
    df_value_init_callback_t initer, df_value_load_callback_t loader, df_value_write_callback_t writer
){
    uint8_t* value_ptr = values + length * size_per_data;
    bool*    null_ptr  = nulls + length;

    length = src_null_end - src_null_ptr;

    while (src_null_ptr < src_null_end) {
        // == init ==
        if (initer) {
            initer(value_ptr);
        }

        // == get null  ==
        bool is_null = *null_ptr = *src_null_ptr;

        // == copy if has value ==
        if (!is_null) {
            df_value_t value = loader(src_value_ptr);
            writer(value, value_ptr);
        }
        
        // == increse ==
        null_ptr++;
        value_ptr += size_per_data;
        src_null_ptr++;
        src_value_ptr += src_size_per_data;
    }
}






df_column_t& df_column_t::append(const df_object_t& object) {
    if (reserve(1)) {
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
    if (reserve(objects.size())) {
        throw df_exception_not_enough_memory();
    }

    return basic_extend(objects,
        df_value_get_init_callback(data_type), df_value_get_write_callback(data_type, data_type)
    );
}


df_column_t& df_column_t::extend(const std::vector<df_object_t>& objects) {
    if (reserve(objects.size())) {
        throw df_exception_not_enough_memory();
    }

    return basic_extend(objects,
        df_value_get_init_callback(data_type), df_value_get_write_callback(data_type, data_type)
    );
}


df_column_t& df_column_t::extend(const df_column_t& other) {
    if (reserve(other.length)) {
        throw df_exception_not_enough_memory();
    }

    basic_extend_values(
        other.values, other.nulls, other.nulls + other.length,
        other.data_type, other.size_per_data,
        df_value_get_init_callback(data_type), df_value_get_load_callback(other.data_type),
        df_value_get_write_callback(other.data_type, data_type)
    );
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




// == iterator ==

class df_column_t::memory_iterator_t {
    friend class df_column_t;
protected:
    uint8_t* value_ptr = NULL;
    bool* null_ptr = NULL;
    int size_per_data = 0;

    constexpr memory_iterator_t(bool* p) : null_ptr(p) {}

    memory_iterator_t(uint8_t* value_ptr, bool* null_ptr, int size_per_data
    ) : value_ptr(value_ptr), null_ptr(null_ptr), size_per_data(size_per_data) {}
    
public:
    memory_iterator_t& operator++(int index) {
        value_ptr += size_per_data;
        null_ptr++;
        return *this;
    }

    memory_iterator_t& operator+=(int index) {
        value_ptr += index * size_per_data;
        null_ptr += index;
        return *this;
    }
    

    bool operator!=(const memory_iterator_t& other) const {
        return null_ptr != other.null_ptr;
    }

    uint8_t* get_value() const {
        return value_ptr;
    }

    bool* get_null() const {
        return null_ptr;
    }
};


class df_column_t::const_memory_iterator_t : public df_column_t::memory_iterator_t {
    friend class df_column_t;

    constexpr const_memory_iterator_t(bool* p) : memory_iterator_t(p) {}
    
    const_memory_iterator_t(uint8_t* value_ptr, bool* null_ptr, int size_per_data
    ) : memory_iterator_t(value_ptr, null_ptr, size_per_data) {}

public:
    const uint8_t* get_value() const {
        return value_ptr;
    }

    const bool* get_null() const {
        return null_ptr;
    }
};


class df_column_t::object_iterator_t : public df_column_t::memory_iterator_t {
    friend class df_column_t;
protected:
    df_object_t proxy;
    df_value_load_callback_t loader = NULL;

    constexpr object_iterator_t(bool* p) : memory_iterator_t(p) {}

    object_iterator_t(
        uint8_t* value_ptr, bool* null_ptr, int size_per_data,
        df_type_t type, df_value_load_callback_t loader
    ) : memory_iterator_t(value_ptr, null_ptr, size_per_data) {
        
        proxy.target_type = type;
        proxy.lock_state = true;
        
        this->loader = loader;
    }
public:
    df_object_t& operator*() {
        proxy.set_target(null_ptr, value_ptr, loader);
        return proxy;
    }
};


class df_column_t::const_object_iterator_t : public df_column_t::object_iterator_t {
    friend class df_column_t;

    constexpr const_object_iterator_t(bool* p) : object_iterator_t(p) {}

    const_object_iterator_t(
        uint8_t* value_ptr, bool* null_ptr, int size_per_data,
        df_type_t type, df_value_load_callback_t loader
    ) : object_iterator_t(value_ptr, null_ptr, size_per_data, type, loader) {}

};




df_column_t::memory_iterator_t df_column_t::memory_begin() {
    return memory_iterator_t{values, nulls, size_per_data};
}

df_column_t::memory_iterator_t df_column_t::memory_end() {
    return memory_iterator_t(nulls + length);
}



df_column_t::const_memory_iterator_t df_column_t::memory_begin() const {
    return const_memory_iterator_t{values, nulls, size_per_data};
}

df_column_t::const_memory_iterator_t df_column_t::memory_end() const {
    return const_memory_iterator_t(nulls + length);
}



df_column_t::object_iterator_t df_column_t::begin() {
    return object_iterator_t{values, nulls, size_per_data, data_type, df_value_get_load_callback(data_type)};
}

df_column_t::object_iterator_t df_column_t::end() {
    return object_iterator_t(nulls + length);
}



df_column_t::const_object_iterator_t df_column_t::begin() const {
    return const_object_iterator_t{values, nulls, size_per_data, data_type, df_value_get_load_callback(data_type)};
}

df_column_t::const_object_iterator_t df_column_t::end() const {
    return const_object_iterator_t(nulls + length);
}









// == print ==

std::ostream& df_column_t::write_stream(std::ostream& os, const char* column_name) const {
    df_value_load_callback_t loader = df_value_get_load_callback(data_type);
    df_value_write_callback_t writer = df_value_get_write_callback(data_type, DF_TYPE_TEXT);

    const_memory_iterator_t iter = memory_begin();
    df_value_t value;
    std::string buf;

    os << "| " << column_name << " |\n";
    for (; iter != memory_end(); iter++) {
        if (*iter.get_null()) {
            os << "| null |\n";
            continue;
        }

        value = loader(iter.get_value());
        writer(value, &buf);

        os << "| " << buf << " |\n";
    }
    return os;
}


std::ostream& operator<<(std::ostream& os, const df_column_t& column) {
    return column.write_stream(os, "VALUES");
}


