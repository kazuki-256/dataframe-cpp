#ifndef _DF_DATAFRAME_HPP_
#define _DF_DATAFRAME_HPP_

#ifndef _DF_CONFIG_HPP_
#include "../config.hpp"
#endif

#ifndef _DF_COLUMN_HPP_
#include "column.hpp"
#endif

#include <tuple>




class df_row_t {
protected:
  std::vector<std::pair<const std::string*, df_column_t::iterator>> named_iter_list;

  df_row_t(std::vector<df_named_column_t>& init_named_named_columns) {
    const int COUNT = init_named_named_columns.size();
    named_iter_list.resize(COUNT);

    for (int i = 0; i < COUNT; i++) {
      df_named_column_t& named_column = init_named_named_columns[i];
      named_iter_list[i] = {&named_column.first, named_column.second.begin()};
    }
  }

public:
  df_row_t& operator++() {
    for (int i = 0; i < named_iter_list.size(); i++) {
      named_iter_list[i].second++;
    }
    return *this;
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
  std::vector<df_named_column_t> named_columns;

public:
  ~dataframe() {
    named_columns.clear();
  }


  dataframe(const std::initializer_list<df_named_column_t>& init_column_list) {
    df_debug4("create dataframe 1");

    named_columns.resize(init_column_list.size());

    for (auto& pair : init_column_list) {
      named_columns[0] = pair;
    }
  }




  // == move ==

  dataframe(dataframe&& src) {
    named_columns = std::move(src.named_columns);
  }


  // == copy ==

  dataframe(dataframe& src) {
    named_columns = src.named_columns;
  }

  dataframe& operator=(dataframe& src) {
    if (!named_columns.empty()) {
      named_columns.clear();
    }

    named_columns = src.named_columns;
    return *this;
  }



  // == get ==

  int get_column_count() const {
    return named_columns.size();
  }

  long get_row_count() const {
    return named_columns[0].second.get_length();
  }
  


  df_named_column_t& operator[](const char* name) {
    for (df_named_column_t& named_column : named_columns) {
      if (named_column.first.compare(name) == 0) {
        return named_column;
      }
    }
    throw df_exception_out_of_index();
  }

  const df_named_column_t& operator[](const char* name) const {
    for (const df_named_column_t& named_column : named_columns) {
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
    const int COLUMN_COUNT = get_column_count();
    const long ROW_COUNT = get_row_count();

    // loader, writer, iterator
    std::vector<std::tuple<df_column_t::const_mem_iterator, df_value_load_callback_t, df_value_write_callback_t>> iter_loader_writer(COLUMN_COUNT);

    // == print titles ==

    os << "|  ";
    int index = 0;
    for (auto& named_column : named_columns) {
      os << named_column.first << "  |  ";
      
      
      auto& info_tuple = iter_loader_writer[index++];
      
      std::get<0>(info_tuple) = named_column.second.mem_begin();
      std::get<1>(info_tuple) = df_value_get_load_callback(named_column.second.get_data_type());
      std::get<2>(info_tuple) = df_value_get_write_callback(named_column.second.get_data_type(), DF_TYPE_TEXT);
    }
    os << "\n";

    // == print data ==


    for (int row = 0; row < ROW_COUNT; row++) {
      os << "| ";
      for (int column_index = 0; column_index < COLUMN_COUNT; column_index++) {
        auto& info_tuple = iter_loader_writer[COLUMN_COUNT];

        df_value_t value = std::get<1>(info_tuple)(*std::get<0>(info_tuple)++);
        df_string_t s;
        std::get<2>(info_tuple)(value, &s);

        os << s.value_or("null") << " | ";
      }
      os << "\n";
    }
    return os;
  }

  friend std::ostream& operator<<(std::ostream& os, const dataframe& df) {
    df.write_stream(os);
    return os;
  }
  

  


  df_query_t select(const char* sql) const;

  df_query_t where(const char* sql) const;

  df_query_t group_by(const char* sql) const;

  df_query_t order_by(const char* sql) const;

  df_query_t with(dataframe& other, const char* as_name) const;
};




#endif // _DF_DATAFRAME_HPP_