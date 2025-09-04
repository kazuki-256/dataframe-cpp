/* df_value_t: a low level object to provide register level free type handling

*/

#ifndef _DF_TYPES_VALUE_HPP_
#define _DF_TYPES_VALUE_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPES_DATE_HPP_
#include "date.hpp"
#endif



typedef union df_value_t {
    void* as_pointer;
    df_string_t* as_string;

    long as_long;
    double as_double;
    df_date_t as_date;

    inline ~df_value_t() {};
    inline df_value_t() {};

    inline df_value_t(void* pointer_value) : as_pointer(pointer_value) {}
    inline df_value_t(df_string_t* string_value) : as_string(string_value) {}

    inline df_value_t(float float_value) : as_double(float_value) {}
    inline df_value_t(double double_value) : as_double(double_value) {}

    inline df_value_t(uint8_t integer_value) : as_long(integer_value) {}
    inline df_value_t(short integer_value) : as_long(integer_value) {}
    inline df_value_t(int integer_value) : as_long(integer_value) {}
    inline df_value_t(long integer_value) : as_long(integer_value) {}

    inline df_value_t(df_date_t date_value) : as_date(date_value) {}
} df_value_t;





// ==== df_value_t release function ====

// just release, not free the pointer
inline void df_value_release(df_value_t value, df_type_t type) {
    if (type == DF_TYPE_TEXT) {
        value.as_string->reset();
    }
}

 
inline void df_value_release_string_mem(void* mem) {
    ((df_string_t*)mem)->reset();
}

// just release, not free the pointer
inline void df_value_release_mem(void* mem, df_type_t type) {
    if (type == DF_TYPE_TEXT) {
        df_value_release_string_mem(mem);
    }
}






// ==== df_value_t load funtions ====


typedef df_value_t (*df_value_load_callback_t)(const void* src);



