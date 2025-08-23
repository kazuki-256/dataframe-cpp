#ifndef _DF_TYPES_BYTE_HPP_
#define _DF_TYPES_BYTE_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPES_DATE_HPP_
#include "date.hpp"
#endif



// == df_value_t ==

typedef union df_value_t {
    void*   as_pointer;

    uint8_t as_uint8;
    short   as_uint16;
    int     as_int32;
    long    as_int64;

    float   as_float32;
    double  as_float64;

    df_string_t* as_string;

    df_date_t as_datetime;

    bool as_bool;
} df_value_t;




using df_mem_callback1_t = bool (*)(const void* mem);
using df_mem_callback2_t = void (*)(const void* src, void* dest);



// == nothing callback ==

void df_mem_nothing_callback(const void*, void*) {}


// == release ==

void df_mem_release(void* mem, df_type_t mem_type) {
    df_error_if_null_pointer(mem, );

    if (mem_type == DF_TYPE_TEXT) {
        ((df_string_t*)mem)->reset();
    }
}

void df_mem_release_string(void* mem) {
    df_error_if_null_pointer(mem, );

    ((df_string_t*)mem)->reset();
}

void df_mem_reuse(void* mem, df_type_t mem_type, df_type_t dest_type) {
    df_error_if_null_pointer(mem, );

    if (mem_type == DF_TYPE_TEXT) {
        ((df_string_t*)mem)->reset();
    }
    if (dest_type == DF_TYPE_TEXT) {
        new (mem) df_string_t();
    }
}



// ==== is null ====

template<typename T>
bool df_mem_is_null_basic(const void* mem) {
    return true;
}


template<>
bool df_mem_is_null_basic<uint8_t>(const void* mem) {
    return *(uint8_t*)mem == DF_NULL_UINT8;
}

template<>
bool df_mem_is_null_basic<short>(const void* mem) {
    return *(short*)mem == DF_NULL_INT16;
}

template<>
bool df_mem_is_null_basic<int>(const void* mem) {
    return *(int*)mem == DF_NULL_INT32;
}

template<>
bool df_mem_is_null_basic<long>(const void* mem) {
    return *(long*)mem == DF_NULL_INT64;
}

template<>
bool df_mem_is_null_basic<float>(const void* mem) {
    return *(float*)mem == DF_NULL_FLOAT32;
}

template<>
bool df_mem_is_null_basic<double>(const void* mem) {
    return *(double*)mem == DF_NULL_FLOAT64;
}

template<>
bool df_mem_is_null_basic<df_date_t>(const void* mem) {
    return *(df_date_t*)mem == DF_NULL_DATE;
}


template<>
bool df_mem_is_null_basic<bool>(const void* mem) {
    return *(uint8_t*)mem == DF_NULL_BOOL;
}

template<>
bool df_mem_is_null_basic<df_string_t>(const void* mem) {
    return !((df_string_t*)mem)->has_value();
}





bool df_mem_is_null(const void* mem, df_type_t type) {
    df_error_if_null_pointer(mem, true);

    constexpr static df_mem_callback1_t setter[] = {
        NULL, NULL,
        df_mem_is_null_basic<uint8_t>, df_mem_is_null_basic<short>, df_mem_is_null_basic<int>, df_mem_is_null_basic<long>,
        df_mem_is_null_basic<float>, df_mem_is_null_basic<double>,
        df_mem_is_null_basic<df_string_t>, NULL,
        df_mem_is_null_basic<df_date_t>, df_mem_is_null_basic<df_date_t>, df_mem_is_null_basic<df_date_t>, NULL,
        df_mem_is_null_basic<bool>
    };

    df_mem_callback1_t callback = setter[df_type_get_typeid(type)];
    return callback ? callback(mem) : true;
}




// ==== convert ====

template<typename SRC, typename DEST>
void df_mem_convert_basic(const void* src, void* dest) {
    *(DEST*)dest = *(SRC*)src;
}


// == const char* to value ==

