# dataframe-cpp (under developmenting)

## info

| programmer  | かずき256      |
| :---------- | :------------- |
| version     | beta 1.0.0     |
| start date  | 2025/08/14     |
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
#include "types/column.hpp"
#include <iostream>

int main(int argc, char** argv) {
    // I will rename it to df_range_<type>()
    df_column_t date = df_column_t::range_date("2000-1-1", "2025-1-1", "1 years");
    df_column_t index = df_column_t::range_int32(0, 25);

    

    std::cout << column << "\n";
    return 0;
}

```

## **Features**

1. read/write dataframe from/to .csv, .xlsx, .html, .db
2. vector operation
3. SQL execution
4. OpenCV::RandomForest
5. host SQL server
6. gpu acceleration by libcuDL

## depends (no anything depending now)

1. Freexl (to read .xlsx)
2. xlsxio_write (to write .xlsx)
3. OpenCV (if you want to use RandomForest, ploting)
4. Sqlite3 (to read/write .db)



---

## **Documents**

### **dataframe.cpp**


**sample**

```cpp
// == create df ==
df_dataframe_t df = {
    // typed column having faster init, but not supporting null
    // typed column same to normal column but having different init method
    {"id", df_column_int32_t{1, 2, 3, 4, 5}},
    {"name", df_column_text_t{"kazuki", "B", "C", "D", "F"}}
};

// == print df information ==

std::cout << "column count: " << df.get_column_count() << "\n";
std::cout << "row count: " << df.get_row_count() << "\n";

// == print df ==
std::cout << "df before modified\n";
std::cout << df << "\n";


// == edit data ==
// get data targeter from column "name" second  data
// you can skip storing df_object_t to `df["name"][1] = value`
df_object_t object = df["name"][1];


// modify to "hello world"
object = "hello world";

// print object
std::cout << "df[\"name\"][1] modified to " << object << "\n";


// == print df modified ==
std::cout << "df after modified\n";
std::cout << df << "\n";

```


**classes**

df_dataframe_t:
- df_dataframe_t(const_list_pair_name_column);

- df_dataframe_t(const_copy_dataframe);
- df_dataframe_t(move_dataframe);
- df_dataframe_t& operator=(const_copy_dataframe);
- df_dataframe_t& operator=(move_dataframe);

- int get_column_count() const;
- long get_row_count() const;

- for (pair_name_column : dataframe);
- df_column_t& operator[](column_name);
- df_row_t loc(row_index);

- std::ostream& write_ostream(ostream) const;
- static std::ostream& operator<<(ostream, const_dataframe);


df_column_t:
- df_column_t(const_list_object, capacity = 4096);
- df_column_t(type, capacity = 4096);

- df_column_t(const_copy_column);
- df_column_t(move_colunn);
- df_column_t& operator=(const_copy_column);
- df_column_t& operator=(move_colunn);

- long get_length() const;

- for (object : column);
- df_object_t operator[](data_index);

- std::ostream& write_ostream(ostream, column_name) const;
- static ostream& operator<<(ostream, const_column_name);


df_object_t:
- template<typename T> df_object_t(const_T_value);

- df_object_t(const_copy_object) = delete;
- df_object_t(move_object);
- df_object_t operator=(const_copy_object);
- df_object_t operator=(move_object);

- bool is_null() const;
- bool is_locked() const;

- template<typename T> operator T() const;
- template<typename T> operator=(const_T_value);
- template<typename T> operator<<(const_T_value);

- ostream std::ostream operator<<(ostream, const_object);


df_date_t:
- df_date_t(t = 0);
- df_date_t(const_char_to_date);
- operator time_t() const;
- operator std::string() const;
- int parse(const_char_to_date);
- df_date_t& operator+(time_interval)

df_interval_t:
- int year, month, day, hour, minute, second;
- df_interval_t(const_char_to_time_format, ...);
- time_t to_const_value() const;
- bool is_const_value() const;
- operator std::string() const;



### **vector.cpp**

methods:
- df_range_...(start, end, interval);
- df_randrange_...(min, max, length);
- df_column_t + value, df_column_t - value, ...
- df_column_t + df_column_t, df_column_t - df_column_t, ...


### **query.cpp**

methods in column_t:
- df_query_t df_column_t::to_query(name);

methods in dataframe_t:
- df_query_t to_query(name);
- df_query_t select(sql);
- df_query_t where(sql);
- df_query_t join(dataframe, as_name, where_sql);
- df_query_t group_by(column_name);
- df_query_t order_by(column_name, 1 || -1);

methods in query_t:
- operator df_column_t();
- operator df_dataframe_t();
- df_query_t& select(sql);
- df_query_t& where(sql);
- df_query_t& join(dataframe, as_name, where_sql);
- df_query_t& group_by(column_name);
- df_query_t& order_by(column_name, 1 || -1);



---

## **Logs**

- 2025-09-04:
  - コード総行数3000突破
  - df_column_t::range_datetime() 高速化
  - df_column_t::begin() + index が可能にする
  - バグ修正

- 2025-09-02:
  - df_column_t の継承クラス作成
  - df_column_t::range_<type>() 追加

- 2025-08-23:
  - types/byte.hpp から types/mem.hpp に変更、void* を通して汎用処理をするシステムにした

- 2025-08-20:
  - テンプレート型構造を破棄
  - コールバックテーブルを使用した types/byte.hpp データ処理コアを作成

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
