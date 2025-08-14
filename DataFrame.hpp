#ifndef _MY_DATA_FRAME_HPP_
#define _MY_DATA_FRAME_HPP_

#ifndef _DF_COLUMN_HPP_
#include "Column.hpp"
#endif

#include <unordered_map>







class DfDataFrame {
  std::unordered_map<std::string, DfColumn> columns;

public:
  ~DfDataFrame() {

  }

  DfDataFrame(DfDataFrame& src) {
    
  }

  DfDataFrame(const std::initializer_list<std::pair<const char*, DfColumn>>& _columns) {
    
  }

  DfDataFrame(const std::initializer_list<std::string>& columnNames) {
    
  }



  DfColumn& operator[](const char* name) {

  }

  DfRow& loc(int index) {
    
  }
  

  


  DfProcess select(const char* sql) const;

  DfProcess where(const char* sql) const;

  DfProcess groupBy(const char* sql) const;

  DfProcess orderBy(const char* sql) const;

  DfProcess with(DfDataFrame& other, const char* asName) const;
};





DfDataFrame DfReadCsv(const char* csvFile) {

}

DfDataFrame DfReadExcel(const char* excelFile) {

}

DfDataFrame DfReadXml(const char* xmlFile) {

}

#ifdef _MYHTML2_H_

DfDataFrame DfReadHtml() {

}

#endif


#endif // _MY_DATA_FRAME_HPP_