template<>
void df_mem_convert_basic<df_string_t, uint8_t>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(uint8_t*)dest = DF_NULL_UINT8;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    int temp;
    sscanf(str->c_str(), DF_INT32_FORMAT, &temp);
    *(uint8_t*)dest = temp;
}

template<>
void df_mem_convert_basic<df_string_t, short>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(short*)dest = DF_NULL_INT16;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    int temp;
    sscanf(str->c_str(), DF_INT32_FORMAT, &temp);
    *(short*)dest = temp;
}

template<>
void df_mem_convert_basic<df_string_t, int>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(int*)dest = DF_NULL_INT32;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    sscanf(str->c_str(), DF_INT32_FORMAT, (int*)dest);
}

template<>
void df_mem_convert_basic<df_string_t, long>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(long*)dest = DF_NULL_INT64;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    sscanf(str->c_str(), DF_INT64_FORMAT, (long*)dest);
}

template<>
void df_mem_convert_basic<df_string_t, float>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(float*)dest = DF_NULL_FLOAT32;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    sscanf(str->c_str(), DF_FLOAT32_FORMAT, (float*)dest);
}

template<>
void df_mem_convert_basic<df_string_t, double>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(double*)dest = DF_NULL_FLOAT64;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    sscanf(str->c_str(), DF_FLOAT64_FORMAT, (double*)dest);
}

template<>
void df_mem_convert_basic<df_string_t, df_date_t>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(df_date_t*)dest = DF_NULL_DATE;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    *(df_date_t*)dest = str->c_str();
}

template<>
void df_mem_convert_basic<df_string_t, bool>(const void* src, void* dest) {
    if (df_mem_is_null_basic<df_string_t>(src)) {
        *(uint8_t*)dest = DF_NULL_BOOL;
        return;
    }

    df_string_t& str = *(df_string_t*)src;
    *(bool*)dest = str->empty();
}


// == null to value ==

template<>
void df_mem_convert_basic<df_null_t, void*>(const void* src, void* dest) {
    *(void**)dest = NULL;
}

template<>
void df_mem_convert_basic<df_null_t, uint8_t>(const void* src, void* dest) {
    *(uint8_t*)dest = DF_NULL_UINT8;
}

template<>
void df_mem_convert_basic<df_null_t, short>(const void* src, void* dest) {
    *(short*)dest = DF_NULL_INT16;
}

template<>
void df_mem_convert_basic<df_null_t, int>(const void* src, void* dest) {
    *(int*)dest = DF_NULL_INT32;
}

template<>
void df_mem_convert_basic<df_null_t, long>(const void* src, void* dest) {
    *(long*)dest = DF_NULL_INT64;
}

template<>
void df_mem_convert_basic<df_null_t, float>(const void* src, void* dest) {
    *(float*)dest = DF_NULL_FLOAT32;
}

template<>
void df_mem_convert_basic<df_null_t, double>(const void* src, void* dest) {
    *(double*)dest = DF_NULL_FLOAT64;
}



template<>
void df_mem_convert_basic<df_null_t, df_date_t>(const void* src, void* dest) {
    *(df_date_t*)dest = DF_NULL_DATE;
}


template<>
void df_mem_convert_basic<df_null_t, bool>(const void* src, void* dest) {
    *(uint8_t*)dest = DF_NULL_BOOL;
}




// == value to df_string_t ==

template<typename SRC>
void df_mem_convert_any_df_string(const void* src, void* dest) {
    fprintf(stderr, "debug: %s to TEXT\n", df_type_get_string(df_type_get_type<SRC>));
    *(df_string_t*)dest = (std::string)*(SRC*)src;
}


// void* to string

