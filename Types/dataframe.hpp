#ifndef _DF_DATAFRAME_HPP_
#define _DF_DATAFRAME_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_COLUMN_HPP_
#include "column.hpp"
#endif




class df_row_t {
protected:
  std::vector<std::pair<const std::string*, df_column_t::iterator>> named_iter_list;

  df_row_t(std::vector<df_named_column_t>& init_named_columns) {
    const int COUNT = init_named_columns.size();
    named_iter_list.resize(COUNT);

    for (int i = 0; i < COUNT; i++) {
      df_named_column_t& named_column = init_named_columns[i];
      named_iter_list[i] = {&named_column.first, named_column.second.begin()};
    }
  }

public:
  df_row_t& operator++() {
    for (int i = 0; i < named_iter_list.size(); i++) {
      named_iter_list[i].second++;
    }
  }

  bool is_end() const {
    return named_iter_list[0].second.is_end();
  }

  df_object_t& operator[](const char* name) {
    for (std::pair<const std::string*, df_column_t::iterator>& named_iter : named_iter_list) {
      if (named_iter.first->compare(name) == 0) {
        return *named_iter.second;
      }
    }
    throw df_exception_out_of_index();
  }
};


class df_const_row_t : df_row_t {

};








class dataframe {
  std::vector<df_named_column_t> columns;

public:
  ~dataframe() {
    columns.clear();
  }


  dataframe(const std::initializer_list<df_named_column_t>& init_column_list) {
    df_debug4("create dataframe 1");

    columns.resize(init_column_list.size());

    for (auto& pair : init_column_list) {
      columns[0] = pair;
    }
  }




  // == move ==

  dataframe(dataframe&& src) {
    columns = std::move(src.columns);
  }


  // == copy ==

  dataframe(dataframe& src) {
    columns = src.columns;
  }

  dataframe& operator=(dataframe& src) {
    if (!columns.empty()) {
      columns.clear();
    }

    columns = src.columns;
    return *this;
  }



  // == get ==

  int get_column_count() const {
    return columns.size();
  }

  int get_row_count() const {
    return columns[0].second.get_length();
  }
  


  df_named_column_t& operator[](const char* name) {
    for (df_named_column_t& named_column : columns) {
      if (named_column.first.compare(name) == 0) {
        return named_column;
      }
    }
    throw df_exception_out_of_index();
  }

  const df_named_column_t& operator[](const char* name) const {
    for (const df_named_column_t& named_column : columns) {
      if (named_column.first.compare(name) == 0) {
        return named_column;
      }
    }
    throw df_exception_out_of_index();
  }


  df_row_t& loc(int index);

  df_row_t& loc(int index) const;



  // == print ==


  std::ostream& write_stream(std::ostream& os) const {
    int column_count = get_column_count();
    int row_count = get_row_count();

    // loader, writer, iterator

    std::vector<int> types(column_count);
    std::vector<> iters(column_count);

    os << "|  ";
    int index = 0;
    for (auto& named_column : columns) {
      os << named_column.first << "  |  ";
      
      types[index] = named_column.second.get_data_type();
      iters[index] = named_column.second.begin();
      index++;
    }
    os << "\n";

    // printf("row_count: %d\n", row_count);
    for (int row = 0; row < row_count; row++) {
      os << "| ";
      for (int column = 0; column < column_count; column++) {
        auto& iter = iters[column];

        os << (*iter). << " | ";
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