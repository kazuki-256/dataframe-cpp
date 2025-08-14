#ifndef _DF_PROCESS_HPP_
#define _DF_PROCESS_HPP_

#ifndef _DF_DATAFRAME_HPP_
#include "DataFrame.hpp"
#endif

#include <vector>
#include <set>


typedef bool (*DfProcessCallback)(DfObject& writeObject, DfRow& sourcesRow, DfObject&);




class DfProcessTask {
  friend class DfProcess;
  friend class DfProcessOutput;

  DfProcessCallback callback;
  DfObject param;

  DfProcessTask(DfProcessCallback _callback, const DfObject& object) {
    callback = _callback;
    param = object;
  }

  inline bool run(DfObject& writeObject, DfRow& srcRow) {
    return callback(writeObject, srcRow, param);
  }
};



class DfProcessSource {
  friend class DfProcess;

  std::string name;
  DfColumn* column;


  DfProcessSource(const char* _name, DfColumn* _column = NULL) {
    name = _name;
    column = _column;
  }

  void set(DfColumn* _colunn) {
    column = _colunn;
  }

public:
  DfColumn& get() const;
};


class DfProcessOutput {
  friend class DfProcess;

  std::string name;    // output name;
  DfProcessSource* source;
  std::vector<DfProcessTask> tasks;

public:
  DfProcessOutput& addTask(DfProcessCallback callback, void* userptr1, void* userptr2);
};




class DfProcess {
  std::list<DfProcessOutput> outputs;
std::vector<DfProcessSource> inputs;

  std::list<DfProcessTask> filter;
  std::set<DfObject> groups;

  DfProcessSource* order = NULL;
  bool desc = false;

  int limit = -1;

  DfDataFrame* temp = NULL;   // for order by

public:

  ~DfProcess() {
    if (temp) {
      delete temp;
    }
  }



  // === create ===

  DfProcess(DfColumn& column) {

  }

  DfProcess(DfDataFrame& dataFrame) {

  }



  // === execute / convert ===

  class Iterator {

  };

  Iterator begin() {

  }

  Iterator end() {

  }


  operator DfColumn() {

  }

  operator DfDataFrame() {

  }



  // === SQL ===

  DfProcess& select(const char* selectCode) {

  }


  DfProcess& join(DfColumn& colunn, const char* asName, const char* whereCode) {

  }

  DfProcess& join(DfDataFrame& dataFrame, const char* asName, const char* whereCode) {

  }

  DfProcess& join(DfProcess& process, const char* whereCode) {

  }


  DfProcess& where(const char* whereCode) {

  }


  DfProcess& groupBy(const char* colName) {

  }


  DfProcess& orderBy(const char* colName, int desc = 0) {

  }


  DfProcess& mutate(const char* code) {

  }



  // === vector operation ===




  // === print ===

  DfProcess& print() {

  }
  
};







#endif // _DF_PROCESS_HPP_