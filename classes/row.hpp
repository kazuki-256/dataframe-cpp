#pragma once

#include "../config.hpp"
#include "column.hpp"



/*
three step to find target:
1. from matched cashe, a pointer number to target map
2. if step 1 couldn't find target, find target from object list, a struct to store running variables, also make matched cashe
3. if step 1 and 2 couldn't finded, create object iter and matched cashe from real data (df_column_t)

*/
class df_const_row_t {
    friend class df_dataframe_t;
    friend class df_const_range_rows_t;
protected:
    struct object_info_t {
        const std::string* name;
        df_column_t* column;
        df_object_t object;
    };

    struct matched_info_t {
        const void* address;
        object_info_t* object_info;
    };

    std::vector<df_named_column_t>* source = NULL;

    matched_info_t* matched_start = NULL;   // base size: (COLUMN_LENGTH * 2 + 1), realloc every p >= match_end
    matched_info_t* matched_end = NULL;     // at match_start + match_length

    object_info_t* object_start = NULL;     // size: COLUMN_LENGTH
    object_info_t* object_end = NULL;       // always start + length, ordered if (uint8_t)(start + length) - 1

    long current = 0;
    long interval = 0;


    df_const_row_t(const std::vector<df_named_column_t>* columns, long index, long interval);

    constexpr df_const_row_t(long index);


    inline const df_object_t& basic_at(const char* name, object_info_t*& info);

public:
    // == destroy ==

    ~df_const_row_t();

    // == get ==

    int get_length() const;



    df_const_row_t& operator*();


    const df_object_t& at(const char* name);

    const df_object_t& operator[](const char* name);

    // == other ==

    df_const_row_t& operator++();

    bool operator!=(const df_const_row_t& other);

    
    // == iterator ==
    
    class iterator_t;

    iterator_t begin();
    
    iterator_t end();


    // == write_stream ==
    
    std::ostream& write_stream(std::ostream& os);

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_t& row);
};


class df_row_t : public df_const_row_t {
    friend class df_dataframe_t;
    friend class df_range_rows_t;

    df_row_t(std::vector<df_named_column_t>* columns, long index, long interval);

    constexpr df_row_t(long index);
public:
    df_object_t& operator[](const char* name);
    
    
    // == write_stream ==

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_t& row);
};


