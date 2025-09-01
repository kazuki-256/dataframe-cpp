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

    df_value_t() : as_long(0) {};
    ~df_value_t() {};
} df_value_t;





// ==== df_value_t release function ====

// just release, not free the pointer
void df_value_release(df_value_t& value, df_type_t type) {
    if (type == DF_TYPE_TEXT) {
        value.as_string->reset();
    }
}

// just release, not free the pointer
void df_value_release_mem(void* mem, df_type_t type) {
    if (type == DF_TYPE_TEXT) {
        ((df_string_t*)mem)->reset();
    }
}





// ==== df_value_t load funtions ====


typedef void (*df_value_load_callback_t)(df_value_t& value, void* src);



void df_value_load_uint8(df_value_t& value, void* src) {
    value.as_long = *(uint8_t*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
}

void df_value_load_short(df_value_t& value, void* src) {
    value.as_long = *(short*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
}

void df_value_load_int(df_value_t& value, void* src) {
    value.as_long = *(int*)src;

    if (value.as_long == DF_NULL_UINT8) {
        value.as_long = DF_NULL_INT64;
    }
}

void df_value_load_long(df_value_t& value, void* src) {
    value.as_long = *(long*)src;
}



void df_value_load_float(df_value_t& value, void* src) {
    value.as_long = *(long*)src;

    if (value.as_long == DF_NULL_FLOAT32) {
        value.as_long = DF_NULL_FLOAT64;
    }
}

void df_value_load_double(df_value_t& value, void* src) {
    value.as_double= *(double*)src;
}



void df_value_load_string(df_value_t& value, void* src) {
    value.as_string = (df_string_t*)src;
}






df_value_load_callback_t DF_VALUE_LOAD_CALLBACKS[DF_TYPE_COUNT] = {
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


void df_value_load(df_value_t& value, void* src, df_type_t type) {
    df_error_if_null_pointer(src, );

    df_type_id_t type_id = df_type_get_typeid(type);

    df_value_load_callback_t callback = DF_VALUE_LOAD_CALLBACKS[type_id];
    if (callback == NULL) {
        df_debug6("invalid load for type %s", df_type_get_string(type));
        return;
    }

    callback(value, src);
    return;
}





// ==== df_value_t is_null functions ====

typedef bool (*df_value_is_null_callback_t)(const df_value_t& value);



bool df_value_is_null_long(const df_value_t& value) {
    return value.as_long == DF_NULL_INT64;
}

bool df_value_is_null_double(const df_value_t& value) {
    return value.as_long == DF_NULL_FLOAT64;
}

bool df_value_is_null_string(const df_value_t& value) {
    return !value.as_string->has_value();
}




df_value_is_null_callback_t DF_VALUE_IS_NULL_CALLBACKS[DF_TYPE_COUNT] = {
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



bool df_value_is_null(df_value_t& value, df_type_t type) {
    df_value_is_null_callback_t callback =
        DF_VALUE_IS_NULL_CALLBACKS[df_type_get_typeid(type)];
    
    if (callback == NULL) {
        df_debug6("invalid load for type %s", df_type_get_string(type));
        return true;
    }

    return callback(value);
}




// ==== df_value_t write functions ====

typedef void (*df_value_write_callback_t)(const df_value_t& value, void* dest);



// == write null ==

void df_value_write_null_uint8(const df_value_t& value, void* dest) {
    *(uint8_t*)dest = DF_NULL_UINT8;
}

void df_value_write_null_short(const df_value_t& value, void* dest) {
    *(short*)dest = DF_NULL_INT16;
}

void df_value_write_null_int(const df_value_t& value, void* dest) {
    *(int*)dest = DF_NULL_INT32;
}

void df_value_write_null_long(const df_value_t& value, void* dest) {
    *(long*)dest = DF_NULL_INT64;
}



void df_value_write_null_float(const df_value_t& value, void* dest) {
    *(int*)dest = DF_NULL_FLOAT32;
}

void df_value_write_null_double(const df_value_t& value, void* dest) {
    *(long*)dest = DF_NULL_FLOAT64;
}



void df_value_write_null_string(const df_value_t& value, void* dest) {
    ((df_string_t*)dest)->reset();
}





// == write long ==

void df_value_write_long_uint8(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_INT64) {
        *(uint8_t*)dest = DF_NULL_UINT8;
        return;
    }
    *(uint8_t*)dest = value.as_long;
}

void df_value_write_long_short(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_INT64) {
        *(short*)dest = DF_NULL_INT16;
        return;
    }
    *(short*)dest = value.as_long;
}

void df_value_write_long_int(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_INT64) {
        *(int*)dest = DF_NULL_INT32;
        return;
    }
    *(int*)dest = value.as_long;
}

void df_value_write_long_long(const df_value_t& value, void* dest) {
    *(long*)dest = value.as_long;
}



void df_value_write_long_float(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_INT64) {
        *(int*)dest = DF_NULL_FLOAT32;
        return;
    }
    *(float*)dest = value.as_long;
}

