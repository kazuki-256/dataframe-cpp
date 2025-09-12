#pragma once

#include "../config.hpp"
#include "column.hpp"



class df_range_objects_t {
    friend class df_column_t;
protected:
    df_column_t* column = NULL;
    long index = 0;
    long interval = 0;

    df_range_objects_t(df_column_t* column, long index, long interval);

public:
    df_memory_iterator_t memory_begin();
    df_memory_iterator_t memory_end();

    df_object_iterator_t begin();
    df_object_iterator_t end();


    std::ostream& write_stream(std::ostream& stream, const char* outside_name) const;
    friend std::ostream& operator<<(std::ostream& stream, const char* outside_name);
};



class df_const_range_objects_t : df_range_objects_t {
    friend class df_column_t;

    df_const_range_objects_t(df_column_t* column, long index, long interval);
public:
    df_const_memory_iterator_t memory_begin() const;
    df_const_memory_iterator_t memory_end() const;

    df_const_object_iterator_t begin() const;
    df_const_object_iterator_t end() const;
};




