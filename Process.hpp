#ifndef _DF_PROCESS_HPP_
#define _DF_PROCESS_HPP_

#ifndef _DF_DATAFRAME_HPP_
#include "DataFrame.hpp"
#endif




class DfProcessTask {
  void (*task)(DfObject& objectInNewColumn, void* userptr);
  void* userptr;  // customisable value, e.g. userptr = 2 when column * 2
};


class DfProcessColumn {
  std::string sourceName;
  std::string destName;    // sourceName as destName
  DfColumn* source;        // if null, get column from ownerProcess.inputs[sourceName]

  std::vector<DfProcessTask> tasks;
};


class DfProcess {
  std::list<DfProcessColumn> outputs;
  std::unordered_map<std::string, DfColumn*> inputs;
};







#endif // _DF_PROCESS_HPP_