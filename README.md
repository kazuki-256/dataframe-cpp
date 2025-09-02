# dataframe-cpp (under developmenting)

## info

| programmer  | かずき256      |
| :---------- | :------------- |
| version     | beta 1.0.0     |
| last update | 2025/08/14     |
| state       | developmenting |

## init

This data frame API is for data science in c++, target at readable, high performance and multi-functions.

The API will store the data handles steps to df_process and execute data processs when df_process convert to iterator, df_column or df_data_frame. it would support sql commands, vector operation and data science method (min(), max(), median(), etc)

Currently, this API stills in making the df_data_frame object, but the main.cpp shows over my excepted cleanly.

## Sample

**excepted**

```cpp
#include "dataframe-cpp/dataframe.hpp"


int main(int argc, char** argv) {
  // == read data ==
  df_dataframe_t staff = df_read_csv("staffs.csv");
  df_dataframe_t job = df_read_csv("job.csv");

  // == data processing ==

  df_dataframe_t df1 = staff.as("staff")      // before to write SQL, you can give your table a name, or start without name (column name only)
          .select("staff.id, staff.name, job.title, job.hourly * staff.worked AS salary")    // select data, just like SQL
           .join(job, "job", "staff.job_id = job.id")      // join other table as "job" by same job id
           .where("salary > 100000")                       // filter
           .order_by("salary", -1);                        // sort the output by desc 

  df_dataframe_t df2 = staff.as("staff")
           .select("job.job_title, AVG(staff.worked * job.hourly), count")
           .join(job, "job", "staff.job_id == job.id")
           .group_by("job.title")                             // group by job.title
           .mutate("COUNT(*) AS count");                      // also, you can define variables by mutate

  // == vector processing ==
  df_column_t yearly = df1["salary"] * 12;
  df_column_t staff_worked = df1["salary"] / df1["job.hourly"];

  // == print ==
  std::cout << df1 << "\n";
  std::cout << df2 << "\n";

  // == output ==
  df_write_csv(df1, "staff_salary.csv");
  df_write_db(df2, "job_info.csv");

  return 0;
}
```

**currently**

```cpp
include "dataframe_cpp/types/column.hpp"

int main(int argc, char** argv) {
    df_column_text_t numbers = {"hello world", "yoshihara kazuki"};
  
    std::cout << numbers << "\n";
    return 0;
}
```

## Features

1. read/write data frame from/to .csv, .xlsx, .html, .db
2. vector operation
3. SQL execution
4. OpenCV::RandomForest
5. SocketSQL to host SQL server

## depends (no anything depending now)

1. Freexl (to read .xlsx)
2. xlsxio_write (to write .xlsx)
3. OpenCV (if you want to use RandomForest, ploting)
4. Sqlite3 (to read/write .db)

## **Classes**

### **df_date_t**

Encapsulation of c `time_t`

```cpp
class df_date_t {
  df_date_t(const char* strdate, const char* datefmt = DEFAULT);

  operator time_t();
  df_date_t& operator+(df_interval_t& interval);

  const char* c_str(const char* datefmt = DEFAULT, char* buffer = DEFAULT, size_t buffer_size = DEFAULT) const;
};
```

### **df_interval_t**

structure to offset df_date_t

```cpp
class df_interval_t {
  int years, months, days;
  int hours, mintues, seconds;

  df_interval_t(const char* fmt, ...);

  const char* c_str(const char* strdate = DEFAULT, char* buffer = DEFAULT, size_t buffer_size = DEFAULT) const;
};
```

### **df_object_t**

object targeter to edit data, create own data or get `df_object_t` from `column_t`, `dataframe_t` or `query_t`

```cpp
class df_object_t {
  df_object(type_t type);

  template<typename T> df_object_t& operator=(const T& value);
  template<typename T> df_object_t& operator!=(const T& other);

  template<typename T> operator T() const;

  std::string& to_string() const;
  friend ostream& operator<<() const;    // std::cout << object;
};
```

### **df_row_t**

row targeter to edit row objects

```cpp
class df_row_t {
  df_object_t& operator[](const char* column_name);

  int get_column_count() const;

  iterator begin();    // for (df_object_t& object : row);
  iterator end();

  std::string& to_string() const;
  friend ostream& operator<<() const;    // std::cout << row;
};
```

### **df_column_t<df_type_t>**

```cpp
template<df_type_t TYPE = DF_UNDEFINED>  // for init column, not different in use
class df_column_t {
  df_column_t(const std::vector<df_raw_t<TYPE>>& raws);

  df_object_t& operator[](int index);
  int get_length() const;

  iterator begin();    // for (df_object_t& object : column)
  iterator end();

  std::string to_string() const;
  friend ostream& operator<<() const;    // std::cout << column;
};
```

### **df_dataframe_t**

dataframe object

```cpp
class df_dataframe_t {
  df_dataframe_t(const std::vector<std::pair<std::string, df_column<DF_UNDEFINED>>>& raws);

  df_column& operator[](const char* name);
  df_row& loc(int index);

  int get_column_count() const;
  int get_row_count() const;

  iterator begin();    // for (df_row_t& row : dataframe)
  iterator end();

  std::string to_string() const;
  friend ostream& operator<<() const;    // std::cout << column;

  // == sql ==
  df_query_t as(const char* name) const;
  df_query_t select(const char* sql) const;
  // ...
};
```

### **df_query_t**

query to process data by sql commands or vector operation (data is not handle by sql)

```cpp
class df_query_t {
  // == sql commands ==
  df_query_t& select(const char* sql);
  df_query_t& where(const char* sql);
  df_query_t& mutatue(const char* sql);
  df_query_t& join(df_dataframe_t& df, const char* as_name, const char* filter_sql);
  df_query_t& group_by(const char* column_name);
  df_query_t& order_by(const char* column_name, int desc = DF_ASC);
  
  // == vector operation ==
  df_query& operator+(double val);
  df_query& operator-(double val);
  // ...

  // == execute/convert ==
  operator df_dataframe_t() const;
  operator df_column_t<DF_UNDEFINED>() const;

  iterator begin() const;    // for (df_row_t& row : query)
  iterator end() const;
};
```

## Logs

- 2025-08-17:

  - fix `df_column<>` memory problem
  - add `df_object<df_category>`
  - add df_column and df_data_frame `std::cout` support
- 2025-08-16:

  - rewrite `df_object`, `df_object_chunk`, `df_column`
  - change naming style from `PascalCase` to `snake_case`
  - some memory fix
  - update README.md
- 2025-08-15:

  - arrange files place
  - start making DfDataFrame
- 2025-08-14:

  - update README.md