inline df_value_t df_value_load_uint8(const void* src) {
    df_value_t value = *(uint8_t*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
    return value;
}

inline df_value_t df_value_load_short(const void* src) {
    df_value_t value = *(short*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
    return value;
}

inline df_value_t df_value_load_int(const void* src) {
    df_value_t value = *(int*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
    return value;
}

inline df_value_t df_value_load_long(const void* src) {
    return *(long*)src;
}



inline df_value_t df_value_load_float(const void* src) {
    df_value_t value = *(float*)src;

    if (value.as_long == DF_NULL_FLOAT32) {
        value.as_long = DF_NULL_FLOAT64;
    }
    return value;
}

inline df_value_t df_value_load_double(const void* src) {
    return *(double*)src;
}



inline df_value_t df_value_load_string(const void* src) {
    return (df_string_t*)src;
}






constexpr df_value_load_callback_t DF_VALUE_LOAD_CALLBACKS[DF_TYPE_COUNT] = {
    nullptr,               // DF_TYPEID_POINTER
    nullptr,               // DF_TYPEID_NULL

    df_value_load_uint8,   // DF_TYPEID_UINT8
    df_value_load_short,   // DF_TYPEID_INT16
    df_value_load_int,     // DF_TYPEID_INT32
    df_value_load_long,    // DF_TYPEID_INT64

    df_value_load_float,   // DF_TYPEID_FLOAT32
    df_value_load_double,  // DF_TYPEID_FLOAT64

    df_value_load_string,  // DF_TYPEID_TEXT
    df_value_load_int,     // DF_TYPEID_CATEGORY

    df_value_load_long,    // DF_TYPEID_DATE
    df_value_load_long,    // DF_TYPEID_TIME
    df_value_load_long,    // DF_TYPEID_DATETIME

    nullptr,               // DF_TYPEID_INTERVAL
    df_value_load_uint8,   // DF_TYPEID_BOOL
};



inline df_value_load_callback_t df_value_get_load_callback(df_type_t type) {
    return DF_VALUE_LOAD_CALLBACKS[df_type_get_typeid(type)];
}

inline df_value_t df_value_load(const void* src, df_type_t type) {
    df_error_if_null_pointer(src, 0);

    const int TYPE_ID = df_type_get_typeid(type);

    df_value_load_callback_t callback = DF_VALUE_LOAD_CALLBACKS[TYPE_ID];
    if (callback == NULL) {
        df_debug6("invalid load for type %s", df_type_get_string(type));
        return 0;
    }

    return callback(src);
}





// ==== df_value_t is_null functions ====

typedef bool (*df_value_is_null_callback_t)(df_value_t value);



inline bool df_value_is_null_long(df_value_t value) {
    return value.as_long == DF_NULL_INT64;
}

inline bool df_value_is_null_double(df_value_t value) {
    return value.as_long == DF_NULL_FLOAT64;
}

inline bool df_value_is_null_string(df_value_t value) {
    return !value.as_string->has_value();
}




constexpr df_value_is_null_callback_t DF_VALUE_IS_NULL_CALLBACKS[DF_TYPE_COUNT] = {
    nullptr,               // DF_TYPEID_POINTER
    nullptr,               // DF_TYPEID_NULL

    df_value_is_null_long,   // DF_TYPEID_UINT8
    df_value_is_null_long,   // DF_TYPEID_INT16
    df_value_is_null_long,   // DF_TYPEID_INT32
    df_value_is_null_long,   // DF_TYPEID_INT64

    df_value_is_null_double, // DF_TYPEID_FLOAT32
    df_value_is_null_double, // DF_TYPEID_FLOAT64

    df_value_is_null_string, // DF_TYPEID_TEXT
    df_value_is_null_long,   // DF_TYPEID_CATEGORY

    df_value_is_null_long,   // DF_TYPEID_DATE
    df_value_is_null_long,   // DF_TYPEID_TIME
    df_value_is_null_long,   // DF_TYPEID_DATETIME

    nullptr,                 // DF_TYPEID_INTERVAL
    df_value_is_null_long,   // DF_TYPEID_BOOL
};



inline df_value_is_null_callback_t df_value_get_is_null_callback(df_type_t type) {
    return DF_VALUE_IS_NULL_CALLBACKS[df_type_get_typeid(type)];
}

inline bool df_value_is_null(df_value_t value, df_type_t type) {
    df_value_is_null_callback_t callback = df_value_get_is_null_callback(type);
    
    if (callback == NULL) {
        df_debug6("invalid load for type %s", df_type_get_string(type));
        return true;
    }

    return callback(value);
}




// ==== df_value_t write functions ====

typedef df_value_t (*df_value_write_callback_t)(df_value_t value, void* dest);



// == write null ==

inline df_value_t df_value_write_null_uint8(df_value_t value, void* dest) {
    *(uint8_t*)dest = DF_NULL_UINT8;
    return value;
}

inline df_value_t df_value_write_null_short(df_value_t value, void* dest) {
    *(short*)dest = DF_NULL_INT16;
    return value;
}

inline df_value_t df_value_write_null_int(df_value_t value, void* dest) {
    *(int*)dest = DF_NULL_INT32;
    return value;
}

inline df_value_t df_value_write_null_long(df_value_t value, void* dest) {
    *(long*)dest = DF_NULL_INT64;
    return value;
}



inline df_value_t df_value_write_null_float(df_value_t value, void* dest) {
    *(int*)dest = DF_NULL_FLOAT32;
    return value;
}

inline df_value_t df_value_write_null_double(df_value_t value, void* dest) {
    *(long*)dest = DF_NULL_FLOAT64;
    return value;
}



inline df_value_t df_value_write_null_string(df_value_t value, void* dest) {
    ((df_string_t*)dest)->reset();
    return dest;
}





// == write long ==

inline df_value_t df_value_write_long_uint8(df_value_t value, void* dest) {
    *(uint8_t*)dest = df_value_is_null_long(value) ? DF_NULL_UINT8 : (uint8_t)value.as_long;
    return value;
}

inline df_value_t df_value_write_long_short(df_value_t value, void* dest) {
    *(short*)dest = df_value_is_null_long(value) ? DF_NULL_INT16 : (short)value.as_long;
    return value;
}

inline df_value_t df_value_write_long_int(df_value_t value, void* dest) {
    *(int*)dest = df_value_is_null_long(value) ? DF_NULL_INT32 : (int)value.as_long;
    return value;
}

inline df_value_t df_value_write_long_long(df_value_t value, void* dest) {
    *(long*)dest = value.as_long;
    return value;
}



inline df_value_t df_value_write_long_float(df_value_t value, void* dest) {
    if (df_value_is_null_long(value)) {
        *(int*)dest = DF_NULL_FLOAT32;
    }
    else {
        *(float*)dest = (float)value.as_long;
    }
    return value;
}

inline df_value_t df_value_write_long_double(df_value_t value, void* dest) {
    if (df_value_is_null_long(value)) {
        *(long*)dest = DF_NULL_FLOAT64;
    }
    else {
        *(double*)dest = (double)value.as_long;
    }
    return value;
}



inline df_value_t df_value_write_long_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    df_string_t& str = *dest_value.as_string;

    if (df_value_is_null_long(value)) {
        str.reset();
        return dest_value;
    }
    
    if (!str.has_value()) str.emplace();
    str->resize(32, '\0');
    str->resize(snprintf(str->data(), str->size(), DF_INT64_FORMAT, value.as_long));

    return dest_value;
}



// == write double ==

inline df_value_t df_value_write_double_uint8(df_value_t value, void* dest) {
    *(uint8_t*)dest = df_value_is_null_double(value) ? DF_NULL_UINT8 : (uint8_t)value.as_double;
    return value;
}

inline df_value_t df_value_write_double_short(df_value_t value, void* dest) {
    *(short*)dest = df_value_is_null_double(value) ? DF_NULL_INT16 : (short)value.as_double;
    return value;
}

inline df_value_t df_value_write_double_int(df_value_t value, void* dest) {
    *(int*)dest = df_value_is_null_double(value) ? DF_NULL_INT32 : (int)value.as_double;
    return value;
}

inline df_value_t df_value_write_double_long(df_value_t value, void* dest) {
    *(long*)dest = df_value_is_null_double(value) ? DF_NULL_INT64 : (long)value.as_double;
    return value;
}



inline df_value_t df_value_write_double_float(df_value_t value, void* dest) {
    *(float*)dest = df_value_is_null_double(value) ? DF_NULL_FLOAT32 : (float)value.as_double;
    return value;
}

inline df_value_t df_value_write_double_double(df_value_t value, void* dest) {
    *(double*)dest = value.as_double;
    return value;
}



inline df_value_t df_value_write_double_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    df_string_t& str = *dest_value.as_string;

    if (df_value_is_null_double(value)) {
        str.reset();
        return dest_value;
    }
    
    if (!str.has_value()) str.emplace();
    str->resize(32, '\0');
    str->resize(snprintf(str->data(), str->size(), DF_FLOAT64_FORMAT, value.as_double));

    return dest_value;
}




// == write string ==

inline df_value_t df_value_write_string_uint8(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    *(uint8_t*)dest = dest_value.as_long =
        !str.has_value() ? DF_NULL_UINT8 : std::stoul(*str);
    
    return dest_value;
}

inline df_value_t df_value_write_string_short(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    *(short*)dest = dest_value.as_long =
        !str.has_value() ? DF_NULL_INT16 : std::stoi(*str);
    
    return dest_value;
}

inline df_value_t df_value_write_string_int(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    *(int*)dest = dest_value.as_long =
        !str.has_value() ? DF_NULL_INT32 : std::stoi(*str);
    
    return dest_value;
}

inline df_value_t df_value_write_string_long(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    *(long*)dest = dest_value.as_long =
        !str.has_value() ? DF_NULL_INT64 : std::stol(*str);
    
    return dest_value;
}



inline df_value_t df_value_write_string_float(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    if (!str.has_value()) {
        *(int*)dest = dest_value.as_long = DF_NULL_FLOAT32;
    }
    else {
        *(float*)dest = dest_value.as_double = std::stof(*str);
    }
    return dest_value;
}

inline df_value_t df_value_write_string_double(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    if (!str.has_value()) {
        *(long*)dest = dest_value.as_long = DF_NULL_FLOAT64;
    }
    else {
        *(float*)dest = dest_value.as_double = std::stod(*str);
    }
    return dest_value;
}



inline df_value_t df_value_write_string_date(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    if (!str.has_value()) {
        *(long*)dest = dest_value.as_long = DF_NULL_DATETIME;
    }
    else {
        dest_value.as_date.parse_date(str->c_str(), DF_DATE_FORMAT);
        *((df_date_t*)dest) = dest_value.as_date;
    }
    return dest_value;
}

inline df_value_t df_value_write_string_time(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    if (!str.has_value()) {
        *(long*)dest = dest_value.as_long = DF_NULL_DATETIME;
    }
    else {
        dest_value.as_date.parse_date(str->c_str(), DF_TIME_FORMAT);
        *((df_date_t*)dest) = dest_value.as_date;
    }
    return dest_value;
}

inline df_value_t df_value_write_string_datetime(df_value_t value, void* dest) {
    df_value_t dest_value;
    df_string_t& str = *value.as_string;

    if (!str.has_value()) {
        *(long*)dest = dest_value.as_long = DF_NULL_DATETIME;
    }
    else {
        dest_value.as_date.parse_date(str->c_str(), DF_DATETIME_FORMAT);
        *((df_date_t*)dest) = dest_value.as_date;
    }
    return dest_value;
}



inline df_value_t df_value_write_string_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    *dest_value.as_string = *value.as_string;
    return dest_value;
}



