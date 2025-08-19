#ifndef _DF_DATAFRAME_HPP_
#define _DF_DATAFRAME_HPP_

#ifndef _DF_COLUMN_HPP_
#include "column.hpp"
#endif

#include <unordered_map>




typedef raw<>& (*df_row_get_by_string)(void* userptr, const char* colName);
typedef raw<>& (*df_row_get_by_index)(void* userptr, int index);
typedef int (*df_row_get_length)(void* userptr);
typedef void (*df_row_release)(void* userptr);



class df_process;




// template for df_row and df_const_row
class basic_row {
protected:
  void* userptr;

  row_get_by_string funcGetByString;
  row_get_by_index funcGetByIndex;

  row_get_length funcGetLength;

  row_release funcRelease;


  void setPtr(void* _userptr);

public:
  ~basic_row();

  int getLength() const;
};


class row : public basic_row {
public:
  row(void* _userptr,
        row_get_by_string getByString,
        row_get_by_index getByIndex,
        row_get_length getLength,
        row_release release)
  {
    userptr = _userptr;
    funcGetByString = getByString;
    funcGetByIndex = getByIndex;
    funcGetLength = getLength;
    funcRelease = release;
  }

  raw<>& operator[](const char* colName) {
    return funcGetByString(userptr, colName);
  }

  raw<>& operator[](int index) {
    return funcGetByIndex(userptr, index);
  }
};


class df_const_row : public basic_row {
public:
  df_const_row(void* _userptr,
        df_row_get_by_string getByString,
        df_row_get_by_index getByIndex,
        df_row_get_length getLength,
        df_row_release release)
  {
    userptr = _userptr;
    funcGetByString = getByString;
    funcGetByIndex = getByIndex;
    funcGetLength = getLength;
    funcRelease = release;
  }

  const raw<>& operator[](const char* colName) const {
    return funcGetByString(userptr, colName);
  }

  const raw<>& operator[](int index) const {
    return funcGetByIndex(userptr, index);
  }
};






class dataframe {
  std::unordered_map<df_string, df_column<void*>> columns;

public:
  ~dataframe() {

  }



  dataframe(const std::initializer_list<std::pair<const char*, df_column<void*>>>& _columns) {
    df_debug4("create dataframe 1");

    for (auto& pair : _columns) {
      columns.insert(pair);
    }
  }

  dataframe(const std::initializer_list<df_string>& columnNames) {
    df_debug4("create dataframe 2");

    for (const df_string& name : columnNames) {
      columns.insert({name, df_column<void*>()});
    }
  }


  // == copy ==

  dataframe& operator=(dataframe& src) {
    if (!columns.empty()) {
      columns.clear();
    }

    columns = src.columns;
    return *this;
  }

  dataframe(dataframe& src) {
    columns = src.columns;
  }

  // == move ==

  dataframe(dataframe&& src) {
    
  }

  dataframe& operator=(dataframe&& src) {
    return *this;    
  }



  // == get ==

  int get_column_count() const {
    return columns.size();
  }

  int get_row_count() const {
    return (*columns.begin()).second.get_length();
  }
  


  df_column<df_undefined>& operator[](const char* name) {
    return columns[name];
  }

  const df_column<df_undefined>& operator[](const char* name) const {
    return columns.at(name);
  }


  df_row& loc(int index);

  df_row& loc(int index) const;



  // == print ==


  std::ostream& write_stream(std::ostream& os) const {
    int column_count = get_column_count();
    int row_count = get_row_count();

    std::vector<int> types(column_count);
    std::vector<df_column<void*>::const_iterator> iters(column_count);

    os << "| ";
    int index = 0;
    for (auto& named_column : columns) {
      os << named_column.first << " | ";
      
      types[index] = named_column.second.get_type();
      iters[index] = named_column.second.begin();
      index++;
    }
    os << "\n";

    // printf("row_count: %d\n", row_count);
    for (int row = 0; row < row_count; row++) {
      os << "| ";
      for (int column = 0; column < column_count; column++) {
        auto& iter = iters[column];

        os << (*iter).c_str(types[column]) << " | ";
        iter++;
      }
      os << "\n";
    }
    return os;
  }

  friend std::ostream& operator<<(std::ostream& os, const dataframe& df) {
    df.write_stream(os);
    return os;
  }
  

  


  df_process select(const char* sql) const;

  df_process where(const char* sql) const;

  df_process group_by(const char* sql) const;

  df_process order_by(const char* sql) const;

  df_process with(dataframe& other, const char* as_name) const;
};




#endif // _DF_DATAFRAME_HPP_