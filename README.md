# My Data Frame (beta 1.0.0)

## init

version: beta 1.0.0
programmer: ふうき255
last update: 2025/08/14


## Features

1. read/write data frame from/to .csv, .xlsx, .html, .db
3. vector operation
4. SQL execution
5. OpenCV::RandomForest

## depends

1. Freexl (if you want to read .xlsx)
2. xlsxio_write (if you want to write .xlsx)
3. OpenCV (if you want RandomForest)
4. Sqlite3 (if you want to read/write .db)
5. myhtml2 (original included [myhtml2.3.0](https://github.com/Fuuki255/myhtml2) at package)

## classes

### **DfDate**

date object, a time_t encapsulation

operation:
```cpp

// selfs strptime(), no depending system
Date date("2025-08-14 00:00");
Date now(time(NULL));

struct tm* tm = localtime(&t);
Date dateFromTm(tm);

printf("the time: %s\n", date.toString());
```

### **DfObject**

cell object, storing multi-type data

samples to get different types data, if getting different types will throw DfException

```cpp
// get type
DfType objType = object.getType();

// is null?
bool isNull = object.isNull();

// get data
void* pointer = object.getPointer();
bool boolean = (bool)object;
int integer = (int)object;
long longInt = (long)object;
double doubleValue = (double)object;
const char* cstr = (const char*)object;
DfDate date = (Date)object;

// set value
object = pointer;
object = boolean;
object = integer;
object = longInt;
object = doubleValue;
object = cstr;
object = date;
```


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
DfDataFrame df = DfReadCsv("data.csv");

// way1
DfProcess process = df.
```


### **DfRow**

a virtual object for get/set data from DfDataframe or DfProcess 
