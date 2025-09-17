#pragma once

#include "classes/column.cpp"




// == range ==


df_column_t df_range_int32(int start, int end, int interval = 1) {
    df_debug6("make column");

    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    long length = (end - start) / interval;
    if (length < 0) {
        throw df_exception_endless_range();
    }

    df_column_t column;
    column._init(DF_TYPE_INT32, length, length);

    df_memory_iterator_t iter = column.memory_begin();

    for (; start != end; start += interval, iter++) {
        *iter.get_null() = false;
        df_value_write_long_long(start, iter.get_value());
    }
    return column;
}



df_column_t df_range_int64(long start, long end, long interval = 1) {
    df_debug6("make column");

    if (interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    long length = (end - start) / interval;
    if (length < 0) {
        throw df_exception_endless_range();
    }

    df_column_t column;
    column._init(DF_TYPE_INT64, length, length);

    df_memory_iterator_t iter = column.memory_begin();

    for (; start != end; start += interval, iter++) {
        *iter.get_null() = false;
        df_value_write_long_long(start, iter.get_value());
    }
    return column;
}



df_column_t df_range_datetime(df_date_t start, df_date_t end, df_interval_t interval) {
    df_debug6("make column");

    const long const_interval = interval.calculate_constant();
    if (const_interval == 0) {
        throw df_exception_interval_couldnot_be_0();
    }

    const long predicted_length = ((time_t)end - (time_t)start) / const_interval;
    if (predicted_length < 0) {
        throw df_exception_endless_range();
    }

    df_column_t column;
    column._init(DF_TYPE_DATETIME, 0, predicted_length);

    df_memory_iterator_t iter = column.memory_begin();

    // == way1: const interval calculation ==
    if (interval.is_constant()) {
        column.length = predicted_length;
        
        for (; start < end; start += const_interval, iter++) {
            *iter.get_null() = false;
            df_value_write_long_long(start, iter.get_value());
        }
        return column;
    }

    // == way2: variable interval calculation ==

    for (; start < end; iter++, start += interval) {
        int ret = column.reserve(1);
        if (ret != 0) {
            if (ret > 0) {
                iter = column.memory_begin() += column.length;
            }
            else {
                throw df_exception_not_enough_memory();
            }
        }
        
        *iter.get_null() = false;
        df_value_write_long_long(start, iter.get_value());
        column.length++;
    }
    return column;
}



df_column_t df_range_date(df_date_t start, df_date_t end, df_interval_t interval) {
    df_debug6("make column");
    
    df_column_t column = df_range_datetime(start, end, interval);
    column.data_type = DF_TYPE_DATE;
    return column;
}



df_column_t df_range_time(df_date_t start, df_date_t end, df_interval_t interval) {
    df_debug6("make column");
    
    df_column_t column = df_range_datetime(start, end, interval);
    column.data_type = DF_TYPE_DATE;
    return column;
}



df_column_t df_range_time(const char* start, const char* end, df_interval_t interval) {
    return df_range_time(df_date_t(start, DF_TIME_FORMAT), df_date_t(end, DF_TIME_FORMAT), interval);
}






// == randrange ==

df_column_t df_randrange_int32(int min, int max, long length);

df_column_t df_randrange_int64(double min, double max, long length);

df_column_t df_randrange_float32(float min, float max, long length);

df_column_t df_randrange_float64(double min, double max, long length);




