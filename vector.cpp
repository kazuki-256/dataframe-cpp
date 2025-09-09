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

    df_column_t column(DF_TYPE_INT32, length);
    df_mem_block_t* block = column.blocks.tlBack();
    uint8_t* p = block->get_data_start();

    for (; start != end; start += interval, p += DF_TYPESIZE_INT32) {
        df_value_write_long_long(start, p);
    }
    memset(block->get_null_start(), false, length);

    column.length = block->usage = length;
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

    df_column_t column(DF_TYPE_INT64, length);
    df_mem_block_t* block = column.blocks.tlBack();
    uint8_t* p = block->get_data_start();

    for (; start != end; start += interval, p += DF_TYPESIZE_INT64) {
        df_value_write_long_long(start, p);
    }
    memset(block->get_null_start(), false, length);

    column.length = block->usage = length;
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

    df_column_t column(DF_TYPE_DATETIME, DF_MAX(predicted_length, DF_DEFAULT_COLUMN_SMALL_START_CAPACITY));
    df_mem_block_t* block = column.blocks.tlBack();
    uint8_t* p = block->get_data_start();

    // == way1: const interval calculation ==
    if (interval.is_constant()) {
        for (; start < end; start += const_interval) {
            df_value_write_long_long(start, p);
            p += DF_TYPESIZE_DATETIME;
        }
        memset(block->get_null_start(), false, predicted_length);

        block->usage = column.length = predicted_length;
        return column;
    }

    // == way2: variable interval calculation ==

    for (; start < end; start += interval) {
        if (block->usage >= block->capacity) {
            memset(block->get_null_start(), false, block->usage);
            column.length += block->usage;

            block = column.blocks.tlAdd(df_mem_block_t::create(DF_TYPE_DATETIME, column.length * 2));
            p = block->get_data_start();
        }

        df_value_write_long_long(start, p + (block->usage++) * DF_TYPESIZE_DATETIME);
    }
    memset(block->get_null_start(), false, block->usage);
    column.length += block->usage;
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




