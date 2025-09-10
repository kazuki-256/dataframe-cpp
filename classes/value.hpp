/* df_value_t: a low level object to provide register level free type handling

*/
#pragma once

#include "../config.hpp"
#include "date.hpp"
#include "exception.hpp"
#include "type.hpp"



typedef union df_value_t {
    void* as_pointer;
    std::string* as_string;

    long as_long;
    double as_double;
    df_date_t as_date;

    ~df_value_t() {};
    constexpr df_value_t() : as_long(0) {};

    df_value_t(void* pointer_value) : as_pointer(pointer_value) {}
    df_value_t(std::string* string_value) : as_string(string_value) {}

    df_value_t(float float_value) : as_double(float_value) {}
    df_value_t(double double_value) : as_double(double_value) {}

    df_value_t(uint8_t integer_value) : as_long(integer_value) {}
    df_value_t(short integer_value) : as_long(integer_value) {}
    df_value_t(int integer_value) : as_long(integer_value) {}
    df_value_t(long integer_value) : as_long(integer_value) {}

    df_value_t(df_date_t date_value) : as_date(date_value) {}
} df_value_t;





// ==== df_value_t release function ====

typedef void (*df_value_release_callback_t)(void* mem);

 
void df_value_release_string(void* mem) {
    ((std::string*)mem)->~basic_string();
}


df_value_release_callback_t df_value_get_release_callback(df_type_t type) {
    if (type == DF_TYPE_TEXT) {
        return df_value_release_string;
    }
return NULL;
}


// just release, not free the pointer
void df_value_release(void* mem, df_type_t type) {
    df_value_release_callback_t release = df_value_get_release_callback(type);

    if (release) {
        release(mem);
    }
}



// ==== df_value_t init ====

typedef void (*df_value_init_callback_t)(void* mem);


void df_value_init_string(void* mem) {
    new (mem) std::string();
}




constexpr df_value_init_callback_t df_value_get_init_callback(df_type_t type) {
    if (type == DF_TYPE_STRING) {
        return df_value_init_string;
    }
    return NULL;
}


void df_value_init(void* mem, df_type_t type) {
    df_value_init_callback_t callback = df_value_get_init_callback(type);
    
    if (callback) {
        callback(mem);
    }
}




// ==== df_value_t load funtions ====


typedef df_value_t (*df_value_load_callback_t)(const void* src);



df_value_t df_value_load_uint8(const void* src) {
    return *(uint8_t*)src;
}

df_value_t df_value_load_short(const void* src) {
    return *(short*)src;
}

df_value_t df_value_load_int(const void* src) {
    return *(int*)src;
}

df_value_t df_value_load_long(const void* src) {
    return *(long*)src;
}



df_value_t df_value_load_float(const void* src) {
    return *(float*)src;
}

df_value_t df_value_load_double(const void* src) {
    return *(double*)src;
}



df_value_t df_value_load_struct(const void* src) {
    return (void*)src;
}






constexpr df_value_load_callback_t DF_VALUE_LOAD_CALLBACKS[DF_TYPE_COUNT] = {
    df_value_load_long,    // DF_TYPEID_POINTER

    df_value_load_uint8,   // DF_TYPEID_UINT8
    df_value_load_short,   // DF_TYPEID_INT16
    df_value_load_int,     // DF_TYPEID_INT32
    df_value_load_long,    // DF_TYPEID_INT64

    df_value_load_float,   // DF_TYPEID_FLOAT32
    df_value_load_double,  // DF_TYPEID_FLOAT64

    df_value_load_struct,  // DF_TYPEID_TEXT
    df_value_load_int,     // DF_TYPEID_CATEGORY

    df_value_load_long,    // DF_TYPEID_DATE
    df_value_load_long,    // DF_TYPEID_TIME
    df_value_load_long,    // DF_TYPEID_DATETIME

    df_value_load_struct,  // DF_TYPEID_INTERVAL
    df_value_load_uint8,   // DF_TYPEID_BOOL
};



constexpr df_value_load_callback_t df_value_get_load_callback(df_type_t type) {
    return DF_VALUE_LOAD_CALLBACKS[df_type_get_typeid(type)];
}

df_value_t df_value_load(const void* src, df_type_t type) {
    DF_RETURN_IF_NULL(src, 0);
    return df_value_get_load_callback(type)(src);
}



