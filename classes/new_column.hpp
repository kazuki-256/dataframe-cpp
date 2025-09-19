#pragma once

#include "object.hpp"
#include "object_iterator.hpp"
#include <list>




class df_column_t {
protected:
    struct mutex_t {
        long start = 0, end = -1;
        bool is_using = false;
        bool is_write = false;
    };

    class shared_lock_t {
        mutex_t* _mutex = NULL;
    public:
        ~shared_lock_t();

        shared_lock_t();
        shared_lock_t(std::list<mutex_t>& mutexs);
    };

    class unique_lock_t {
        mutex_t* _mutex = NULL;
    public:
        ~unique_lock_t();

        unique_lock_t();
        unique_lock_t(std::list<mutex_t>& mutexs);
    };

    struct _column_t {
        df_type_t data_type = DF_TYPE_UINT8;
        int size_per_data = 1;
        df_value_init_callback_t type_initer = NULL;
        df_value_load_callback_t type_loader = NULL;

        uint8_t* values = NULL;
        bool*    nulls  = NULL;

        long length = 0;
        long capacity = 0;


        std::string label = "";

        mutable std::list<mutex_t> mutexs;
        mutable int use_count = 0;

        bool can_read = true;
        bool can_write = true;
    };



    // == variables ==

    _column_t* _column;




    // == private init ==
    
    void _init(df_type_t data_type, long start_capacity = 0);
    
    template<typename T>
    void _init_typed_value(df_type_t data_type, const std::initializer_list<T>& sources);


    // == private destroy / copy / move ==

    void _destroy() noexcept;

    void _copy(const df_column_t& other) noexcept;

    void _move(const df_column_t& other) noexcept;


    // == private lock ==

    shared_lock_t _shared_lock(long start, long end) const;

    unique_lock_t _unique_lock(long start, long end) const;

public:
    // == destroy ==

    ~df_column_t() noexcept;


    // == init ==

    df_column_t(df_type_t data_type, long start_capacity = 0);

    df_column_t(const std::initializer_list<df_object_t>& objects);

    df_column_t(const df_object_range_t& range);



    // == copy ==

    df_column_t(const df_column_t& other) noexcept;

    df_column_t& operator=(const df_column_t& other) noexcept;


    // == move ==

    df_column_t(const df_column_t&& other) noexcept;

    df_column_t& operator=(const df_column_t&& other) noexcept;



    // == iterator ==

    df_memory_iterator_t memory_begin();

    df_memory_iterator_t memory_end();

    df_object_iterator_t begin();

    df_object_iterator_t end();


    df_const_memory_iterator_t memory_begin() const;

    df_const_memory_iterator_t memory_end() const;


    df_const_object_iterator_t begin() const;

    df_const_object_iterator_t end() const;



    // == range ==

    df_object_range_t range(long start = 0, long end = -1, long interval = 1);
    
    df_object_range_t range(long start = 0, long end = -1, long interval = 1) const;



    // == get ==

    inline df_type_t get_data_type() const;

    inline long get_length() const;

    inline const std::string& get_label() const;



    df_object_t operator[](long index);



    // == append / extend / merged ==

    df_column_t& append(const df_object_t& object);



    df_column_t& extend(const std::initializer_list<df_object_t>& objects);

    df_column_t& extend(const std::vector<df_object_t>& objects);

    df_column_t& extend(const df_object_range_t& objects);

    df_column_t& extend(const df_column_t& objects);

    
    
    df_column_t merged(const std::initializer_list<df_object_t>& objects) const;

    df_column_t merged(const std::vector<df_object_t>& objects) const;

    df_column_t merged(const df_column_t& other) const;



    // == set ==

    df_column_t& set_label(const std::string& label);



    // == convert ==

    template<typename DEST> operator std::vector<DEST>() const;


    // == print ==

    std::ostream& write_stream(std::ostream& os, const char* column_name) const;

    friend std::ostream& operator<<(std::ostream& os, const df_column_t& column);



    // == SQL ==

    df_column_t(const df_query_t& query);

    df_query_t as(const char* name) const;



    // == vector ==

    friend df_column_t df_range_uint8(uint8_t, uint8_t, uint8_t);
    friend df_column_t df_range_int16(short, short, short);
    friend df_column_t df_range_int32(int, int, int);
    friend df_column_t df_range_int64(long, long, long);

    friend df_column_t df_range_date(df_date_t, df_date_t, df_interval_t);
    friend df_column_t df_range_time(df_date_t, df_date_t, df_interval_t);
    friend df_column_t df_range_datetime(df_date_t, df_date_t, df_interval_t);


    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator+(long num) const;
    df_query_t operator+(double num) const;

    df_query_t operator-(const df_query_t& query) const;
    df_query_t operator-(long num) const;
    df_query_t operator-(double num) const;

    df_query_t operator*(const df_query_t& query) const;
    df_query_t operator*(long num) const;
    df_query_t operator*(double num) const;

    df_query_t operator/(const df_query_t& query) const;
    df_query_t operator/(long num) const;
    df_query_t operator/(double num) const;

    df_query_t operator%(const df_query_t& query) const;
    df_query_t operator%(long num) const;
    df_query_t operator%(double num) const;
};