template<>
void df_mem_convert_any_df_string<void*>(const void* src, void* dest) {
    if (df_mem_is_null_basic<void*>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, "%p", *(void**)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}


// df_null_t to string

template<>
void df_mem_convert_any_df_string<df_null_t>(const void* src, void* dest) {
    ((df_string_t*)dest)->reset();
}


// numbers to df_string_t

template<>
void df_mem_convert_any_df_string<uint8_t>(const void* src, void* dest) {
    if (df_mem_is_null_basic<uint8_t>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT32_FORMAT, *(uint8_t*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<short>(const void* src, void* dest) {
    if (df_mem_is_null_basic<short>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT32_FORMAT, *(short*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<int>(const void* src, void* dest) {
    if (df_mem_is_null_basic<int>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT32_FORMAT, *(int*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<long>(const void* src, void* dest) {
    if (df_mem_is_null_basic<long>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT64_FORMAT, *(long*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<float>(const void* src, void* dest) {
    if (df_mem_is_null_basic<float>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_FLOAT32_FORMAT, *(float*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<double>(const void* src, void* dest) {
    if (df_mem_is_null_basic<double>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_FLOAT64_FORMAT, *(double*)src);
        *(df_string_t*)dest = DF_STATIC_BUFFER;
    }
}

template<>
void df_mem_convert_any_df_string<bool>(const void* src, void* dest) {
    df_debug6("bool (%d) to string", *(uint8_t*)src);
    if (df_mem_is_null_basic<uint8_t>(src)) {
        ((df_string_t*)dest)->reset();
    }
    else {
        *(df_string_t*)dest = *(bool*)src ? "true" : "false";
    }
}


// string to string

template<>
void df_mem_convert_any_df_string<df_string_t>(const void* src, void* dest) {
    *(df_string_t*)dest = *(df_string_t*)src;
}


// string to date/time

void df_mem_convert_date_df_string(const void* src, void* dest) {
    *(df_string_t*)dest = ((df_date_t*)src)->c_str(DF_DATE_FORMAT);
}

void df_mem_convert_time_df_string(const void* src, void* dest) {
    *(df_string_t*)dest = ((df_date_t*)src)->c_str(DF_TIME_FORMAT);
}

void df_mem_convert_datetime_df_string(const void* src, void* dest) {
    *(df_string_t*)dest = ((df_date_t*)src)->c_str(DF_DATETIME_FORMAT);
}

void df_mem_convert_interval_df_string(const void* src, void* dest) {
    *(df_string_t*)dest = ((df_date_t*)src)->c_str(DF_DATETIME_FORMAT);
}








// == main convertor ==


constexpr static df_mem_callback2_t DF_MEM_CONVERT_TABLE[DF_TYPE_COUNT][DF_TYPE_COUNT] = {
    // void* -> any
    {
        df_mem_convert_basic<long, long>, df_mem_nothing_callback,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        df_mem_convert_any_df_string<void*>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<void*, bool>
    },

    // null -> any
    {
        df_mem_convert_basic<df_null_t, void*>, df_mem_nothing_callback,
        df_mem_convert_basic<df_null_t, uint8_t>, df_mem_convert_basic<df_null_t, short>, df_mem_convert_basic<df_null_t, int>, df_mem_convert_basic<df_null_t, long>,
        df_mem_convert_basic<df_null_t, float>, df_mem_convert_basic<df_null_t, double>,
        df_mem_convert_any_df_string<df_null_t>, NULL,
        df_mem_convert_basic<df_null_t, df_date_t>, df_mem_convert_basic<df_null_t, df_date_t>, df_mem_convert_basic<df_null_t, df_date_t>, NULL,
        df_mem_convert_basic<df_null_t, bool>
    },

    // uint8_t -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<uint8_t, uint8_t>, df_mem_convert_basic<uint8_t, short>, df_mem_convert_basic<uint8_t, int>, df_mem_convert_basic<uint8_t, long>,
        df_mem_convert_basic<uint8_t, float>, df_mem_convert_basic<uint8_t, double>,
        df_mem_convert_any_df_string<uint8_t>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<uint8_t, bool>
    },

    // short -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<short, uint8_t>, df_mem_convert_basic<short, short>, df_mem_convert_basic<short, int>, df_mem_convert_basic<short, long>,
        df_mem_convert_basic<short, float>, df_mem_convert_basic<short, double>, 
        df_mem_convert_any_df_string<short>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<short, bool>
    },

    // int -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<int, uint8_t>, df_mem_convert_basic<int, short>, df_mem_convert_basic<int, int>, df_mem_convert_basic<int, long>,
        df_mem_convert_basic<int, float>, df_mem_convert_basic<int, double>,
        df_mem_convert_any_df_string<int>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<int, bool>
    },

    // long -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<long, uint8_t>, df_mem_convert_basic<long, short>, df_mem_convert_basic<long, int>, df_mem_convert_basic<long, long>,
        df_mem_convert_basic<long, float>, df_mem_convert_basic<long, double>,
        df_mem_convert_any_df_string<long>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<long, bool>
    },

    // float -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<float, uint8_t>, df_mem_convert_basic<float, short>, df_mem_convert_basic<float, int>, df_mem_convert_basic<float, long>,
        df_mem_convert_basic<float, float>, df_mem_convert_basic<float, double>,
        df_mem_convert_any_df_string<float>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<float, bool>
    },

    // double -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<double, uint8_t>, df_mem_convert_basic<double, short>, df_mem_convert_basic<double, int>, df_mem_convert_basic<double, long>,
        df_mem_convert_basic<double, float>, df_mem_convert_basic<double, double>,
        df_mem_convert_any_df_string<double>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<double, bool>
    },

    // text -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<df_string_t, uint8_t>, df_mem_convert_basic<df_string_t, short>, df_mem_convert_basic<df_string_t, int>, df_mem_convert_basic<df_string_t, long>,
        df_mem_convert_basic<df_string_t, float>, df_mem_convert_basic<df_string_t, double>,
        df_mem_convert_any_df_string<df_string_t>, NULL,
        df_mem_convert_basic<df_string_t, df_date_t>, df_mem_convert_basic<df_string_t, df_date_t>, df_mem_convert_basic<df_string_t, df_date_t>, NULL,
        df_mem_convert_basic<df_string_t, bool>
    },

    // category -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<int, uint8_t>, df_mem_convert_basic<int, short>, df_mem_convert_basic<int, int>, df_mem_convert_basic<int, long>,
        NULL, NULL,
        /* category string convert must done at column or object */ NULL, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<int, bool>
    },

    // date -> any
    {
        NULL, df_mem_nothing_callback,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        df_mem_convert_date_df_string, NULL,
        df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, NULL,
        df_mem_convert_basic<long, bool>
    },

    // time -> any
    {
        NULL, df_mem_nothing_callback,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        df_mem_convert_time_df_string, NULL,
        df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, NULL,
        df_mem_convert_basic<long, bool>
    },

    // datetime -> any
    {
        NULL, df_mem_nothing_callback,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        df_mem_convert_datetime_df_string, NULL,
        df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, df_mem_convert_basic<long, long>, NULL,
        df_mem_convert_basic<long, bool>
    },

    // interval -> any
    {
        NULL, df_mem_nothing_callback,
        NULL, NULL, NULL, NULL,
        NULL, NULL,
        df_mem_convert_any_df_string<df_interval_t>, NULL,
        NULL, NULL, NULL, NULL,
        NULL,
    },

    // bool -> any
    {
        NULL, df_mem_nothing_callback,
        df_mem_convert_basic<bool, uint8_t>, df_mem_convert_basic<bool, short>, df_mem_convert_basic<bool, int>, df_mem_convert_basic<bool, long>,
        df_mem_convert_basic<bool, float>, df_mem_convert_basic<bool, double>,
        df_mem_convert_any_df_string<bool>, NULL,
        NULL, NULL, NULL, NULL,
        df_mem_convert_basic<bool, bool>
    },
};




int df_mem_convert(const void* src, df_type_t src_type, void* dest, df_type_t dest_type) {
    df_error_if_null_pointer(src, -1);
    df_error_if_null_pointer(src, -2);

    df_mem_callback2_t convertor = DF_MEM_CONVERT_TABLE[df_type_get_typeid(src_type)][df_type_get_typeid(dest_type)];

    if (convertor == NULL) {
        df_debug6("invalid convert: %s -> %s", df_type_get_string(src_type), df_type_get_string(dest_type));
        return -3;
    } 
    
    convertor(src, dest);
    return 0;
}







#endif // _DF_TYPES_BYTE_HPP_