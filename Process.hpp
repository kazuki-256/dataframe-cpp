#ifndef _DF_PROCESS_HPP_
#define _DF_PROCESS_HPP_

#ifndef _DF_DATAFRAME_HPP_
#include "DataFrame.hpp"
#endif


typedef bool (*DfProcessCallback)(DfObject& writeObject, void*, void*);




class DfProcessTask {
  friend class DfProcess;
  friend class DfProcessOutput;

  DfProcessCallback callback;
  void* userptr1;
  void* userptr2;

  DfProcessTask(DfProcessCallback _callback, void* _userptr1, _userptr2) {
    callback = _callback;
    userptr1 = _userptr1;
    userptr2 = _userptr2;
  }

  inline bool run(DfObject& writeObject) {
    return callback(writeObject, userptr1, userptr2);
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

  DfSource* order = NULL;
  bool desc = false;

  DfDataFrame* temp = NULL;

public:

  ~DfProcess() {
    if (temp) {
      delete temp;
    }
  }



  DfProcess(DfColumn& column) {

  }

  DfProcess(DfDataFrame& dataFrame) {

  }



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


  DfProcess& groupBy(const char* columnName) {

  }


  DfProcess& orderBy(const char* columnName, int desc = 0) {

  }


  DfProcess& print() {

  }
  
};







#endif // _DF_PROCESS_HPP_