// ==== df_value_t write functions ====

typedef df_value_t (*df_value_write_callback_t)(df_value_t value, void* dest);





// == write long ==

df_value_t df_value_write_long_uint8(df_value_t value, void* dest) {
    *(uint8_t*)dest = (uint8_t)value.as_long;
    return value;
}

df_value_t df_value_write_long_short(df_value_t value, void* dest) {
    *(short*)dest = (short)value.as_long;
    return value;
}

df_value_t df_value_write_long_int(df_value_t value, void* dest) {
    *(int*)dest = (int)value.as_long;
    return value;
}

df_value_t df_value_write_long_long(df_value_t value, void* dest) {
    *(long*)dest = value.as_long;
    return value;
}



df_value_t df_value_write_long_float(df_value_t value, void* dest) {
    df_value_t dest_value = (double)value.as_long;
    *(float*)dest = dest_value.as_long;
    return dest_value;
}

df_value_t df_value_write_long_double(df_value_t value, void* dest) {
    df_value_t dest_value = (double)value.as_long;
    *(double*)dest = dest_value.as_long;
    return dest_value;
}



df_value_t df_value_write_long_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    std::string& str = *dest_value.as_string;

    str.resize(32, '\0');
    str.resize(snprintf(str.data(), str.size(), DF_INT64_FORMAT, value.as_long));

    return dest_value;
}



// == write double ==

df_value_t df_value_write_double_uint8(df_value_t value, void* dest) {
    df_value_t dest_value = (long)value.as_double;
    *(uint8_t*)dest = dest_value.as_long;
    return dest_value;
}

df_value_t df_value_write_double_short(df_value_t value, void* dest) {
    df_value_t dest_value = (long)value.as_double;
    *(short*)dest = dest_value.as_long;
    return dest_value;
}

df_value_t df_value_write_double_int(df_value_t value, void* dest) {
    df_value_t dest_value = (long)value.as_double;
    *(int*)dest = dest_value.as_long;
    return dest_value;
}

df_value_t df_value_write_double_long(df_value_t value, void* dest) {
    df_value_t dest_value = (long)value.as_double;
    *(long*)dest = dest_value.as_long;
    return dest_value;
}



df_value_t df_value_write_double_float(df_value_t value, void* dest) {
    *(float*)dest = (float)value.as_double;
    return value;
}

df_value_t df_value_write_double_double(df_value_t value, void* dest) {
    *(double*)dest = value.as_double;
    return value;
}



df_value_t df_value_write_double_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    std::string& str = *dest_value.as_string;

    str.resize(32, '\0');
    str.resize(snprintf(str.data(), str.size(), DF_FLOAT64_FORMAT, value.as_double));

    return dest_value;
}




// == write string ==

df_value_t df_value_write_string_uint8(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(uint8_t*)dest = dest_value.as_long = std::stoul(*value.as_string);
    return dest_value;
}

df_value_t df_value_write_string_short(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(short*)dest = dest_value.as_long = std::stoi(*value.as_string);
    return dest_value;
}

df_value_t df_value_write_string_int(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(int*)dest = dest_value.as_long = std::stoi(*value.as_string);
    return dest_value;
}

df_value_t df_value_write_string_long(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(long*)dest = dest_value.as_long = std::stol(*value.as_string);
    return dest_value;
}



df_value_t df_value_write_string_float(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(float*)dest = dest_value.as_double = std::stof(*value.as_string);
    return dest_value;
}

df_value_t df_value_write_string_double(df_value_t value, void* dest) {
    df_value_t dest_value;
    *(float*)dest = dest_value.as_double = std::stod(*value.as_string);
    return dest_value;
}



df_value_t df_value_write_string_date(df_value_t value, void* dest) {
    df_value_t dest_value;
    dest_value.as_date.parse_date(value.as_string->c_str(), DF_DATE_FORMAT);
    *(df_date_t*)dest = dest_value.as_date;
    return dest_value;
}

df_value_t df_value_write_string_time(df_value_t value, void* dest) {
    df_value_t dest_value;
    dest_value.as_date.parse_date(value.as_string->c_str(), DF_TIME_FORMAT);
    *(df_date_t*)dest = dest_value.as_date;
    return dest_value;
}