// == date, time, datetime to text ==

inline df_value_t df_value_write_date_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    df_string_t& str = *dest_value.as_string;

    if (df_value_is_null_long(value)) {
        str.reset();
        return dest_value;
    }
    
    str.emplace(64, '\0');
    value.as_date.c_str(DF_DATE_FORMAT, str->data());
    return dest_value;
}

inline df_value_t df_value_write_time_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    df_string_t& str = *dest_value.as_string;

    if (df_value_is_null_long(value)) {
        str.reset();
        return dest_value;
    }
    
    str.emplace(64, '\0');
    value.as_date.c_str(DF_TIME_FORMAT, str->data());
    return dest_value;
}

inline df_value_t df_value_write_datetime_string(df_value_t value, void* dest) {
    df_value_t dest_value = dest;
    df_string_t& str = *dest_value.as_string;

    if (df_value_is_null_long(value)) {
        str.reset();
        return dest_value;
    }
    
    str.emplace(64, '\0');
    value.as_date.c_str(DF_DATETIME_FORMAT, str->data());
    return dest_value;
}










constexpr df_value_write_callback_t DF_VALUE_WRITE_CALLBACKS[DF_TYPE_COUNT][DF_TYPE_COUNT] = {
    // POINTER ->
    {
        df_value_write_null_long, NULL,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL
    },
    // NULL ->
    {
        NULL, NULL,
        df_value_write_null_uint8, df_value_write_null_short, df_value_write_null_int, df_value_write_null_long,
        df_value_write_null_float, df_value_write_null_double,
        df_value_write_null_string, NULL,
        df_value_write_null_long, df_value_write_null_long, df_value_write_null_long, NULL,
        df_value_write_null_uint8
    },
    // UINT8 ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT16 ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT32 ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INT64 ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // FLOAT32 ->
    {
        NULL, NULL,
        df_value_write_double_uint8, df_value_write_double_short, df_value_write_double_int, df_value_write_double_long,
        df_value_write_double_float, df_value_write_double_double,
        df_value_write_double_string, NULL,
        df_value_write_double_long, df_value_write_double_long, df_value_write_double_long, NULL,
        df_value_write_double_uint8
    },
    // FLOAT64 ->
    {
        NULL, NULL,
        df_value_write_double_uint8, df_value_write_double_short, df_value_write_double_int, df_value_write_double_long,
        df_value_write_double_float, df_value_write_double_double,
        df_value_write_double_string, NULL,
        df_value_write_double_long, df_value_write_double_long, df_value_write_double_long, NULL,
        df_value_write_double_uint8
    },
    // TEXT ->
    {
        NULL, NULL,
        df_value_write_string_uint8, df_value_write_string_short, df_value_write_string_int, df_value_write_string_long,
        df_value_write_string_float, df_value_write_string_double,
        df_value_write_string_string, NULL,
        df_value_write_string_date, df_value_write_string_time, df_value_write_string_datetime, NULL,
        df_value_write_string_uint8
    },
    // CATEGORY ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // DATE ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_date_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // TIME ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_time_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // DATETIME ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_datetime_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // INTERVAL ->
    {
        NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL
    },
    // BOOL ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
};



inline df_value_write_callback_t df_value_get_write_callback(df_type_t src_type, df_type_t dest_type) {
    return DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(src_type)][df_type_get_typeid(dest_type)];
}

inline df_value_t df_value_write(df_value_t src, df_type_t src_type, void* dest, df_type_t dest_type) {
    df_error_if_null_pointer(dest, 0);

    df_value_write_callback_t callback =
        DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(src_type)][df_type_get_typeid(dest_type)];

    if (callback == NULL) {
        df_debug6("invalid write from type %s to type %s",
            df_type_get_string(src_type), df_type_get_string(dest_type));
        return 0;
    }

    return callback(src, dest);
}







#endif // _DF_TYPES_VALUE_HPP_
