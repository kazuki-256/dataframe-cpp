#pragma once

#include "../config.hpp"
#include "object.hpp"

#include <list>
#include <set>



class df_column_t {
protected:
    df_type_t data_type = DF_TYPE_UINT8;
    int size_per_data = 1;

    uint8_t* values = NULL;
    bool*    nulls  = NULL;

    long length = 0;
    long capacity = 0;


    // == private make ==
    
    inline void basic_init(df_type_t data_type, long start_capacity);
    
    template<typename DEST, typename SRC>
    inline void typed_init(df_type_t data_type, const std::initializer_list<SRC>& const_list);



    // == private destroy / copy / move ==

    inline void destroy() noexcept;

    inline void move(df_column_t& src) noexcept;


    // == private extend ==

    template<typename T> inline df_column_t& basic_extend(
        T& objects, df_value_init_callback_t initer, df_value_write_callback_t writer
    );


    inline void basic_extend_values(
        uint8_t* src_value_ptr, bool* src_null_ptr, bool* src_null_end,
        df_type_t src_type, int size_per_data,
        df_value_init_callback_t initer, df_value_load_callback_t loader, df_value_write_callback_t writer
    );


public:
    // == destroy ==

    ~df_column_t();



    // == make ==

    df_column_t(df_type_t data_type, long start_capacity);

    df_column_t(const std::initializer_list<df_object_t>& objects);



    // == copy ==

    df_column_t(const df_column_t& src);

    // df_column_t operator=(const df_column_t& src);


    // == move ==

    df_column_t(df_column_t&& src) noexcept;

    df_column_t& operator=(df_column_t&& src) noexcept;



    // == get ==

    inline df_type_t get_data_type() const;

    inline long get_length() const;



    df_object_t operator[](long index) const;

    const df_object_t operator[](long index);



    // == set ==

    // df_column_t& set_foregin(df_column_t& column);       // wait for make


    // sure n object size
    // 0 if success, !0 if failure
    int reserve(const long n);
    

    // == append / extend / merged ==

    df_column_t& append(const df_object_t& object);



    df_column_t& extend(const std::initializer_list<df_object_t>& objects);

    df_column_t& extend(const std::vector<df_object_t>& objects);

    df_column_t& extend(const df_column_t& objects);

    
    
    df_column_t merged(const std::initializer_list<df_object_t>& objects) const;

    df_column_t merged(const std::vector<df_object_t>& objects) const;

    df_column_t merged(const df_column_t& other) const;



    // == convert ==

    template<typename DEST> operator std::vector<DEST>() const;



    // == iterator ==

    class memory_iterator_t;
    class const_memory_iterator_t;

    class object_iterator_t;
    class const_object_iterator_t;


    memory_iterator_t memory_begin();

    memory_iterator_t memory_end();

    const_memory_iterator_t memory_begin() const;

    const_memory_iterator_t memory_end() const;



    object_iterator_t begin();

    object_iterator_t end();

    const_object_iterator_t begin() const;

    const_object_iterator_t end() const;



    // == print ==

    std::ostream& write_stream(std::ostream& os, const char* column_name) const;

    friend std::ostream& operator<<(std::ostream& os, const df_column_t& column);


    // == SQL ==

    df_column_t(const df_query_t& query);

    df_query_t as(const char* name) const;


    // == vector ==

    df_query_t operator+(long num) const;
    df_query_t operator+(double num) const;

    df_query_t operator-(long num) const;
    df_query_t operator-(double num) const;

    df_query_t operator*(long num) const;
    df_query_t operator*(double num) const;

    df_query_t operator/(long num) const;
    df_query_t operator/(double num) const;

    df_query_t operator%(long num) const;
    df_query_t operator%(double num) const;
};




std::ostream& operator<<(std::ostream& os, const df_named_column_t& named_column);





class df_column_uint8_t : public df_column_t {
public:
    df_column_uint8_t(const std::initializer_list<uint8_t>& sources);
}


class df_column_int16_t : public df_column_t {
public:
    df_column_int16_t(const std::initializer_list<short>& sources);
}


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
