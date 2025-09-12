#pragma once

#include "../config.hpp"
#include "object.hpp"

#include <list>
#include <set>


class df_column_t {
    friend class df_dataframe_t;
    friend class std::pair<std::string, df_column_t>;

    friend class df_row_t;
    friend class df_const_row_t;

    friend class df_range_rows_t;
    friend class df_const_range_rows_t;
protected:
    df_type_t data_type = DF_TYPE_UINT8;
    int size_per_data = 1;
    df_value_init_callback_t type_initer;
    df_value_load_callback_t type_loader;

    uint8_t* values = NULL;
    bool*    nulls  = NULL;

    long length = 0;
    long capacity = 0;


    // == private make ==
    
    inline void basic_init(df_type_t data_type, long length, long start_capacity);
    
    template<typename T>
    inline void typed_init_no_init(df_type_t data_type, const std::initializer_list<T>& sources);


    df_column_t();


    // == private destroy / copy / move ==

    inline void destroy() noexcept;

    inline void move(df_column_t& src) noexcept;


    // == private extend ==

    // for initial_list<T> or vector<T>
    // ! no length checking
    template<typename T> inline df_column_t& basic_extend_values(T& objects);


    // for df_column_t
    // ! no length checking
    inline void basic_extend_column(const df_column_t& src);


public:
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



    // == destroy ==

    ~df_column_t();



    // == make ==

    // constexpr df_column_t();

    df_column_t(df_type_t data_type, long start_capacity);

    df_column_t(const std::initializer_list<df_object_t>& objects);



    // == copy ==

    df_column_t(const df_column_t& src) noexcept;

    df_column_t& operator=(const df_column_t& src) noexcept;


    // == move ==

    df_column_t(df_column_t&& src) noexcept;

    df_column_t& operator=(df_column_t&& src) noexcept;



    // == get ==

    inline df_type_t get_data_type() const;

    inline long get_length() const;



    df_object_t operator[](long index);

    const df_object_t operator[](long index) const;



    // == set ==

    // df_column_t& set_foregin(df_column_t& column);       // wait for make


    // sure n object size
    // @return >0 when realloced, =0 when nothing, <0 when failure
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

    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator-(long num) const;
    df_query_t operator-(double num) const;

    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator*(long num) const;
    df_query_t operator*(double num) const;

    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator/(long num) const;
    df_query_t operator/(double num) const;

    df_query_t operator+(const df_query_t& query) const;
    df_query_t operator%(long num) const;
    df_query_t operator%(double num) const;
};




// == named_column_t ==

std::ostream& operator<<(std::ostream& os, const df_named_column_t& named_column);






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
