#ifndef _DF_TYPES_BYTE_HPP_
#define _DF_TYPES_BYTE_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPES_DATE_HPP_
#include "date.hpp"
#endif


template<typename T>
using df_mem_callback_t = int (*)(void* mem, T& value);


// == release ==

void df_mem_release(void* mem, df_type_t type) {
    df_error_if_null_pointer(mem, );

    if (type == DF_TEXT) {
        ((std::string*)mem)->~basic_string();
    }
}


// == fail basic ==

template<typename DEST>
int df_mem_fall_basic(void* mem, DEST& value) {
    return 1;
}



// == set ==

template<typename DEST, typename SOURCE>
int df_mem_set_basic(void* mem, SOURCE& value) {
    *(DEST*)mem = value;
    return 0;
}

template<>
int df_mem_set_basic<uint8_t, const char*>(void* mem, const char* s) {
  long temp;
  sscanf(s, DF_INT_FORMAT, &temp);
  *(uint8_t*)mem = temp;
  return 0;
}

template<>
int df_mem_set_basic<short, const char*>(void* mem, const char* s) {
  long temp;
  sscanf(s, DF_INT_FORMAT, &temp);
  *(short*)mem = temp;
  return 0;
}

template<>
int df_mem_set_basic<int, const char*>(void* mem, const char* s) {
  sscanf(s, "%d", mem);
  return 0;
}

template<>
int df_mem_set_basic<long, const char*>(void* mem, const char* s) {
  sscanf(s, DF_INT_FORMAT, mem);
  return 0;
}

template<>
int df_mem_set_basic<float, const char*>(void* mem, const char* s) {
  sscanf(s, "%f", mem);
  return 0;
}

template<>
int df_mem_set_basic<double, const char*>(void* mem, const char* s) {
  sscanf(s, DF_FLOAT_FORMAT, mem);
  return 0;
}





template<typename T>
int df_mem_set(void* mem, df_type_t type, T& value) {
    df_error_if_null_pointer(mem, -1);

    constexpr static df_mem_callback_t<T> setter[] = {
        df_mem_fall_basic, df_mem_fall_basic,
        df_mem_set_basic<uint8_t>, df_mem_set_basic<short>, df_mem_set_basic<int>, df_mem_set_basic<long>,
        df_mem_set_basic<float>, df_mem_set_basic<double>,
        df_mem_set_basic<std::string>, df_mem_fall_basic,
        df_mem_fall_basic, df_mem_fall_basic, df_mem_fall_basic, df_mem_fall_basic,
        df_mem_set_basic<bool>
    };

    int type_number = df_type_get_number(type);
    if (type_number >= DF_TYPE_COUNT) {
        df_debug1("invalid type %d\n", type);
        return -1;
    }
    if (setter[type_number](mem, value)) {
        df_debug1("couldn't set %s to %s\n", df_type_get_string(df_type_get_type<T>), df_type_get_string(type));
        return -2;
    }
    return 0;
}




// == get ==

template<typename SOURCE, typename DEST>
int df_mem_get_basic(void* mem, DEST& value) {
    if constexpr (std::is_same_v<DEST, std::string>) {
        if constexpr (std::is_same_v<SOURCE, uint8_t>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT_FORMAT, *(uint8_t*)mem);
        }
        else if constexpr (std::is_same_v<SOURCE, short>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT_FORMAT, *(short*)mem);
        }
        else if constexpr (std::is_same_v<SOURCE, int>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT_FORMAT, *(int*)mem);
        }
        else if constexpr (std::is_same_v<SOURCE, long>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_INT_FORMAT, *(long*)mem);
        }
        else if constexpr (std::is_same_v<SOURCE, float>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_FLOAT_FORMAT, *(float*)mem);
        }
        else if constexpr (std::is_same_v<SOURCE, double>) {
            snprintf(DF_STATIC_BUFFER, DF_STATIC_BUFFER_LENGTH, DF_FLOAT_FORMAT, *(double*)mem);
        }
        else {
            value = *(SOURCE*)mem;
            return 0;
        }

        value = std::string(DF_STATIC_BUFFER);
        return 0;
    }
    else {
        value = *(SOURCE*)mem;
        return 0;
    }
}

template<>
int df_mem_get_basic<std::string, int>(void* mem, int& value) {
    sscanf(((std::string*)mem)->c_str(), "%d", value);
    return 0;
}


template<typename T>
int df_mem_get(void* mem, df_type_t type, T& value) {
    df_error_if_null_pointer(mem, -1);

    constexpr static df_mem_callback_t<T> setter[] = {
        df_mem_fall_basic, df_mem_fall_basic,
        df_mem_get_basic<uint8_t>, df_mem_get_basic<short>, df_mem_get_basic<int>, df_mem_get_basic<long>,
        df_mem_get_basic<float>, df_mem_get_basic<double>,
        df_mem_get_basic<std::string>, df_mem_fall_basic,
        df_mem_get_basic<df_date_t>, df_mem_get_basic<df_date_t>, df_mem_get_basic<df_date_t>, df_mem_get_basic<df_date_t>,
        df_mem_get_basic<bool>
    };

    int type_number = df_type_get_number(type);
    if (type_number >= DF_TYPE_COUNT) {
        df_debug1("invalid type %d\n", type);
        return -1;
    }
    if (setter[type_number](mem, value)) {
        df_debug1("couldn't get %s to %s\n", df_type_get_string(type), df_type_get_string(df_type_get_type<T>));
        return -2;
    }
    return 0;
}














#endif // _DF_TYPES_BYTE_HPP_