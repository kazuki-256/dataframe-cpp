#pragma once

#include "../config.hpp"
#include "object.hpp"
#include "object_range.hpp"

#include <list>
#include <set>


class df_column_t {
    friend class df_dataframe_t;
    friend class std::pair<std::string, df_column_t>;

    friend class df_memory_iterator_t;
    friend class df_const_memory_iterator_t;
    friend class df_object_iterator_t;
    friend class df_const_object_iterator_t;

    friend class df_row_t;
    friend class df_const_row_t;

    friend class df_row_range_t;
    friend class df_const_row_range_t;
    friend class df_object_range_t;
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


    df_type_t data_type = DF_TYPE_UINT8;
    int size_per_data = 1;
    df_value_init_callback_t type_initer;
    df_value_load_callback_t type_loader;

    uint8_t* values = NULL;
    bool*    nulls  = NULL;

    long length = 0;
    long capacity = 0;


    std::string label = "";

    bool can_read = true;
    bool can_write = true;

    mutable std::list<mutex_t> mutexs;



    // == private make ==
    
    inline void _init(df_type_t data_type, long length, long start_capacity);
    
    template<typename T>
    inline void _init_typed_value(df_type_t data_type, const std::initializer_list<T>& sources);


    df_column_t();


    // == private destroy / copy / move ==

    inline void destroy() noexcept;

    inline void move(df_column_t& src) noexcept;


    // == private extend ==

    // for initial_list<T> or vector<T>
    // ! no length checking
    template<typename T> inline df_column_t& _extend_values(T& objects);


    // for df_column_t
    // ! no length checking
    inline void _extend_column(const df_column_t& src);


    // == delay system ==

    mutex_t* _get_shared_mutex(long start, long end) {
        mutex_t* reuse_ptr = NULL;
        for (auto& mutex : mutexs) {
            if (mutex.is_using) {
                reuse_ptr = &mutex;
                continue;
            }

            if (!(end < mutex.start || start > mutex.end)) {
                mutex.start = DF_MIN(mutex.start, start);
                mutex.end = DF_MAX(mutex.end, end);
                return &mutex;
            }
        }
        if (reuse_ptr) {
            reuse_ptr->start = start;
            reuse_ptr->end = end;
            return reuse_ptr;
        }
        return &mutexs.emplace_back(start, end, false, false);
    }


public:
    // == destroy ==

    ~df_column_t();



    // == make ==

    // constexpr df_column_t();

    df_column_t(df_type_t data_type, long start_capacity);

    df_column_t(const std::initializer_list<df_object_t>& objects);

    df_column_t(const df_object_range_t& range);



    // == copy ==

    df_column_t(const df_column_t& src) noexcept;

    df_column_t& operator=(const df_column_t& src) noexcept;


    // == move ==

    df_column_t(df_column_t&& src) noexcept;

    df_column_t& operator=(df_column_t&& src) noexcept;



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
    
    const df_object_range_t range(long start = 0, long end = -1, long interval = 1) const;




    // == check state ==

    bool is_delaying_read() const;

    bool is_delaying_write() const;


    bool is_readable() const;

    bool is_writable() const;

    bool is_deletable() const;


    // == get ==

    inline df_type_t get_data_type() const;

    inline long get_length() const;



    df_object_t operator[](long index);

    const df_object_t operator[](long index) const;



    // == set ==

    df_column_t& set_label(const char* label);

    // df_column_t& set_foregin(df_column_t& column);       // wait for make


    // sure n object size
    // @return >0 when realloced, =0 when nothing, <0 when failure
    int reserve(const long n);
    

    // == append / extend / merged ==

    df_column_t& append(const df_object_t& object);



    df_column_t& extend(const std::initializer_list<df_object_t>& objects);

    df_column_t& extend(const std::vector<df_object_t>& objects);

    df_column_t& extend(const df_column_t& objects);

    df_column_t& extend(const df_object_range_t& objects);

    
    
    df_column_t merged(const std::initializer_list<df_object_t>& objects) const;

    df_column_t merged(const std::vector<df_object_t>& objects) const;

    df_column_t merged(const df_column_t& other) const;



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





// == typed_column_t ==


class df_column_uint8_t : public df_column_t {
public:
    df_column_uint8_t(const std::initializer_list<uint8_t>& sources);
};


class df_column_int16_t : public df_column_t {
public:
    df_column_int16_t(const std::initializer_list<short>& sources);
};


class df_column_int32_t : public df_column_t {
public:
    df_column_int32_t(const std::initializer_list<int>& sources);
};


class df_column_int64_t : public df_column_t {
public:
    df_column_int64_t(const std::initializer_list<long>& sources);
};



class df_column_float32_t : public df_column_t {
public:
    df_column_float32_t(const std::initializer_list<float>& sources);
};

class df_column_float64_t : public df_column_t {
public:
    df_column_float64_t(const std::initializer_list<double>& sources);
};



class df_column_text_t : public df_column_t {
public:
    df_column_text_t(const std::initializer_list<const char*>& sources);

    df_column_text_t(const std::initializer_list<std::string>& sources);
};



class df_column_date_t : public df_column_t {
public:
    df_column_date_t(const std::initializer_list<df_date_t>& sources);
};

class df_column_time_t : public df_column_t {
public:
    df_column_time_t(const std::initializer_list<const char*>& sources);
};

class df_column_datetime_t : public df_column_t {
public:
    df_column_datetime_t(const std::initializer_list<df_date_t>& sources);
};