df_value_t df_value_write_string_datetime(df_value_t value, void* dest) {
    df_value_t dest_value;
    dest_value.as_date.parse_date(value.as_string->c_str(), DF_DATETIME_FORMAT);
    *(df_date_t*)dest = dest_value.as_date;
    return dest_value;
}



df_value_t df_value_write_string_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    *dest_value.as_string = *value.as_string;
    return dest_value;
}



// == date, time, datetime to text ==

df_value_t df_value_write_date_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    std::string& str = *dest_value.as_string;

    str.resize(64, '\0');
    value.as_date.c_str(DF_DATE_FORMAT, str.data());
    return dest_value;
}

df_value_t df_value_write_time_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    std::string& str = *dest_value.as_string;
    
    str.resize(64, '\0');
    value.as_date.c_str(DF_TIME_FORMAT, str.data());
    return dest_value;
}

df_value_t df_value_write_datetime_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    std::string& str = *dest_value.as_string;
    
    str.resize(64, '\0');
    value.as_date.c_str(DF_DATETIME_FORMAT, str.data());
    return dest_value;
}










constexpr df_value_write_callback_t DF_VALUE_WRITE_CALLBACKS[DF_TYPE_COUNT][DF_TYPE_COUNT] = {
    // POINTER ->
    {
        df_value_write_long_long,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL
    },
    // UINT8 ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT16 ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT32 ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT64 ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // FLOAT32 ->
    {
        NULL,
        df_value_write_double_uint8, df_value_write_double_short, df_value_write_double_int, df_value_write_double_long,
        df_value_write_double_float, df_value_write_double_double,
        df_value_write_double_string, NULL,
        df_value_write_double_long, df_value_write_double_long, df_value_write_double_long, NULL,
        df_value_write_double_uint8
    },
    // FLOAT64 ->
    {
        NULL,
        df_value_write_double_uint8, df_value_write_double_short, df_value_write_double_int, df_value_write_double_long,
        df_value_write_double_float, df_value_write_double_double,
        df_value_write_double_string, NULL,
        df_value_write_double_long, df_value_write_double_long, df_value_write_double_long, NULL,
        df_value_write_double_uint8
    },
    // TEXT ->
    {
        NULL,
        df_value_write_string_uint8, df_value_write_string_short, df_value_write_string_int, df_value_write_string_long,
        df_value_write_string_float, df_value_write_string_double,
        df_value_write_string_string, NULL,
        df_value_write_string_date, df_value_write_string_time, df_value_write_string_datetime, NULL,
        df_value_write_string_uint8
    },
    // CATEGORY ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // DATE ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_date_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // TIME ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_time_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // DATETIME ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_datetime_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INTERVAL ->
    {
        NULL,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL
    },
    // BOOL ->
    {
        NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
};



constexpr df_value_write_callback_t df_value_get_write_callback(df_type_t src_type, df_type_t dest_type) {
    return DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(src_type)][df_type_get_typeid(dest_type)];
}



df_value_t df_value_write(df_value_t src, df_type_t src_type, void* dest, df_type_t dest_type) {
    DF_RETURN_IF_NULL(dest, 0);

    df_value_write_callback_t callback = df_value_get_write_callback(src_type, dest_type);
    
    if (callback == NULL) {
        df_debug6("invalid write type %s to type %s",
            df_type_get_string(src_type), df_type_get_string(dest_type));
        return 0;
    }

    return callback(src, dest);
}




// ==== move ====


df_value_t df_value_move_string_string(df_value_t src, void* dest) {
    *(std::string*)dest = std::move(*src.as_string);
    return dest;
}



constexpr df_value_write_callback_t df_value_get_move_callback(df_type_t src_type, df_type_t dest_type) {
    if (src_type == DF_TYPE_STRING && dest_type == DF_TYPE_STRING) {
        return df_value_move_string_string;
    }
    return df_value_get_write_callback(src_type, dest_type);
}


df_value_t df_value_move(df_value_t src, df_type_t src_type, void* dest, df_type_t dest_type) {
    DF_RETURN_IF_NULL(dest, 0);

    df_value_write_callback_t writer = df_value_get_move_callback(src_type, dest_type);
    if (writer == NULL) {
        df_debug6("invalid move type %s to type %s",
            df_type_get_string(src_type), df_type_get_string(dest_type));
        return 0;
    }

    return writer(src, dest);
}

