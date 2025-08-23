#ifndef _DF_TYPES_BYTE_HPP_
#define _DF_TYPES_BYTE_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPES_DATE_HPP_
#include "date.hpp"
#endif



// == df_value_t ==

typedef std::optional<std::string*> df_string_t;

typedef union df_value_t {
    void*   as_pointer;

    uint8_t as_uint8;
    short   as_uint16;
    int     as_int32;
    long    as_int64;

    float   as_float32;
    double  as_float64;

    df_string_t as_string;

    df_date_t as_datetime;

    bool as_bool;
} df_value_t;




// ==== df_mem_destroy ====





#endif // _DF_TYPES_BYTE_HPP_