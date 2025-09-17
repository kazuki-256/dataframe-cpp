# dataframe-cpp (under developmenting)

## info

| programmer | かずき256      |
| :--------- | :------------- |
| version    | beta 1.0.0     |
| start date | 2025/08/14     |
| state      | developmenting |

## init

This data frame API is for data science in c++, target at readable, high performance and multi-functions.

The API will store the data handles steps to df_process and execute data processs when df_process convert to iterator, df_column or df_data_frame. it would support sql commands, vector operation and data science method (min(), max(), median(), etc)

Currently, this API stills in making the df_data_frame object, but the main.cpp shows over my excepted cleanly.

## Features

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

## Sample

**excepted**

```cpp
#include "dataframe_cpp/dataframe.cpp"
#include "dataframe_cpp/query.cpp"
#include "dataframe_cpp/read_csv.cpp"
#include "dataframe_cpp/write_csv.cpp"


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
           .variable("COUNT(*) AS count");                      // also, you can define variables by variable

  // == vector processing ==
  df_column_t yearly = df1["salary"] * 12;
  df_column_t staff_worked = df1["salary"] / df1["job.hourly"];

  // == print ==
  std::cout << df1 << "\n";
  std::cout << df2 << "\n";

  // == output ==
  df_write_csv(df1, "staff_salary.csv");
  df_write_csv(df2, "job_info.csv");

  return 0;
}
```

**currently**

```cpp
#include "dataframe_cpp/classes/dataframe.cpp"

int main(int argc, char** argv) {
	df_dataframe_t df = {
		{"index", df_range_int32(1, 6)},	// INT32[5] of {1, 2, 3, 4, 5}
		{"date", df_range_date("2000-1-1", "2025-1-1", "1 years")},	// DATE[5] of {2000-01-01 00:00, ...}
		{"text", df_column_text_t{"kazuki", "dataframe", "c++", "minecraft", "hello world"}},	// typed-column, having faster construct
		{"pi", {3.14, 314, "3.14", 3.1415f, 3.1415}}	// default consturct able to use any type if able to convert, and typed by first object type
	};

	// print dataframe
	std::cout << " === Data Frame ===\n";
	std::cout << df << "\n";

	// set ["text"][4] as text, << will cast any type to dest type
	df["text"][4] << "next";

	// range each row
	for (df_row_t& row : df.range_rows()) {		// df_row_t is a lazy row, it wouldn't iterate useless columns
		std::cout << row["date"] << "   " << row["text"] << "\n";

		std::cout << row << "\n";	// print row
	}

	// get row at 4
	df.row(4)["date"] << "2025-09-10 12:00";	// set ["date"][4] as 2025-09-10 12:00 (auto convert to its type: DATE);

	// print dataframe again
	std::cout << df << "\n";
	return 0;
}

```

---

## Documents

### dataframe.cpp

#### **sample**

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
// get data targeter from column "name" second data
// you can skip storing df_object_t to `df["name"][1] = value`
df_object_t object = df["name"][1];

// modify to "hello world"
object << "hello world";

// print object
std::cout << "df[\"name\"][1] modified to " << object << "\n";


// == print df modified ==
std::cout << "df after modified\n";
std::cout << df << "\n";

```

#### **df_dataframe_t**

**init**

```cpp
df_dataframe_t(const std::initial_list<df_named_column_t> name_column);

// df_named_column_t is std::pair<std::string, df_column_t>
```

**copy**

```cpp
df_dataframe_t(const df_dataframe_t& dataframe);

df_dataframe_t& operator=(const df_dataframe_t& dataframe);
```

**move**

```cpp
df_dataframe_t(df_data_frame_t&& dataframe);

df_dataframe_t& operator=(df_dataframe_t&& dataframe);
```

**get**

```cpp
int get_column_count() const;

long get_row_count() const;


df_column_t& operator[](const char* column_name);

df_row_t loc(row_index);
```

**iterate**

```cpp
for (df_named_column_t& named_column : dataframe) {
  // your code
}
```

**print**

```cpp
std::ostream& write_ostream(std::ostream& output) const;

static std::ostream& operator<<(std::ostream& output, const df_dataframe_t dataframe);
```

#### **df_column_t**

**init**

```cpp
df_column_t(const std::list<df_object_t> object, long start_capacity = 4096);

df_column_t(df_type_t type, long start_capacity = 4096);
```

**copy**

```cpp
df_column_t(const df_column_t& column);

df_column_t& operator=(const df_column_t& column);
```

**move**

```cpp
df_column_t(df_column_t&& colunn);

df_column_t& operator=(df_column_t&& colunn);
```

**get**

```cpp
// get data type
df_type_t data_type = your_column.get_data_type();

// get length
long length = your_column.get_length();

// target data
// df_object_t is not the data in column, it is a targeter help you to edit data
df_object_t targeter = your_column[index];
```

**iterate**

```cpp
for (df_object_t& object : column) {
  // your code
}
```

**print**

```cpp
// method 1
your_column.write_ostream(std::cout, "your_column_name");

// method 2
std::cout << your_column << "\n";
```

#### **df_object_t**

**init**

```cpp
template<typename T> df_object_t(const T value);
```

- df_object_t(const_copy_object) = delete;
- df_object_t(move_object);
- df_object_t operator=(const_copy_object);
- df_object_t operator=(move_object);
- bool is_null() const;
- bool is_locked() const;
- template `<typename T>` operator T() const;
- template `<typename T>` operator=(const_T_value);
- template `<typename T>` operator<<(const_T_value);
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
