#pragma once

#include "../config.hpp"
#include "column.hpp"


/*
three step to find target:
1. from matched cashe, a pointer number to target map
2. if step 1 couldn't find target, find target from object list, a struct to store running variables, also make matched cashe
3. if step 1 and 2 couldn't finded, create object iter and matched cashe from real data (df_column_t)

*/
class df_row_t {
    friend class df_dataframe_t;
    friend class df_row_range_t;
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

    typedef std::vector<std::pair<std::string, df_column_t*>> source_t;

    source_t* source = NULL;

    mutable matched_info_t* matched_start = NULL;   // base size: (COLUMN_LENGTH * 2 + 1), realloc every p >= match_end
    mutable matched_info_t* matched_end = NULL;     // at match_start + match_length

    mutable object_info_t* object_start = NULL;     // size: COLUMN_LENGTH
    mutable object_info_t* object_end = NULL;       // always start + length, ordered if (uint8_t)(start + length) - 1

    long current = 0;
    long interval = 0;
    mutable bool ordered = false;


    df_row_t(source_t* columns, long index, long interval);

    constexpr df_row_t(long index);


    inline object_info_t* _add_object_cashe(std::pair<std::string, df_column_t*>& pair) const;

    inline df_object_t& _at(const char* name, object_info_t*& info) const;

public:
    // == destroy ==

    ~df_row_t();

    // == get ==

    int get_column_count() const;



    df_row_t& operator*();


    df_object_t& at(const char* name) const;

    df_object_t& operator[](const char* name) const;

    // == other ==

    df_row_t& operator++();

    bool operator!=(const df_row_t& other) const;

    
    // == iterator ==
    
    class iterator_t;

    iterator_t begin() const;
    
    iterator_t end() const;


    // == write_stream ==
    
    std::ostream& write_stream(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const df_row_t& row);
};





class df_const_row_t : public df_row_t {
    friend class df_dataframe_t;
    friend class df_const_row_range_t;

    typedef const std::vector<std::pair<std::string, df_column_t*>> const_source_t;

    df_const_row_t(const_source_t* columns, long index, long interval);

    constexpr df_const_row_t(long index);
public:
    const df_object_t& operator[](const char* name) const;
    
    
    // == write_stream ==

    friend std::ostream& operator<<(std::ostream& os, const df_const_row_t& row);
};


