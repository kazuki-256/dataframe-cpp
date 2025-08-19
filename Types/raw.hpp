#ifndef _DF_TYPES_RAW_HPP_
#define _DF_TYPES_RAW_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_TYPES_EXCEPTION_HPP_
#include "exception.hpp"
#endif

#ifndef _DF_TYPES_DATE_HPP_
#include "date.hpp"
#endif




template<df_type_t TYPE>
class df_raw_t {
    static_assert(
        df_get_type_category(TYPE) == DF_TYPE_CATEGORY_INT ||
        df_get_type_category(TYPE) == DF_TYPE_CATEGORY_FLOAT ||
        TYPE == DF_BOOLEAN ||
        df_get_type_category(TYPE) == DF_TYPE_CATEGORY_DATE,
        "invalid type");

    char bytes[df_get_type_size(TYPE)];


    template<typename T>
    inline T& cast() const {
        return *(T*)bytes;
    }

    inline void from_string(const char* str) {
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_INT) {
            long src;
            sscanf(str, "%ld", &src);
            *this = src;
            return;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_FLOAT) {
            double src;
            sscanf(str, "%lf", &src);
            *this = src;
            return;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_BOOLEAN) {
            cast<bool>() = str ? str[0] : false;
            return;
        }
        else if constexpr (TYPE == DF_DATE) {
            cast<df_date_t>() = df_date_t(str, DF_DATE_FORMAT);
            return;
        }
        else if constexpr (TYPE == DF_TIME) {
            cast<df_date_t>() = df_date_t(str, DF_TIME_FORMAT);
            return;
        }
        else if constexpr (TYPE == DF_DATETIME || TYPE == DF_INTERVAL) {
            cast<df_date_t>() = df_date_t(str, DF_DATETIME_FORMAT);
            return;
        }
    }

public:
    // == construct ==

    inline df_raw_t() {
        *this = 0;
    }

    template<typename T>
    inline df_raw_t(T val) {
        *this = val;
    }


    // == cast ==
    
    template<df_type_t DEST>
    operator df_raw_t<DEST>&() {
        return *(df_raw_t<DEST>*)this;
    }


    // == set/get ==

    template<typename T>
    inline df_raw_t& operator=(T& val) {
        if constexpr (std::is_same_v<T, const char*>) {
            from_string(val);
            return *this;
        }
        else if constexpr (std::is_same_v<T, df_raw_t<DF_NULL>>) {
            as_null();
            return *this;
        }
        else if constexpr (TYPE == DF_UINT8) {
            cast<uint8_t>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_INT16) {
            cast<short>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_INT32) {
            cast<int>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_INT64) {
            cast<long>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_FLOAT32) {
            cast<float>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_FLOAT64) {
            cast<double>() = val;
            return *this;
        }
        else if constexpr (TYPE == DF_BOOLEAN) {
            cast<bool>() = val;
            return *this;
        }
        else if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_DATE) {
            cast<df_date_t>() = val;
            return *this;
        }
    }


    template<typename T>
    inline operator T() const {
        if constexpr (TYPE == DF_UINT8) {
            return cast<uint8_t>();
        }
        if constexpr (TYPE == DF_INT16) {
            return cast<short>();
        }
        if constexpr (TYPE == DF_INT32) {
            return cast<int>();
        }
        if constexpr (TYPE == DF_INT64) {
            return cast<long>();
        }
        if constexpr (TYPE == DF_FLOAT32) {
            return cast<float>();
        }
        if constexpr (TYPE == DF_FLOAT64) {
            return cast<long>();
        }
        if constexpr (TYPE == DF_BOOLEAN) {
            return cast<bool>();
        }
        if constexpr (TYPE == DF_DATE) {
            return cast<df_date_t>();
        }
    }


    // == null ==

    inline df_raw_t& as_null() {
        if constexpr (TYPE == DF_UINT8) {
            cast<uint8_t>() = DF_NULL_UINT8;
        }
        if constexpr (TYPE == DF_INT16) {
            cast<short>() = DF_NULL_INT16;
        }
        if constexpr (TYPE == DF_INT32) {
            cast<int>() = DF_NULL_INT32;
        }
        if constexpr (TYPE == DF_INT64) {
            cast<long>() = DF_NULL_INT64;
        }
        if constexpr (TYPE == DF_FLOAT32) {
            cast<float>() = DF_NULL_FLOAT32;
        }
        if constexpr (TYPE == DF_FLOAT64) {
            cast<long>() = DF_NULL_FLOAT64;
        }
        if constexpr (TYPE == DF_BOOLEAN) {
            cast<bool>() = DF_NULL_BOOLEAN;
        }
        if constexpr (TYPE == DF_BOOLEAN) {
            cast<df_date_t>() = DF_NULL_DATE;
        }
        return *this;
    }

    bool is_null() const {
        if constexpr (TYPE == DF_UINT8) {
            return cast<uint8_t>() == DF_NULL_UINT8;
        }
        if constexpr (TYPE == DF_INT16) {
            return cast<short>() == DF_NULL_INT16;
        }
        if constexpr (TYPE == DF_INT32) {
            return cast<int>() == DF_NULL_INT32;
        }
        if constexpr (TYPE == DF_INT64) {
            return cast<long>() == DF_NULL_INT64;
        }
        if constexpr (TYPE == DF_FLOAT32) {
            return cast<float>() == DF_NULL_FLOAT32;
        }
        if constexpr (TYPE == DF_FLOAT64) {
            return cast<long>() == DF_NULL_FLOAT64;
        }
        if constexpr (TYPE == DF_BOOLEAN) {
            return cast<bool>() == DF_NULL_BOOLEAN;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_DATE) {
            return cast<df_date_t>() == DF_NULL_DATE;
        }
    }


    // // == equal ==

    inline bool operator==(const df_raw_t& other) const {
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_INT) {
            return (long)(*this) == (long)other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_FLOAT) {
            return (double)(*this) == (double)other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_BOOLEAN) {
            return (bool)(*this) == (bool)other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_DATE) {
            return (df_date_t)(*this) == (df_date_t)other;
        }
    }

    template<typename T>
    inline bool operator==(const T& other) const {
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_INT) {
            return (long)(*this) == other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_FLOAT) {
            return (double)(*this) == other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_BOOLEAN) {
            return (bool)(*this) == other;
        }
        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_DATE) {
            return (df_date_t)(*this) == other;
        }
    }


    // == c_str() ==

    static const char* static_c_str(const df_raw_t& raw, char* buffer, size_t buffer_size) {
        if (raw.is_null()) {
            return "null";
        }

        if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_INT) {
            snprintf(buffer, buffer_size, DF_INT_FORMAT, (long)raw);
            return buffer;
        }
        else if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_FLOAT) {
            snprintf(buffer, buffer_size, DF_FLOAT_FORMAT, (double)raw);
            return buffer;
        }
        else if constexpr (df_get_type_category(TYPE) == DF_TYPE_CATEGORY_BOOLEAN) {
            return raw.cast<bool> ? "true" : "false";
        }
        else if constexpr (TYPE == DF_DATE) {
            return raw.cast<df_date_t>().c_str(DF_DATE_FORMAT);
        }
        else if constexpr (TYPE == DF_TIME) {
            return raw.cast<df_date_t>().c_str(DF_TIME_FORMAT);
        }
        else if constexpr (TYPE == DF_DATETIME) {
            return raw.cast<df_date_t>().c_str(DF_DATETIME_FORMAT);
        }
    }

    const char* c_str(char* buffer = DF_STATIC_BUFFER, size_t buffer_size = DF_STATIC_BUFFER_LENGTH) const {
        return static_c_str(*this, buffer, buffer_size);
    }
};