void df_value_write_long_double(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_INT64) {
        *(long*)dest = DF_NULL_FLOAT64;
        return;
    }
    *(double*)dest = value.as_long;
}



void df_value_write_long_string(const df_value_t& value, void* dest) {
    df_string_t& str = *(df_string_t*)dest;

    if (value.as_long == DF_NULL_INT64) {
        str.reset();
        return;
    }
    
    if (!str.has_value()) str.emplace();
    str->resize(32, '\0');
    str->resize(snprintf(str->data(), str->size(), DF_INT64_FORMAT, value.as_long));
}



// == write double ==

void df_value_write_double_uint8(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_FLOAT64) {
        *(uint8_t*)dest = DF_NULL_UINT8;
        return;
    }
    *(uint8_t*)dest = value.as_double;
}

void df_value_write_double_short(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_FLOAT64) {
        *(short*)dest = DF_NULL_INT16;
        return;
    }
    *(short*)dest = value.as_double;
}

void df_value_write_double_int(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_FLOAT64) {
        *(int*)dest = DF_NULL_INT32;
        return;
    }
    *(int*)dest = value.as_double;
}

void df_value_write_double_long(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_FLOAT64) {
        *(long*)dest = DF_NULL_INT64;
        return;
    }
    *(long*)dest = value.as_double;
}



void df_value_write_double_float(const df_value_t& value, void* dest) {
    if (value.as_long == DF_NULL_FLOAT64) {
        *(int*)dest = DF_NULL_FLOAT32;
        return;
    }
    *(float*)dest = value.as_double;
}

void df_value_write_double_double(const df_value_t& value, void* dest) {
    *(double*)dest = value.as_double;
}



void df_value_write_double_string(const df_value_t& value, void* dest) {
    df_string_t& str = *(df_string_t*)dest;

    if (value.as_long == DF_NULL_FLOAT64) {
        str.reset();
        return;
    }
    
    if (!str.has_value()) str.emplace();
    str->resize(32, '\0');
    str->resize(snprintf(str->data(), str->size(), DF_FLOAT64_FORMAT, value.as_double));
}




// == write string ==

void df_value_write_string_uint8(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(uint8_t*)dest = std::stoul(*str);
        return;
    }
    *(uint8_t*)dest = DF_NULL_UINT8;
}

void df_value_write_string_short(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(short*)dest = std::stol(*str);
        return;
    }
    *(short*)dest = DF_NULL_INT16;
}

void df_value_write_string_int(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(int*)dest = std::stol(*str);
        return;
    }
    *(int*)dest = DF_NULL_INT32;
}

void df_value_write_string_long(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(long*)dest = std::stol(*str);
        return;
    }
    *(long*)dest = DF_NULL_INT64;
}



void df_value_write_string_float(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(float*)dest = std::stof(*str);
        return;
    }
    *(int*)dest = DF_NULL_FLOAT32;
}

void df_value_write_string_double(const df_value_t& value, void* dest) {
    df_string_t& str = *value.as_string;

    if (str.has_value()) {
        *(double*)dest = std::stod(*str);
        return;
    }
    *(long*)dest = DF_NULL_FLOAT64;
}



void df_value_write_string_string(const df_value_t& value, void* dest) {
    *(df_string_t*)dest = *value.as_string;
}










df_value_write_callback_t DF_VALUE_WRITE_CALLBACKS[DF_TYPE_COUNT][DF_TYPE_COUNT] = {
    // POINTER ->
    {
        NULL, NULL,
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
        NULL, NULL, NULL, NULL,
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
        NULL, NULL, NULL, NULL,
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
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // TIME ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
        df_value_write_long_long, df_value_write_long_long, df_value_write_long_long, NULL,
        df_value_write_long_uint8
    },
    // DATETIME ->
    {
        NULL, NULL,
        df_value_write_long_uint8, df_value_write_long_short, df_value_write_long_int, df_value_write_long_long,
        df_value_write_long_float, df_value_write_long_double,
        df_value_write_long_string, NULL,
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





void df_value_write(const df_value_t& src, df_type_t src_type, void* dest, df_type_t dest_type) {
    df_error_if_null_pointer(dest, );

    df_value_write_callback_t callback =
        DF_VALUE_WRITE_CALLBACKS[df_type_get_typeid(src_type)][df_type_get_typeid(dest_type)];

    if (callback == NULL) {
        df_debug6("invalid write from type %s to type %s",
            df_type_get_string(src_type), df_type_get_string(dest_type));
        return;
    }

    callback(src, dest);
}







#endif // _DF_TYPES_VALUE_HPP_
