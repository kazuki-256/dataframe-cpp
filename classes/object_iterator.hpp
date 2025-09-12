#pragma once

#include "object.hpp"
#include "column.hpp"



class df_memory_iterator_t {
    friend class df_column_t;

    friend class df_row_range_t;
protected:
    df_column_t* source = NULL;
    long index = 0;

    
    constexpr df_memory_iterator_t(df_column_t* column, long index);

public:
    inline df_memory_iterator_t& operator++();

    inline df_memory_iterator_t& operator++(int);

    inline df_memory_iterator_t& operator+=(long offset);


    inline bool operator!=(const df_memory_iterator_t& other) const;


    inline bool* get_null() const;

    inline uint8_t* get_value() const;
};



class df_const_memory_iterator_t : public df_memory_iterator_t {
    friend class df_column_t;

    constexpr df_const_memory_iterator_t(const df_column_t* column, long index);
public:
    inline const bool* get_null() const;

    inline const uint8_t* get_value() const;
};




class df_object_iterator_t : public df_memory_iterator_t {
    friend class df_column_t;
protected:
    df_object_t proxy;

    constexpr df_object_iterator_t(long index);

    inline df_object_iterator_t(df_column_t* column, long index);
public:

    inline df_object_t& operator*();
};


class df_const_object_iterator_t : public df_object_iterator_t {
    friend class df_column_t;

    inline df_const_object_iterator_t(const df_column_t* column, long index);

public:
    constexpr df_const_object_iterator_t(long index);

    inline const df_object_t& operator*();
};

