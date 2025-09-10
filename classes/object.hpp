/* df_object_t: a data targetor to edit data

features:
1. owns data or target to column data
2. constant data type, not allowed to change type after create
3. edit targeted data
4. convert data
5. std::cout support

*/

#pragma once


#include "../config.hpp"
#include "value.hpp"

#include <vector>





class df_null_t {};

constexpr df_null_t DF_NULL;




class df_object_t {
    friend class df_mem_block_t;
    friend class df_object_iterator_t;
    friend class df_const_object_iterator_t;
    friend class df_column_t;

    std::vector<std::string>* category_titles = NULL;

    bool*    target_null     = NULL;
    uint8_t*    target_value     = NULL;
    df_value_t  target_preload;
    df_type_t   target_type     = DF_TYPE_UINT8;

    bool        lock_state  = false;

    uint8_t buffer[DF_MAX_TYPE_SIZE] = {};



    void destroy();


    constexpr df_object_t();

    void init_as_local();   // basic init of free object


    void copy(const df_object_t& other) noexcept;
    
    void move(df_object_t& other) noexcept;



    void set_target(bool* target_null, uint8_t* target_value, df_value_load_callback_t loader);

    void lock();


    inline void basic_set(df_type_t dest_type);

public:
    // == destroy ==

    ~df_object_t();


    // == init ==

    template<typename T> df_object_t(const T& const_value);

    df_object_t(const char* const_string);
    df_object_t(std::string&& other);

    df_object_t(df_null_t);

    df_object_t(const char*&) = delete;



    // == copy ==

    df_object_t(const df_object_t&) noexcept;

    df_object_t& operator=(const df_object_t& other);



    // == move ==

    df_object_t(df_object_t&& other) noexcept;

    df_object_t& operator=(df_object_t&& other);



    // == set ==

    template<typename T> df_object_t& operator=(const T src);

    df_object_t& operator=(const char* src);

    df_object_t& operator=(df_null_t);


    df_object_t& operator=(const char*&) = delete;
    df_object_t& operator=(uint8_t) = delete;
    df_object_t& operator=(short) = delete;



    template<typename T> df_object_t& operator<<(const T src);

    df_object_t& operator<<(const char* src);

    df_object_t& operator<<(const df_object_t& src);
    
    df_object_t& operator<<(df_null_t);


    df_object_t& operator<<(const char*& src) = delete;




    // == convert ==

    template<typename T> operator T() const;



    // == check state ==

    inline bool is_locked() const ;

    inline bool is_proxy() const;

    inline bool is_variant() const;

    inline bool is_null() const;
    

    inline df_type_t get_type() const;


    // == print ==

    inline std::string to_string() const;

    friend inline std::ostream& operator<<(std::ostream& stream, const df_object_t& object);
};