template<>
class df_raw_t<DF_UNDEFINED> {
    char bytes[df_get_type_size(DF_UNDEFINED)];

public:
    df_raw_t() = delete;

    template<df_type_t DEST>
    operator df_raw_t<DEST>&() {
        return *(df_raw_t<DEST>*)this;
    }


    bool is_null() {
        return true;
    }

    static const char* static_c_str(const df_raw_t& raw, char*, size_t) {
        return "undefined";
    }

    const char* c_str() const {
        return "undefined";
    }
};





template<>
class df_raw_t<DF_NULL> {
    friend df_raw_t<DF_NULL> df_null();

    char bytes[df_get_type_size(DF_NULL)];

    df_raw_t() {}

public:
    template<df_type_t OTHER>
    inline bool operator==(const df_raw_t<OTHER>& other) const {
        return other.is_null();
    }

    template<df_type_t DEST>
    inline operator df_raw_t<DEST>() const {
        return df_raw_t<DEST>().as_null();
    }
};

df_raw_t<DF_NULL> df_null() {
    return df_raw_t<DF_NULL>();
}




template<>
class df_raw_t<DF_TEXT> {
    union U {
        char bytes[32];     // <-- force the size as 32bytes
        std::string* real;

        U() {}
        ~U() {}
    } value;


    void from_copy(const df_raw_t& other) {
        if (other.is_null()) {
            value.real = NULL;
            return;
        }
        value.real = new std::string(*other.value.real);
    }

    inline void release() {
        if (value.real) {
            printf("delete\n");
            delete value.real;
            value.real = NULL;
        }
    }

public:
    ~df_raw_t() {
        release();
    }


    df_raw_t() {
        value.real = NULL;
    }

    df_raw_t(std::string& text) {
        value.real = new std::string(text);
    }

    df_raw_t(const char* text) {
        value.real = new std::string(text);
    }


    // == cast ==

    template<df_type_t DEST>
    operator df_raw_t<DEST>&() {
        return *(df_raw_t<DEST>*)this;
    }

    // == move ==

    df_raw_t(df_raw_t&& other) {
        value.real = other.value.real;
        other.value.real = NULL;
    }

    df_raw_t& operator=(df_raw_t&& other) {
        release();
        
        value.real = other.value.real;
        other.value.real = NULL;
        return *this;
    }


    // == copy ==

    df_raw_t(const df_raw_t& other) {
        from_copy(other);
    }

    df_raw_t& operator=(const df_raw_t& other) {
        release();
        from_copy(other);
        return *this;
    }
    



    // == convert ==

    operator std::string&() {
        return *value.real;
    }

    // operator const std::string&() const {
    //     return is_null() ? *value.real : NULL;
    // }


    // == null ==

    df_raw_t<DF_TEXT>& as_null() {
        release();
        return *this;
    }

    bool is_null() const {
        return value.real == NULL;
    }


    // == operator ==

    bool operator==(const df_raw_t& other) const {
        if (value.real == other.value.real) {
            return true;
        }
        if (is_null()) {
            return false;
        }
        return value.real->compare(*other.value.real) == 0;
    }


    // == c_str() ==


    static const char* static_c_str(const df_raw_t& raw, char*, size_t) {
        if (raw.is_null()) {
            return "null";
        }
        return raw.value.real->c_str();
    }

    const char* c_str() const {
        return static_c_str(*this, NULL, 0);
    }
};




#endif // _DF_TYPES_RAW_HPP_