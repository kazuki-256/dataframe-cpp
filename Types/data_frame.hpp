#ifndef _DF_DATA_FRAME_HPP_
#define _DF_DATA_FRAME_HPP_

#ifndef _DF_COLUMN_HPP_
#include "column.hpp"
#endif

#include <unordered_map>




typedef df_object<>& (*df_row_get_by_string)(void* userptr, const char* colName);
typedef df_object<>& (*df_row_get_by_index)(void* userptr, int index);
typedef int (*df_row_get_length)(void* userptr);
typedef void (*df_row_release)(void* userptr);



class df_process;




// template for df_row and df_const_row
class _df_row {
protected:
  void* userptr;

  df_row_get_by_string funcGetByString;
  df_row_get_by_index funcGetByIndex;

  df_row_get_length funcGetLength;

  df_row_release funcRelease;


  void setPtr(void* _userptr) {
    userptr = _userptr;
  }

public:
  ~_df_row() {
    if (userptr) funcRelease(userptr);
  }

  int getLength() const {
    return funcGetLength(userptr);
  }
};


class df_row : public _df_row {
public:
  df_row(void* _userptr,
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

  df_object<>& operator[](const char* colName) {
    return funcGetByString(userptr, colName);
  }

  df_object<>& operator[](int index) {
    return funcGetByIndex(userptr, index);
  }
};


class df_const_row : public _df_row {
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

  const df_object<>& operator[](const char* colName) const {
    return funcGetByString(userptr, colName);
  }

  const df_object<>& operator[](int index) const {
    return funcGetByIndex(userptr, index);
  }
};






class df_data_frame {
  std::unordered_map<df_string, df_column<void*>> columns;

public:
  ~df_data_frame() {

  }



  df_data_frame(const std::initializer_list<std::pair<const char*, df_column<void*>>>& _columns) {
    df_debug4("create df_data_frame 1");

    for (auto& pair : _columns) {
      columns.insert(pair);
    }
  }

  df_data_frame(const std::initializer_list<df_string>& columnNames) {
    df_debug4("create df_data_frame 2");

    for (const df_string& name : columnNames) {
      columns.insert({name, df_column<void*>()});
    }
  }


  // == copy ==

  df_data_frame& operator=(df_data_frame& src) {
    if (!columns.empty()) {
      columns.clear();
    }

    columns = src.columns;
    return *this;
  }

  df_data_frame(df_data_frame& src) {
    columns = src.columns;
  }

  // == move ==

  df_data_frame(df_data_frame&& src) {
    
  }

  df_data_frame& operator=(df_data_frame&& src) {
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

  friend std::ostream& operator<<(std::ostream& os, const df_data_frame& df) {
    df.write_stream(os);
    return os;
  }
  

  


  df_process select(const char* sql) const;

  df_process where(const char* sql) const;

  df_process group_by(const char* sql) const;

  df_process order_by(const char* sql) const;

  df_process with(df_data_frame& other, const char* as_name) const;
};





df_data_frame df_read_csv(const char* csv_file);

df_data_frame df_read_excel(const char* excel_file);

df_data_frame df_read_html(const char* html_file);

df_data_frame df_read_xml(const char* xml_file);

df_data_frame df_read_db();



#ifdef _MYHTML2_H_

df_data_frame DfReadHtml() {

}

#endif


#endif // _DF_DATA_FRAME_HPP_