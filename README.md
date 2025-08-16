# data_frame_cpp (under developmenting)

## info

| programmer   | ふうき255          |
|:-------------|:-------------------|
| version      | beta 1.0.0         |
| last update  | 2025/08/14         |
| state        | under development  |


## init

This data frame API is for data science in c++, target at readable, high performance and multi-functions.

The API will store the data handles steps to df_process and execute data processs when df_process convert to iterator, df_column or df_data_frame. it would support sql commands, vector operation and data science method (min(), max(), median(), etc)

Currently, this API stills in making the df_data_frame object, but the main.cpp shows over my excepted cleanly. 


## Sample

**excepted**

```cpp
#include "data_frame/data_frame.hpp"

using namespace std;

int main(int argc, char** argv) {
  // == read data ==
  df_data_frame staff = df_read_csv("staffs.csv");
  df_data_frame job = df_read_csv("job.csv");

  // == data processing ==

  df_data_frame df1 = staff.as("staff")      // before to write SQL, you can give your table a name, or start without name (column name only)
            .select("staff.id, staff.name, job.title, job.hourly * staff.worked AS salary")    // select data, just like SQL
            .join(job, "job", "staff.job_id = job.id")      // join other table as "job" by same job id
            .where("salary > 100000")                       // filter
            .order_by("salary", -1);                        // sort the output by desc 

  df_data_frame df2 = staff.as("staff")
            .select("job.job_title, AVG(staff.worked * job.hourly), count")
            .join(job, "job", "staff.job_id == job.id")
            .group_by("job.title")                             // group by job.title
            .mutate("COUNT(*) AS count");                      // also, you can define variables by mutate

  // == vector processing ==
  df_column yearly = df1["salary"] * 12;
  df_column staff_worked = df1["salary"] / df1["job.hourly"];

  // == print ==
  df1.print();
  df2.print();

  // == output ==
  df_write_csv(df1, "staff_salary.csv");
  df_write_db(df2, "job_info.csv");

  return 0;
}
```


**currently**

```cpp
include "data_frame/types/data_frame.hpp"

int main(int argc, char** argv) {
    df_column<df_string> numbers = {"hello world", "yoshihara kazuki"};

    df_data_frame df = {
        {"id", df_column<int>{1, 2, 3, 4, 5}},
        {"name", df_column<df_string>{"1", "2", "3", "4", "5"}}
    };

    // df_data_frame still not completed
    
    numbers.print();
    return 0;
}
```

## Features

1. read/write data frame from/to .csv, .xlsx, .html, .db
2. vector operation
3. SQL execution
4. OpenCV::RandomForest
5. SocketSQL to host SQL server

## depends

1. Freexl (if you want to read .xlsx)
2. xlsxio_write (if you want to write .xlsx)
3. OpenCV (if you want RandomForest)
4. Sqlite3 (if you want to read/write .db)
5. myhtml2 (original included [myhtml2.3.0](https://github.com/Fuuki255/myhtml2) at package)


## Updates

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



## **Classes**

### **df_date**

date object, a time_t encapsulation

operation:
```cpp

// selfs strptime(), no depending system
df_date date("2025-08-14 00:00");
df_date now(time(NULL));

struct tm* tm = localtime(&t);
df_date date_localtime(tm);

printf("the time: %s\n", date.c_str());
```

### **df_object**

multi-types object with compact size (8 bytes)

**structure:**

```cpp
template<typename T>          // the T will not use to store data directly, but it will change the object handling
class df_object {
  union U {
    void*       as_pointer;    // sometime could use df_object to store some customize object, but df_object will not help your memory delete
    double      as_number;     // double type to store BOOLEAN or NUMBER data
    long        as_integer;    // to store CATEGORY data
    df_string*  as_string;     // STRING data
    df_date     as_date;       // DATE data

  } data;
};
```

**methods



### **DfColumn**

column object, store DfObject by chunked vector.

you could directly handlea object by:
- `DfObject& object = column[index]` or
- `for (DfObject& object : column) {}`

also, you can use to execute SQL command:

| code | description|
|:---|:---|
| DfProcess process = column.as("name") | give name to column and become a DfProcess |
| dataFrame.select("id, name").join(column, "name") | join other process as "name" |

also, you can do operation:

```cpp
// == vector operation ==
DfColumn a = column + 2;
DfColumn b = column + other;
DfColumn c = column - 2;
DfColumn d = column - other;
DfColumn e = column * 2;
DfColumn f = column * other;
DfColumn g = column / 2;
DfColumn h = column / 2;
DfColunn i = DfSqrt(column);

// == math operators ==
double total = DfSum(column);
double avg = DfAvg(column);    // also alias DfMean
double medium = DfMedium(column);
double range = DfRange(column);
double min = DfMin(column);
double max = DfMax(column);
int length = column.getLength();

// == object operatuon ==
column.addObject(VALUE);

column.addChunkBack(SIZE);      // create SIZE size empty chunk at back
column.addChunkFront(SIZE);     // create SIZE size chunk at front

column.remove(INDEX);           // remove object in INDEX, not suggested in great size removing, suggest to create a new column

// == iterating objects ==
for (DfObject& object : column) {
  std::cout << object << "\n";
}

```


### **DfDataFrame**

data frame object, store DfObject by multi named-column.

You could directly handles objects by:
- `DfRow& loc(int index)` or
- `for (DfRow& row : dataFrame) {}`

Or you can execute sql command to create new data frame:

| code | description |
|:---|:---|
| df.as("df") | give name to the data frame, all DfColumn or DfDataFrame don't have their owns name |
| df.select("id, x, y") | select output data like SQL (default: SELECT *) |
| df.join(dfSalary, "other", "job = other.id") | include other data frame |
| df.where("age < 25") | set conditions |
| df.groupBy("job") | group by data |
| df.orderBy("age", DF_ASC) | asc/desc order by data |
| df.limit(10) | limit result count |

- all SQL methods making a new DfProcess

Also vector operation: (math 2d vector basic)

```
>>> df
x y
1 2
3 4
>>> df + 2;
x y
3 4
5 6
>>> df - 2;
 x y
-1 0
 1 2
>>> df * 2;
x y
2 4
6 8
>>> df * df;
 x  y
 7 10
15 22
>>> df / 2;
x   y
0.5 1
1.5 2
>>> dfSqrt(df);
x    y
1    1.41
1.73 2
```

- not the real output


### **DfProcess**

a virtual data frame to execute sql command or vector process, its data could convert to Iterator, DfDataFrame or DfColumn.

Sample
```cpp
DfDataFrame staffs = DfReadCsv("staffs.csv");
DfDataFrame jobs = DfReadCsv("jobs.csv");

// == sample1 ==
DfProcess process1 = staffs.as("staff")
    .select("staff.id, staff.name, job.title, job.hourly, salary")
    .join(jobs, "job", "job.id = staff.jobId")
    .mutate("staff.worked * job.hourly", "salary")
    .where("salary > 6000");

process1.print();    // not preferred because no actually data stored

// Outputs:
// staff.id staff.name job.title job.hourly salary
// ...


// == sample2 ==
DfProcess process2 = process1["salary"] * 12;


process2.print();

// Outputs:
// salary
// .
// .
// .



// == outputs ==

DfDataFrame output1 = process1;
DfColumn output1Salary = process["salary"];

if (process.getColumnCount() == 1) {
    DfColumn output2Yearly = process2;  // only for one column output
}
DfDataFrame output2 = process2;


```


### **DfRow**

a virtual object for get/set data from DfDataframe or DfProcess 
