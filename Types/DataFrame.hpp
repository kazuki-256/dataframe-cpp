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




// template for DfRow and DfConstRow
class DfTempRow {
protected:
  void* userptr;

  DfRowGetByString funcGetByString;
  DfRowGetByIndex funcGetByIndex;

  DfRowGetLength funcGetLength;

  DfRowRelease funcRelease;


  void setPtr(void* _userptr) {
    userptr = _userptr;
  }

public:
  ~DfTempRow() {
    if (userptr) funcRelease(userptr);
  }

  int getLength() const {
    return funcGetLength(userptr);
  }
};


class DfRow : public DfTempRow {
public:
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

  DfObject& operator[](const char* colName) {
    return funcGetByString(userptr, colName);
  }

  DfObject& operator[](int index) {
    return funcGetByIndex(userptr, index);
  }
};


class DfConstRow : public DfTempRow {
public:
  DfConstRow(void* _userptr,
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

  const DfObject& operator[](const char* colName) const {
    return funcGetByString(userptr, colName);
  }

  const DfObject& operator[](int index) const {
    return funcGetByIndex(userptr, index);
  }
};






class DfDataFrame {
  std::unordered_map<std::string, DfColumn> columns;

public:
  ~DfDataFrame() {

  }



  DfDataFrame(const std::initializer_list<std::pair<const char*, DfColumn>>& _columns) {
    DfDebug4("create DfDataFrame 1");

    for (auto& pair : _columns) {
      columns.insert(pair);
    }
  }

  DfDataFrame(const std::initializer_list<std::string>& columnNames) {
    DfDebug4("create DfDataFrame 2");

    for (const std::string& name : columnNames) {
      columns.insert({name, DfColumn()});
    }
  }


  // == copy ==

  DfDataFrame& operator=(DfDataFrame& src) {
    if (!columns.empty()) {
      columns.clear();
    }

    columns = src.columns;
    return *this;
  }

  DfDataFrame(DfDataFrame& src) {
    columns = src.columns;
  }

  // == move ==

  DfDataFrame& operator=(DfDataFrame&& src) {
    return *this;    
  }

  DfDataFrame(DfDataFrame&& src) {
    
  }



  // == get ==

  DfColumn& operator[](const char* name) {
    return columns[name];
  }

  const DfColumn& operator[](const char* name) const {
    return columns.at(name);
  }


  DfRow& loc(int index) {
    
  }

  DfRow& loc(int index) const {
    
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