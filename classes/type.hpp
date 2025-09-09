#pragma once

constexpr int DF_SIZE_SHIFT = 0x3f;
constexpr int DF_TYPE_SHIFT = 6;
constexpr int DF_TYPE_COUNT = 14;

constexpr int DF_MAX_TYPE_SIZE = 32;


typedef enum df_typeid_t {
  DF_TYPEID_POINTER,

  DF_TYPEID_UINT8,
  DF_TYPEID_INT16,
  DF_TYPEID_INT32,
  DF_TYPEID_INT64,

  DF_TYPEID_FLOAT32,
  DF_TYPEID_FLOAT64,
  
  DF_TYPEID_TEXT,
  DF_TYPEID_CATEGORY,

  DF_TYPEID_DATE,
  DF_TYPEID_TIME,
  DF_TYPEID_DATETIME,
  DF_TYPEID_INTERVAL,

  DF_TYPEID_BOOL,
} df_typeid_t;


#define DF_TYPESIZE_NULL 1
#define DF_TYPESIZE_POINTER 8

#define DF_TYPESIZE_UINT8 1
#define DF_TYPESIZE_INT16 2
#define DF_TYPESIZE_INT32 4
#define DF_TYPESIZE_INT64 8

#define DF_TYPESIZE_FLOAT32 4
#define DF_TYPESIZE_FLOAT64 8

#define DF_TYPESIZE_TEXT 32
#define DF_TYPESIZE_CATEGORY 4

#define DF_TYPESIZE_DATETIME 8
#define DF_TYPESIZE_INTERVAL 24

#define DF_TYPESIZE_BOOL 1



typedef enum df_type_t {
  DF_TYPE_POINTER = DF_TYPEID_POINTER << DF_TYPE_SHIFT | DF_TYPESIZE_POINTER,    // void*

  DF_TYPE_UINT8 = DF_TYPEID_UINT8 << DF_TYPE_SHIFT | DF_TYPESIZE_UINT8,        // uint8_t
  DF_TYPE_INT16 = DF_TYPEID_INT16 << DF_TYPE_SHIFT | DF_TYPESIZE_INT16,        // short
  DF_TYPE_INT32 = DF_TYPEID_INT32 << DF_TYPE_SHIFT | DF_TYPESIZE_INT32,        // int
  DF_TYPE_INT64 = DF_TYPEID_INT64 << DF_TYPE_SHIFT | DF_TYPESIZE_INT64,        // long

  DF_TYPE_FLOAT32 = DF_TYPEID_FLOAT32 << DF_TYPE_SHIFT | DF_TYPESIZE_FLOAT32,    // float
  DF_TYPE_FLOAT64 = DF_TYPEID_FLOAT64 << DF_TYPE_SHIFT | DF_TYPESIZE_FLOAT64,    // double

  DF_TYPE_TEXT = DF_TYPEID_TEXT << DF_TYPE_SHIFT | DF_TYPESIZE_TEXT,         // std::optional<std::string>
  DF_TYPE_CATEGORY = DF_TYPEID_CATEGORY << DF_TYPE_SHIFT | DF_TYPESIZE_CATEGORY,  // int

  DF_TYPE_DATE = DF_TYPEID_DATE << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,          // df_date_t
  DF_TYPE_TIME = DF_TYPEID_TIME << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,          // df_date_t
  DF_TYPE_DATETIME = DF_TYPEID_DATETIME << DF_TYPE_SHIFT | DF_TYPESIZE_DATETIME,  // df_date_t
  DF_TYPE_INTERVAL = DF_TYPEID_INTERVAL << DF_TYPE_SHIFT | DF_TYPESIZE_INTERVAL, // df_interval_t

  DF_TYPE_BOOL = DF_TYPEID_BOOL << DF_TYPE_SHIFT | DF_TYPESIZE_BOOL,          // bool


  // == alias ==

  DF_TYPE_SHORT = DF_TYPE_INT16,
  DF_TYPE_INT = DF_TYPE_INT32,
  DF_TYPE_LONG = DF_TYPE_INT64,

  DF_TYPE_STRING = DF_TYPE_TEXT,
} df_type_t;


template<typename T>
constexpr df_type_t df_type_get_type =
    std::is_same_v<T, std::string> ? DF_TYPE_TEXT
    : std::is_same_v<T, const char*> ? DF_TYPE_TEXT
    : std::is_same_v<T, char*> ? DF_TYPE_TEXT

    : std::is_pointer_v<T> ? DF_TYPE_POINTER

    : std::is_same_v<T, uint8_t> ? DF_TYPE_UINT8
    : std::is_same_v<T, short> ? DF_TYPE_INT16
    : std::is_same_v<T, int> ? DF_TYPE_INT32
    : std::is_same_v<T, long> ? DF_TYPE_INT64

    : std::is_same_v<T, float> ? DF_TYPE_FLOAT32
    : std::is_same_v<T, double> ? DF_TYPE_FLOAT64

    : std::is_same_v<T, df_date_t> ? DF_TYPE_DATETIME

    : std::is_same_v<T, bool> ? DF_TYPE_BOOL
    : DF_TYPE_UINT8;




constexpr inline bool df_type_is_struct(df_type_t type) {
  return type == DF_TYPE_TEXT;
}

constexpr inline int df_type_get_typeid(df_type_t type) {
  return type >> DF_TYPE_SHIFT;
}

constexpr inline bool df_type_is_number(df_type_t type) {
  int type_id = df_type_get_typeid(type);
  return (type_id >= DF_TYPEID_UINT8 && type_id <= DF_TYPEID_FLOAT64) || type == DF_TYPE_BOOL;
}

constexpr inline int df_type_get_size(df_type_t type) {
  return type & DF_SIZE_SHIFT;
}


constexpr const char DF_TYPE_NAMES[DF_TYPE_COUNT][9] = {
  "POINTER",
  "UINT8", "SHORT", "INT", "LONG",
  "FLOAT", "DOUBLE",
  "TEXT", "CATEGORY",
  "DATE", "TIME", "DATETIME", "INTERVAL",
  "BOOL"
};

constexpr const char* df_typeid_get_string(int type_id) {
  return type_id < DF_TYPE_COUNT ? DF_TYPE_NAMES[type_id] : "INVALID_TYPE";
}

constexpr const char* df_type_get_string(df_type_t type) {
  return df_typeid_get_string(df_type_get_typeid(type));
}

