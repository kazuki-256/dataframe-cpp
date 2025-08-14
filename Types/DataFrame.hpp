#ifndef _DF_DATA_FRAME_HPP_
#define _DF_DATA_FRAME_HPP_

#ifndef _DF_COLUMN_HPP_
#include "Column.hpp"
#endif

#include <unordered_map>




typedef DfObject& (*DfRowGetByString)(void* userptr, const char* colName);
typedef DfObject& (*DfRowGetByIndex)(void* userptr, int index);

typedef int (*DfRowGetLength)(void* userptr);

typedef void (*DfRowRelease)(void* userptr);



class DfProcess;



class DfRow {
  void* userptr;

  DfRowGetByString funcGetByString;
  DfRowGetByIndex funcGetByIndex;

  DfRowGetLength funcGetLength;

  DfRowRelease funcRelease;


  DfRow(void* _userptr,
        DfRowGetByString getByString,
        DfRowGetByIndex getByIndex,
        DfRowGetLength getLength,
        DfRowRelease release)
  {
    userptr = _userptr;
    funcGetByString = getByString;
    funcGetByIndex = getByIndex;
    funcGetLength = getLength;
    funcRelease = release;
  }

  void setPtr(void* _userptr) {
    userptr = _userptr;
  }

public:
  ~DfRow() {
    if (userptr) funcRelease(userptr);
  }

  DfObject& operator[](const char* colName) {
    return funcGetByString(userptr, colName);
  }

  DfObject& operator[](int index) {
    return funcGetByIndex(userptr, index);
  }

  int getLength() const {
    return funcGetLength(userptr);
  }
};






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

DfDataFrame DfReadSqlite() {
  
}

#ifdef _MYHTML2_H_

DfDataFrame DfReadHtml() {

}

#endif


#endif // _DF_DATA_FRAME_HPP_